cmake_minimum_required (VERSION 2.6)
project (Fishy)

file(GLOB_RECURSE templategen_src
	"*.h"
	"*.inl"
	"*.cpp"
)
assign_source_group(${templategen_src})

add_executable(templategen ${templategen_src})
