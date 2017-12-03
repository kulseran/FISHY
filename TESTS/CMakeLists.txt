cmake_minimum_required (VERSION 2.6)
project (Fishy)

file(GLOB_RECURSE tests_src
	"*.h"
	"*.inl"
	"*.cpp"
)
assign_source_group(${tests_src})

add_executable(tests ${tests_src})
target_compile_definitions(tests PRIVATE TESTING=1)
set_target_properties(tests PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(tests core Threads::Threads)
