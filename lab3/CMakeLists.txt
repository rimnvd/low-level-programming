cmake_minimum_required(VERSION 3.20)
project(llp3 C)

set(CMAKE_C_STANDARD 99)

include_directories(client)

include_directories(protos)

include_directories(protos/nanopb)

add_executable(llp_3_client
        proto/msg.pb.c
        proto/nanopb/pb_common.c
        proto/nanopb/pb_decode.c
        proto/nanopb/pb_encode.c
        client/lex.yy.c
        client/parser.tab.c
        client/common/common.c client/common/common.h)

add_executable(llp_3_server server/server.c client/common.c
        proto/msg.pb.c
        proto/nanopb/pb_common.c
        proto/nanopb/pb_decode.c
        proto/nanopb/pb_encode.c
        server/utils/file_utils/file_utils.c
        server/utils/structs_utils/structs_utils.c
        server/utils/string_utils/string_utils.c
        server/generator/generator.c
        server/crud/api/api.c
        server/crud/basic/basic.c
        server/commands/commands/commands.c
        server/init.c
        server/user_interface/interactive_mode.c
       )

