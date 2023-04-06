#ifndef LLP1_STRUCTS_UTILS_H
#define LLP1_STRUCTS_UTILS_H

#include <stdlib.h>
#include <string.h>

#include "../../structs/tree_header.h"
#include "../file_utils/file_utils.h"
#include "../../structs/tuple.h"
#include "../../configuration.h"
#include "../../generator/generator.h"

size_t get_tuple_size(uint64_t pattern_size);

size_t get_id_array_size(uint64_t current_id, uint64_t pattern_size);

enum write_file_status write_tuple(size_t tuple_size, struct tuple* tuple, FILE* file);

enum read_file_status read_str_tuple(uint64_t pattern_size, struct tuple** tuple, FILE* file);

enum read_file_status read_default_tuple(uint64_t pattern_size, struct tuple** tuple, FILE* file);

enum read_file_status read_tuple(uint64_t pattern_size, struct tuple** tuple, FILE* file);

enum read_file_status read_string_from_tuple(uint64_t offset, char** string, uint64_t pattern_size, FILE* file);

enum write_file_status init_empty_file(FILE* file, char** pattern, size_t pattern_size, uint32_t* types, size_t* key_sizes);

enum write_file_status write_tree_header(struct tree_header* header, FILE* file);

enum read_file_status read_tree_header(struct tree_header* header, FILE* file);

void* get_pointer(size_t size);

void free_pointer(void* pointer);

void free_tuple(struct tuple* tuple);

void free_tree_header(struct tree_header* header);

#endif //LLP1_STRUCTS_UTILS_H
