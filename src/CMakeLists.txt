cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

add_executable(totato
	./main.cpp
	./tox_client.hpp
	./tox_client.cpp
	./tox_private_impl.hpp
	./auto_dirty.hpp
	./auto_dirty.cpp
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
)

