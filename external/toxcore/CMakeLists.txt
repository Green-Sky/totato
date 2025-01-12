cmake_minimum_required(VERSION 3.13...3.16 FATAL_ERROR)

if (NOT TARGET toxcore)
	# HACK: support old libsodium find
	# libs should handle this case themselfs
	list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

	set(EXPERIMENTAL_API ON CACHE BOOL "" FORCE)
	set(UNITTEST OFF CACHE BOOL "" FORCE)
	set(BOOTSTRAP_DAEMON OFF CACHE BOOL "" FORCE)
	#if (TOMATO_TOX_AV)
	#    set(BUILD_TOXAV ON CACHE BOOL "" FORCE)
	#    set(MUST_BUILD_TOXAV ON CACHE BOOL "" FORCE)
	#endif()


	include(FetchContent)

	FetchContent_Declare(toxcore
		GIT_REPOSITORY https://github.com/TokTok/c-toxcore.git
		GIT_TAG v0.2.20
		EXCLUDE_FROM_ALL
	)

	FetchContent_GetProperties(toxcore)
	if(NOT toxcore_POPULATED)
		# pupulates and adds cmake subdir
		FetchContent_MakeAvailable(toxcore)

		# TODO: should this all be in the populated if??

		set(TOX_DIR "${toxcore_SOURCE_DIR}/")

		#message("II BUILD_TOXAV: ${BUILD_TOXAV}")

		# the sad case
		add_library(toxcore INTERFACE)

		if (TARGET toxcore_static)
			target_link_libraries(toxcore INTERFACE toxcore_static)

			# the ideal case
			#add_library(toxcore ALIAS toxcore_static)
		else()
			target_link_libraries(toxcore INTERFACE toxcore_shared)

			# the ideal case
			#add_library(toxcore ALIAS toxcore_shared)
		endif()

		# HACK: "install" api headers into binary dir
		configure_file(
			${TOX_DIR}toxcore/tox.h
			${toxcore_BINARY_DIR}/include/tox/tox.h
			@ONLY
		)
		configure_file(
			${TOX_DIR}toxcore/tox_events.h
			${toxcore_BINARY_DIR}/include/tox/tox_events.h
			@ONLY
		)
		configure_file(
			${TOX_DIR}toxcore/tox_private.h
			${toxcore_BINARY_DIR}/include/tox/tox_private.h
			@ONLY
		)
		configure_file(
			${TOX_DIR}toxencryptsave/toxencryptsave.h
			${toxcore_BINARY_DIR}/include/tox/toxencryptsave.h
			@ONLY
		)
		#configure_file(
		#    ${TOX_DIR}toxav/toxav.h
		#    ${toxcore_BINARY_DIR}/include/tox/toxav.h
		#    @ONLY
		#)

		target_include_directories(toxcore INTERFACE ${toxcore_BINARY_DIR}/include/)

		find_package(sodium QUIET)
		if(unofficial-sodium_FOUND) # vcpkg
			if(TARGET unofficial-sodium::sodium)
				target_link_libraries(toxcore INTERFACE unofficial-sodium::sodium)
			endif()
			if(TARGET unofficial-sodium::sodium_config_public)
				target_link_libraries(toxcore INTERFACE unofficial-sodium::sodium_config_public)
			endif()
		elseif(sodium_FOUND)
			target_link_libraries(toxcore INTERFACE sodium)
		else()
			message(SEND_ERROR "missing libsodium")
		endif()

		#if(BUILD_TOXAV)
		#    set_target_properties(toxcore PROPERTIES TOX_HAS_TOXAV ON)
		#endif()

	endif(NOT toxcore_POPULATED)

endif(NOT TARGET toxcore)
