cmake_minimum_required (VERSION 2.6)
project (Fishy)

file(GLOB_RECURSE game_server_src
	"*.h"
	"*.inl"
	"*.cpp"
)
assign_source_group(${game_server_src})

file(GLOB_RECURSE game_server_protos
	"*.proto"
)
proto_generate(game_server_proto_src ${game_server_protos})
assign_source_group(${game_server_proto_src})

add_executable(game_server ${game_server_src} ${game_server_proto_src})
target_link_libraries(game_server appshared)
