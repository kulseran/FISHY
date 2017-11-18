cmake_minimum_required (VERSION 2.6)
project (Fishy)

file(GLOB core_src
	"*.h"
	"*.inl"
	"*.cpp"
)

add_library(core STATIC ${core_src})
