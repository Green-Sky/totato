cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

add_executable(totato
	./main.cpp

	./tox_client.hpp
	./tox_client.cpp
	./tox_private_impl.hpp
	./auto_dirty.hpp
	./auto_dirty.cpp

	./message_cleanser.hpp
	./message_cleanser.cpp

	./message_command_dispatcher.hpp
	./message_command_dispatcher.cpp

	./managment_commands.hpp
	./managment_commands.cpp

	./config_commands.hpp
	./config_commands.cpp

	./tox_commands.hpp
	./tox_commands.cpp
)

target_compile_features(totato PUBLIC cxx_std_17)
target_link_libraries(totato PUBLIC
	solanaceae_util
	solanaceae_contact
	solanaceae_message3

	solanaceae_plugin

	solanaceae_toxcore
	solanaceae_tox_contacts
	solanaceae_tox_messages

	nlohmann_json::nlohmann_json
)

