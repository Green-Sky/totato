cmake_minimum_required(VERSION 3.14...3.24 FATAL_ERROR)

include(FetchContent)

if (NOT TARGET solanaceae_util)
	FetchContent_Declare(solanaceae_util
		GIT_REPOSITORY https://github.com/Green-Sky/solanaceae_util.git
		GIT_TAG master
		EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(solanaceae_util)
endif()

if (NOT TARGET solanaceae_contact)
	FetchContent_Declare(solanaceae_contact
		GIT_REPOSITORY https://github.com/Green-Sky/solanaceae_contact.git
		GIT_TAG master
		EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(solanaceae_contact)
endif()

if (NOT TARGET solanaceae_message3)
	FetchContent_Declare(solanaceae_message3
		GIT_REPOSITORY https://github.com/Green-Sky/solanaceae_message3.git
		GIT_TAG master
		EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(solanaceae_message3)
endif()

if (NOT TARGET solanaceae_plugin)
	FetchContent_Declare(solanaceae_plugin
		GIT_REPOSITORY https://github.com/Green-Sky/solanaceae_plugin.git
		GIT_TAG master
		EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(solanaceae_plugin)
endif()

if (NOT TARGET solanaceae_object_store)
	FetchContent_Declare(solanaceae_object_store
		GIT_REPOSITORY https://github.com/Green-Sky/solanaceae_object_store.git
		GIT_TAG master
		EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(solanaceae_object_store)
endif()

add_subdirectory(./toxcore)

if (NOT TARGET solanaceae_toxcore)
	FetchContent_Declare(solanaceae_toxcore
		GIT_REPOSITORY https://github.com/Green-Sky/solanaceae_toxcore.git
		GIT_TAG master
		EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(solanaceae_toxcore)
endif()

if (NOT TARGET solanaceae_tox_contacts AND NOT TARGET solanaceae_tox_messages)
	FetchContent_Declare(solanaceae_tox
		GIT_REPOSITORY https://github.com/Green-Sky/solanaceae_tox.git
		GIT_TAG master
		EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(solanaceae_tox)
endif()

if (NOT TARGET nlohmann_json::nlohmann_json)
	FetchContent_Declare(json
		URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
		URL_HASH SHA256=d6c65aca6b1ed68e7a182f4757257b107ae403032760ed6ef121c9d55e81757d
		EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(json)
endif()

