cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

add_executable(totato
	./main.cpp
)

target_compile_features(totato PUBLIC cxx_std_17)
target_link_libraries(totato PUBLIC
	toxcore
)

