#ifndef LLP1_TREE_HEADER_H
#define LLP1_TREE_HEADER_H

#include <inttypes.h>
#include <stdio.h>
#include "../utils/file_utils/file_utils.h"
#pragma once

struct tree_subheader {
    uint64_t ASCII_signature;
    uint64_t root_offset;
    uint64_t first_seq;
    uint64_t second_seq;
    uint64_t current_id;
    uint64_t pattern_size;
};

#pragma pack(push, 4)
struct key_header {
    uint32_t size;
    uint32_t type;
};
struct key {
    struct key_header* header;
    char* key_value;
};
#pragma pack(pop)

struct tree_header {
    struct tree_subheader* subheader;
    struct key** pattern;
    uint64_t* sequence_id;
};

#endif //LLP1_STRUCTS_H
