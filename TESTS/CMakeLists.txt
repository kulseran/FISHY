cmake_minimum_required (VERSION 3.1)
project (Fishy)

# only the version part of CMAKE_SYSTEM 
MESSAGE( STATUS "CMAKE_SYSTEM_VERSION: " ${CMAKE_VERSION} )

file(GLOB_RECURSE tests_src
	"*.h"
	"*.inl"
	"*.cpp"
)
assign_source_group(${tests_src})

file(GLOB_RECURSE test_protos
	"*.proto"
)
message(STATUS ${test_protos})
proto_generate(test_proto_src ${test_protos})
assign_source_group(${test_proto_src})

add_executable(tests ${tests_src} ${test_proto_src})

target_compile_definitions(tests PRIVATE TESTING=1)
set_target_properties(tests PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

target_link_libraries(tests core protoclib)
