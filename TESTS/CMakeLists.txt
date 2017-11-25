cmake_minimum_required (VERSION 2.6)
project (Fishy)

file(GLOB_RECURSE tests_src
	"main.cpp"
	"*.h"
	"*.inl"
	"*.cpp"
)
assign_source_group(${tests_src})

add_executable(tests ${tests_src})
target_compile_definitions(tests PRIVATE TESTING=1)
target_link_libraries(tests core)
