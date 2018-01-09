cmake_minimum_required (VERSION 2.6)
project (Fishy)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)

file(GLOB_RECURSE core_src
	"*.h"
	"*.inl"
	"*.cpp"
)

add_library(core STATIC ${core_src})
set_target_properties(core PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(core Threads::Threads ${OPENGL_LIBRARY} glew)

assign_source_group(${core_src})
