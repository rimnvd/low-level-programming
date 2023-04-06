cmake_minimum_required(VERSION 3.20)
project(llp1 C)

set(CMAKE_C_STANDARD 99)

add_executable(llp1 main.c utils/file_utils/file_utils.c utils/file_utils/file_utils.h utils/string_utils/string_utils.c utils/string_utils/string_utils.h
        structs/tree_header.h utils/structs_utils/structs_utils.c utils/structs_utils/structs_utils.h structs/tuple.h
        generator/generator.c generator/generator.h configuration.h crud/crud_status.h
        commands/commands.c commands/commands.h crud/interface.c crud/interface.h crud/basic.c crud/basic.h utils/string_utils/string_utils.c utils/string_utils/string_utils.h)
