cmake_minimum_required (VERSION 2.6)
project (Fishy)

file(GLOB core_src
	"types.h"
	"types.cpp"
	"*/*.h"
	"*/*.inl"
	"*/*.cpp"
)

add_library(core STATIC ${core_src})
set_target_properties(core PROPERTIES LINKER_LANGUAGE CXX)

assign_source_group(${core_src})
