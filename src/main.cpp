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

	// TODO: parse arg
	// totato <config.json> -p <path/to/plugin.so>

	//auto last_time = std::chrono::steady_clock::now();

	std::this_thread::sleep_for(std::chrono::milliseconds(10)); // at startup, just to be safe

	while (!quit) {
		//auto new_time = std::chrono::steady_clock::now();

		//std::this_thread::sleep_for( // time left to get to 60fps
			//std::chrono::duration<float, std::chrono::seconds::period>(0.0166f) // 60fps frame duration
			//- std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::steady_clock::now() - new_time) // time used for rendering
		//);
		std::this_thread::sleep_for(std::chrono::milliseconds(20)); // HACK: until i figure out the best main loop
	}

	std::cout << "\nTOTATO shutting down...\n";

	return 0;
}

