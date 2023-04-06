#ifndef LLP1_BASIC_H
#define LLP1_BASIC_H

#include "../utils/structs_utils/structs_utils.h"
#include "crud_status.h"
#include <unistd.h>
#include <string.h>

enum crud_operation_status move_last_tuple(uint64_t to, size_t tuple_size, FILE* file);

enum crud_operation_status add_new_tuple(struct tuple* tuple, size_t full_tuple_size, uint64_t* tuple_pos, FILE* file);

enum crud_operation_status add_string_tuple(char* string, uint64_t* str_pos, size_t tuple_size, FILE* file);

void get_types(size_t* size, uint32_t** types, FILE* file);

size_t add_to_id_array(uint64_t off, FILE *file);

enum crud_operation_status remove_from_id_array(uint64_t id, uint64_t *off, FILE* file);

enum crud_operation_status convert_id(uint64_t id, uint64_t* off, FILE* file);

enum crud_operation_status convert_offset(uint64_t* id, uint64_t off, FILE* file);

enum crud_operation_status change_string_tuple(char* new_string, uint64_t size, uint64_t off, FILE* file);

enum crud_operation_status set_tuple_strings(struct tuple* tuple, uint64_t tuple_off, FILE* file);

#endif //LLP1_BASIC_H
