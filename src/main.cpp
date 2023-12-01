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

#include <chrono>
#include <thread>
#include <atomic>
#include <iostream>

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

	// TODO: parse arg
	// totato <config.json> -p <path/to/plugin.so>

	// HACK: config
	std::string tox_profile_path {"totato.tox"};

	SimpleConfigModel conf;
	Contact3Registry cr;
	RegistryMessageModel rmm{cr};
	MessageTimeSort mts{rmm};

	PluginManager pm;

	ToxEventLogger tel{std::cout};
	ToxClient tc{tox_profile_path, ""};
	ToxPrivateImpl tpi{tc.getTox()};
	AutoDirty ad{tc};
	ToxContactModel2 tcm{cr, tc, tc};
	ToxMessageManager tmm{rmm, cr, tcm, tc, tc};
	ToxTransferManager ttm{rmm, cr, tcm, tc, tc};

	{ // setup plugin instances
		g_provideInstance<ConfigModelI>("ConfigModelI", "host", &conf);
		g_provideInstance<Contact3Registry>("Contact3Registry", "host", &cr);
		g_provideInstance<RegistryMessageModel>("RegistryMessageModel", "host", &rmm);

		//g_provideInstance<ToxI>("ToxI", "host", &tc);
		//g_provideInstance<ToxPrivateI>("ToxPrivateI", "host", &tpi);
		//g_provideInstance<ToxEventProviderI>("ToxEventProviderI", "host", &tc);
		//g_provideInstance<ToxContactModel2>("ToxContactModel2", "host", &tcm);

		// TODO: pm?
	}

	for (const auto& ppath : {"../../solanaceae_ecosystem/build/bin/libplugin_transfer_auto_accept.so"}) {
		if (!pm.add(ppath)) {
			std::cerr << "TOTATO error: loading plugin '" << ppath << "' failed!\n";
			// thow?
			assert(false && "failed to load plugin");
		}
	}

	conf.dump();

	std::this_thread::sleep_for(std::chrono::milliseconds(10)); // at startup, just to be safe

	while (!quit) {
		//auto new_time = std::chrono::steady_clock::now();

		quit = !tc.iterate();
		tcm.iterate(/*time_delta*/0.02f);
		ttm.iterate();

		mts.iterate();

		pm.tick(/*time_delta*/0.02f);

		//std::this_thread::sleep_for( // time left to get to 60fps
			//std::chrono::duration<float, std::chrono::seconds::period>(0.0166f) // 60fps frame duration
			//- std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::steady_clock::now() - new_time) // time used for rendering
		//);
		std::this_thread::sleep_for(std::chrono::milliseconds(20)); // HACK: until i figure out the best main loop
	}

	std::cout << "\nTOTATO shutting down...\n";

	return 0;
}

