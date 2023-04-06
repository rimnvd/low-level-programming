#ifndef LLP1_API_H
#define LLP1_API_H

#include "../../utils/structs_utils/structs_utils.h"
#include "../../utils/string_utils/string_utils.h"
#include "../basic/basic.h"
#include "../crud_status.h"
#include "msg.pb.h"

struct tuple_result_list {
    struct tuple* value;
    uint64_t tuple_id;
    struct tuple_result_list* previous;
};

size_t add_tuple(uint64_t* fields, uint64_t parent_id, FILE* file);

enum crud_operation_status find_by_parent(uint64_t parent_id, struct tuple_result_list** result, FILE* file);

enum crud_operation_status find_by_field(uint64_t field_numb, uint64_t* condition, struct tuple_result_list** result, FILE* file);

enum crud_operation_status get_tuple(uint64_t id, uint64_t** fields, FILE* file);

enum crud_operation_status update_tuple(uint64_t* new_value, uint64_t field_number, uint64_t id, FILE* file);

size_t find_by_conditions(FILE* file, Query_tree_Filter* filters, size_t filters_count, struct tuple_result_list** result,
                          char** pattern_names);

enum crud_operation_status remove_tuple(uint64_t id, uint8_t string_flag, FILE* file);

void print_tree_header(FILE* file);

void print_tuple_array(FILE* file);

#endif //LLP1_API_H
