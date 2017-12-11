cmake_minimum_required (VERSION 2.6)
project (Fishy)

file(GLOB_RECURSE protoc_src
	"*.h"
	"*.inl"
	"*.cpp"
)
assign_source_group(${protoc_src})

filter_items(protoc_src ".*main.cpp")

add_library(protoclib ${protoc_src})
add_executable(protoc main.cpp ${protoc_src})
target_link_libraries(protoc core)
