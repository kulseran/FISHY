cmake_minimum_required (VERSION 2.6)
project (Fishy)

file(GLOB_RECURSE bmfontutil_src
	"*.h"
	"*.inl"
	"*.cpp"
)
assign_source_group(${bmfontutil_src})
filter_items(bmfontutil_src ".*main.cpp")

file(GLOB_RECURSE bmfontutil_protos
	"*.proto"
)
message(STATUS ${bmfontutil_protos})
proto_generate(bmfontutil_proto_src ${bmfontutil_protos})
assign_source_group(${bmfontutil_proto_src})

add_library(bmfontprotolib ${bmfontutil_proto_src})
add_library(bmfontutillib ${bmfontutil_src} ${bmfontutil_proto_src})
add_executable(bmfontutil main.cpp ${bmfontutil_src} ${bmfontutil_proto_src})
target_link_libraries(bmfontutil appshared)
