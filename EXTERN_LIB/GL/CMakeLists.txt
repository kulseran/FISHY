cmake_minimum_required (VERSION 2.6)
project (Fishy)

file(GLOB_RECURSE glew_src
	"*.h"
	"glew.c"
)

add_library(glew STATIC ${glew_src})
set_target_properties(glew PROPERTIES LINKER_LANGUAGE CXX)
target_compile_definitions(glew PRIVATE GLEW_STATIC _LIB)

assign_source_group(${glew_src})
