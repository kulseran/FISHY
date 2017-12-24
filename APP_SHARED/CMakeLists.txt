cmake_minimum_required (VERSION 2.6)
project (Fishy)

file(GLOB_RECURSE appshared_src
	"*.h"
	"*.inl"
	"*.cpp"
)

add_library(appshared STATIC ${appshared_src})
set_target_properties(appshared PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(appshared core)

assign_source_group(${appshared_src})
