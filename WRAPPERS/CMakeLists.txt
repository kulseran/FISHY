cmake_minimum_required (VERSION 2.6)
project (Fishy)

IF(WIN32)
set(HIDLIB "hid")
ENDIF(WIN32)

file(GLOB_RECURSE wrappers_src
	"*.h"
	"*.inl"
	"*.cpp"
)
assign_source_group(${wrappers_src})

file(GLOB_RECURSE wrappers_protos
	"*.proto"
)
message(STATUS ${wrappers_protos})
proto_generate(wrappers_proto_src ${wrappers_protos})
assign_source_group(${wrappers_proto_src})

add_library(wrappers STATIC ${wrappers_src} ${wrappers_proto_src})
target_link_libraries(wrappers core ${HIDLIB})
