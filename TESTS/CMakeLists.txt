cmake_minimum_required (VERSION 2.6)
project (Fishy)

file(GLOB tests_src
	"main.cpp"
	"*/*.h"
	"*/*.inl"
	"*/*.cpp"
)

add_executable(tests ${tests_src})
# target_link_libraries(tests core)
