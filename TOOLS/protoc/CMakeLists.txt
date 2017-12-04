cmake_minimum_required (VERSION 2.6)
project (Fishy)

file(GLOB_RECURSE protoc_src
	"*.h"
	"*.inl"
	"*.cpp"
)
assign_source_group(${protoc_src})

add_executable(bin2h ${protoc_src})
target_link_libraries(bin2h core)
