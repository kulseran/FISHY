cmake_minimum_required (VERSION 2.6)
project (Fishy)

file(GLOB_RECURSE game_src
	"*.h"
	"*.inl"
	"*.cpp"
)
assign_source_group(${game_src})

file(GLOB_RECURSE game_protos
	"*.proto"
)
message(STATUS ${game_protos})
proto_generate(game_proto_src ${game_protos})
assign_source_group(${game_proto_src})

add_executable(game ${game_src} ${game_proto_src})
target_link_libraries(game appshared)
