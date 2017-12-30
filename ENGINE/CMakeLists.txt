cmake_minimum_required (VERSION 2.6)
project (Fishy)

file(GLOB_RECURSE engine_src
	"*.h"
	"*.inl"
	"*.cpp"
)

add_library(engine STATIC ${engine_src})
set_target_properties(engine PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(engine core)

assign_source_group(${engine_src})
