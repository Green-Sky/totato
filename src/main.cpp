#include <solanaceae/object_store/object_store.hpp>
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
#include <solanaceae/message3/message_command_dispatcher.hpp>

#include "./managment_commands.hpp"
#include "./config_commands.hpp"
#include "./tox_commands.hpp"
#include "./fun_commands.hpp"
#include <solanaceae/message3/components.hpp> // TODO: move uptime

//#include <solanaceae/message3/components.hpp>
//#include <solanaceae/contact/components.hpp>
//#include <solanaceae/tox_contacts/components.hpp>
//#include <solanaceae/toxcore/utils.hpp>

#include <nlohmann/json.hpp>

#include <entt/entt.hpp>
#include <entt/fwd.hpp>

#include <cstdint>
#include <cmath>
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

// why is min not variadic?
template<typename... T>
float min_var(float v0, T... args) {
	if constexpr (sizeof...(args) == 0) {
		return v0;
	} else {
		return std::min(v0, min_var(args...));
	}
}

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

	const auto started_at = std::chrono::steady_clock::now();
	auto last_time_tick = std::chrono::steady_clock::now();

	ObjectStore2 os;

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
	MessageCleanser mc{cr, rmm, conf};
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


	ToxEventLogger tel{std::cout}; // TODO: config

	// TODO: password?
	ToxClient tc{conf, conf.get_string("tox", "save_file_path").value(), ""};
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

	PluginManager pm;

	{ // setup plugin instances
		g_provideInstance<ObjectStore2>("ObjectStore2", "host", &os);

		g_provideInstance<ConfigModelI>("ConfigModelI", "host", &conf);
		g_provideInstance<Contact3Registry>("Contact3Registry", "1", "host", &cr);
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

	registerManagementCommands(mcd, conf, cr, rmm);
	// TODO: finish impl
	//registerConfigCommands(mcd, conf, cr, rmm);
	registerToxCommands(mcd, conf, cr, rmm, tc, tpi);
	registerFunCommands(mcd, conf, cr, rmm);

	mcd.registerCommand(
		"totato", "",
		"uptime",
		[&](std::string_view params, Message3Handle m) -> bool {
			const auto contact_from = m.get<Message::Components::ContactFrom>().c;

			const auto uptime = (std::chrono::steady_clock::now() - started_at);

			const auto days = std::chrono::duration_cast<std::chrono::duration<int64_t, std::ratio<86400>>>(uptime);
			const auto hours = std::chrono::duration_cast<std::chrono::hours>(uptime) - std::chrono::duration_cast<std::chrono::hours>(days);
			const auto minutes = (std::chrono::duration_cast<std::chrono::minutes>(uptime) - std::chrono::duration_cast<std::chrono::minutes>(days)) - std::chrono::duration_cast<std::chrono::minutes>(hours);
			const auto seconds = ((std::chrono::duration_cast<std::chrono::seconds>(uptime) - std::chrono::duration_cast<std::chrono::seconds>(days)) - std::chrono::duration_cast<std::chrono::seconds>(hours)) - std::chrono::duration_cast<std::chrono::seconds>(minutes);

			std::string reply_text;
			reply_text += "totato uptime: ";
			reply_text += std::to_string(days.count());
			reply_text += "d ";
			reply_text += std::to_string(hours.count());
			reply_text += "h ";
			reply_text += std::to_string(minutes.count());
			reply_text += "min ";
			reply_text += std::to_string(seconds.count());
			reply_text += "s (";
			reply_text += std::to_string(std::chrono::duration_cast<std::chrono::seconds>(uptime).count());
			reply_text += "s)";

			rmm.sendText(
				contact_from,
				reply_text
			);
			return true;
		},
		"get current uptime.",
		MessageCommandDispatcher::Perms::EVERYONE // mod?
	);

	conf.dump();

	std::this_thread::sleep_for(std::chrono::milliseconds(25)); // at startup, just to be safe

	float last_min_interval {0.1f};
	while (!quit) {
		auto new_time = std::chrono::steady_clock::now();
		const float time_delta_tick = std::chrono::duration<float, std::chrono::seconds::period>(new_time - last_time_tick).count();

		// TODO: implement the equivalent of screen->nextTick() for plugs
		const bool tick = time_delta_tick >= last_min_interval;

		if (tick) {
			quit = !tc.iterate(time_delta_tick);
			tcm.iterate(time_delta_tick);
			ttm.iterate();

			mts.iterate();

			const float pm_interval = pm.tick(time_delta_tick);
			const float mc_interval = mc.iterate(time_delta_tick);
			const float mcd_interval = mcd.iterate(time_delta_tick);
			const float tox_interval = std::pow(tc.toxIterationInterval(), 1.6f) / 1000.f;

			last_min_interval = min_var(
				pm_interval,
				mc_interval,
				mcd_interval,
				tox_interval
			);

			// dont sleep and do an extra check

			last_time_tick = new_time;
			//std::cout << "M: time_delta_tick: " << time_delta_tick << "\n";
			//std::cout << "M: last_min_interval: " << last_min_interval << " (t:" << tox_interval << " p:" << pm_interval << ")\n";
		} else {
			// TODO: replace with something that works on windows
			const float sleep_dur = std::max(last_min_interval-time_delta_tick, 0.001f);
			std::this_thread::sleep_for(std::chrono::milliseconds(int64_t(sleep_dur*1000)));
		}
	}

	conf.dump();

	std::cout << "\nTOTATO shutting down...\n";

	return 0;
}

