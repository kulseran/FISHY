cmake_minimum_required (VERSION 2.6)
project (Fishy)

file(GLOB_RECURSE bin2h_src
	"*.h"
	"*.inl"
	"*.cpp"
)
assign_source_group(${bin2h_src})

add_executable(bin2h ${bin2h_src})
target_link_libraries(bin2h core)
