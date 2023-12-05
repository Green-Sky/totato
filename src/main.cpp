#include <solanaceae/util/simple_config_model.hpp>
#include <solanaceae/contact/contact_model3.hpp>
#include <solanaceae/message3/registry_message_model.hpp>
#include <solanaceae/message3/message_time_sort.hpp>
#include <solanaceae/plugin/plugin_manager.hpp>
#include <solanaceae/toxcore/tox_event_logger.hpp>
#include "./tox_private_impl.hpp"

#include <solanaceae/tox_contacts/tox_contact_model2.hpp>
#include <solanaceae/tox_messages/tox_message_manager.hpp>
#include <solanaceae/tox_messages/tox_transfer_manager.hpp>

#include "./tox_client.hpp"
#include "./auto_dirty.hpp"
#include "./message_cleanser.hpp"
#include "./message_command_dispatcher.hpp"

#include "./tox_commands.hpp"

//#include <solanaceae/message3/components.hpp>
//#include <solanaceae/contact/components.hpp>
//#include <solanaceae/tox_contacts/components.hpp>
//#include <solanaceae/toxcore/utils.hpp>

#include <nlohmann/json.hpp>

#include <chrono>
#include <thread>
#include <atomic>
#include <iostream>
#include <fstream>
#include <filesystem>

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <signal.h>
#include <unistd.h>
#elif defined (_WIN32)
#include <signal.h>
#endif

std::atomic_bool quit = false;

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__)) || defined (_WIN32)
void sigint_handler(int signo) {
	if (signo == SIGINT) {
		quit = true;
	}
}
#endif

bool load_json_into_config(const nlohmann::ordered_json& config_json, SimpleConfigModel& conf) {
	if (!config_json.is_object()) {
		std::cout << "TOTATO error: config file is not an json object!!!\n";
		return false;
	}
	for (const auto& [mod, cats] : config_json.items()) {
		for (const auto& [cat, cat_v] : cats.items()) {
			if (cat_v.is_object()) {
				if (cat_v.contains("default")) {
					const auto& value = cat_v["default"];
					if (value.is_string()) {
						conf.set(mod, cat, value.get_ref<const std::string&>());
					} else if (value.is_boolean()) {
						conf.set(mod, cat, value.get_ref<const bool&>());
					} else if (value.is_number_float()) {
						conf.set(mod, cat, value.get_ref<const double&>());
					} else if (value.is_number_integer()) {
						conf.set(mod, cat, value.get_ref<const int64_t&>());
					} else {
						std::cerr << "JSON error: wrong value type in " << mod << "::" << cat << " = " << value << "\n";
						return false;
					}
				}
				if (cat_v.contains("entries")) {
					for (const auto& [ent, ent_v] : cat_v["entries"].items()) {
						if (ent_v.is_string()) {
							conf.set(mod, cat, ent, ent_v.get_ref<const std::string&>());
						} else if (ent_v.is_boolean()) {
							conf.set(mod, cat, ent, ent_v.get_ref<const bool&>());
						} else if (ent_v.is_number_float()) {
							conf.set(mod, cat, ent, ent_v.get_ref<const double&>());
						} else if (ent_v.is_number_integer()) {
							conf.set(mod, cat, ent, ent_v.get_ref<const int64_t&>());
						} else {
							std::cerr << "JSON error: wrong value type in " << mod << "::" << cat << "::" << ent << " = " << ent_v << "\n";
							return false;
						}
					}
				}
			} else {
				if (cat_v.is_string()) {
					conf.set(mod, cat, cat_v.get_ref<const std::string&>());
				} else if (cat_v.is_boolean()) {
					conf.set(mod, cat, cat_v.get_ref<const bool&>());
				} else if (cat_v.is_number_float()) {
					conf.set(mod, cat, cat_v.get_ref<const double&>());
				} else if (cat_v.is_number_integer()) {
					conf.set(mod, cat, cat_v.get_ref<const int64_t&>());
				} else {
					std::cerr << "JSON error: wrong value type in " << mod << "::" << cat << " = " << cat_v << "\n";
					return false;
				}
			}
		}
	}

	return true;
}

int main(int argc, char** argv) {
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
	struct sigaction sigint_action;
	sigint_action.sa_handler = sigint_handler;
	sigemptyset (&sigint_action.sa_mask);
	sigint_action.sa_flags = 0;
	sigaction(SIGINT, &sigint_action, NULL);
#elif defined (_WIN32)
	signal(SIGINT, sigint_handler);
#endif

	auto last_time = std::chrono::steady_clock::now();

	std::string config_path {"config.json"};

	// totato <config.json> -p <path/to/plugin.so>
	// TODO: parse arg
	if (argc == 2) {
		config_path = argv[1];
	}

	SimpleConfigModel conf;

	{ // read conf from json TODO: refactor extract this for reuse
		auto config_file = std::ifstream(config_path);
		if (!config_file.is_open()) {
			std::cerr << "TOTATO error: failed to open config file '" << config_path << "', exiting...\n";
			return -1;
		}

		auto config_json = nlohmann::ordered_json::parse(std::ifstream(config_path));
		if (!load_json_into_config(config_json, conf)) {
			std::cerr << "TOTATO error in config json, exiting...\n";
			return -1;
		}
	}

	{ // fill in defaults
		// config file folder
		const auto config_path_base = std::filesystem::path{config_path}.parent_path();

		if (!conf.has_string("tox", "save_file_path")) {
			// default to totato.tox relative to config file
			conf.set("tox", "save_file_path", (config_path_base / "totato.tox").u8string());
		} else { // transform relative to config to absolute
			const auto tox_conf_path = std::filesystem::path{static_cast<std::string_view>(conf.get_string("tox", "save_file_path").value())};
			if (tox_conf_path.is_relative()) {
				// is relative to config
				conf.set("tox", "save_file_path", std::filesystem::canonical(config_path_base / tox_conf_path).u8string());
			}
		}

		// TODO: name
	}

	Contact3Registry cr;
	RegistryMessageModel rmm{cr};
	MessageTimeSort mts{rmm};
	MessageCleanser mc{cr, rmm};
	MessageCommandDispatcher mcd{cr, rmm, conf};

	{ // setup basic commands for bot
		mcd.registerCommand(
			"host", "",
			"info",
			[](std::string_view, Message3Handle m) -> bool {
				std::cout << "INFO got called :)\n";
				return true;
			},
			"Get basic information about this bot"
		);

	}

	PluginManager pm;

	ToxEventLogger tel{std::cout}; // TODO: config

	// TODO: password?
	ToxClient tc{conf.get_string("tox", "save_file_path").value(), ""};
	tel.subscribeAll(tc);
	{ // name stuff
		auto name = tc.toxSelfGetName();
		if (name.empty()) {
			name = "totato";
		}
		conf.set("tox", "name", name);
		tc.setSelfName(name); // TODO: this is ugly
	}

	std::cout << "TOTATO: own address: " << tc.toxSelfGetAddressStr() << "\n";

	ToxPrivateImpl tpi{tc.getTox()};
	AutoDirty ad{tc};
	ToxContactModel2 tcm{cr, tc, tc};
	ToxMessageManager tmm{rmm, cr, tcm, tc, tc};
	ToxTransferManager ttm{rmm, cr, tcm, tc, tc};

	{ // setup plugin instances
		g_provideInstance<ConfigModelI>("ConfigModelI", "host", &conf);
		g_provideInstance<Contact3Registry>("Contact3Registry", "host", &cr);
		g_provideInstance<RegistryMessageModel>("RegistryMessageModel", "host", &rmm);
		g_provideInstance<MessageCommandDispatcher>("MessageCommandDispatcher", "host", &mcd);

		g_provideInstance<ToxI>("ToxI", "host", &tc);
		g_provideInstance<ToxPrivateI>("ToxPrivateI", "host", &tpi);
		g_provideInstance<ToxEventProviderI>("ToxEventProviderI", "host", &tc);
		g_provideInstance<ToxContactModel2>("ToxContactModel2", "host", &tcm);

		// TODO: pm?
	}

	// load from config!!!
	// HACK: we cheat and directly access the members
	// TODO: add api to iterate
	if (conf._map_bool.count("PluginManager") && conf._map_bool.at("PluginManager").count("autoload")) {
		const auto config_path_base = std::filesystem::path{config_path}.parent_path();

		for (const auto& [plugin_path, plugin_autoload] : conf._map_bool.at("PluginManager").at("autoload").second) {
			if (plugin_autoload) {
				std::filesystem::path real_plugin_path = plugin_path;
				if (real_plugin_path.is_relative()) {
					real_plugin_path = config_path_base / real_plugin_path;
				}

				if (!pm.add(real_plugin_path.u8string())) {
					std::cerr << "TOTATO error: loading plugin '" << real_plugin_path << "' failed!\n";
					// thow?
					assert(false && "failed to load plugin");
				}
			}
		}
	}

	registerToxCommands(mcd, conf, cr, rmm, tc, tpi);

	conf.dump();

	std::this_thread::sleep_for(std::chrono::milliseconds(10)); // at startup, just to be safe

	while (!quit) {
		//auto new_time = std::chrono::steady_clock::now();

		quit = !tc.iterate();
		tcm.iterate(/*time_delta*/0.02f);
		ttm.iterate();

		mts.iterate();

		pm.tick(/*time_delta*/0.02f);

		mc.iterate(0.02f);

		mcd.iterate(0.02f);

		//std::this_thread::sleep_for( // time left to get to 60fps
			//std::chrono::duration<float, std::chrono::seconds::period>(0.0166f) // 60fps frame duration
			//- std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::steady_clock::now() - new_time) // time used for rendering
		//);
		std::this_thread::sleep_for(std::chrono::milliseconds(20)); // HACK: until i figure out the best main loop
	}

	std::cout << "\nTOTATO shutting down...\n";

	return 0;
}

