cmake_minimum_required (VERSION 2.6)
project (Fishy)

file(GLOB_RECURSE game_client_src
	"*.h"
	"*.inl"
	"*.cpp"
)
assign_source_group(${game_client_src})

file(GLOB_RECURSE game_client_protos
	"*.proto"
)
proto_generate(game_client_proto_src ${game_client_protos})
assign_source_group(${game_client_proto_src})

add_executable(game_client ${game_client_src} ${game_client_proto_src})
target_link_libraries(game_client appshared wrappers)

