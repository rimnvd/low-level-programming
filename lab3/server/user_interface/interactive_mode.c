#include "interactive_mode.h"

void handle_query(FILE* file, Query_tree tree, char** response_) {
    char* response = "";
    char tmp[256];
    size_t pattern_size;
    size_t err_code;
    struct tree_header* header = get_pointer(sizeof(struct tree_header));
    read_tree_header(header, file);
    pattern_size = header->subheader->pattern_size;
    uint32_t* pattern_types = get_pointer(sizeof(uint32_t) * pattern_size);
    char** pattern_names = get_pointer(sizeof(char *) * pattern_size);
    for (int i = 0; i < pattern_size; i++) {
        pattern_types[i] = header->pattern[i]->header->type;
        pattern_names[i] = header->pattern[i]->key_value;
    }
    struct tuple_result_list* result = NULL;
    switch (tree.command) {
        case 0:
            err_code = find_by_conditions(file, tree.filters, tree.filters_count, &result, pattern_names);
            if (err_code == 3) {
                response = concat_strings(response, "at least one of your fields doesn't exist\n");
                break;
            }
            if (result) {
                response = concat_strings(response, "result:\n");
                do {
                    sprintf(tmp, "tuple %3zu:\n", result->tuple_id);
                    response = concat_strings(response, tmp);
                    for (size_t iter = 0; iter < pattern_size; iter++) {
                        if (pattern_types[iter] == STRING_TYPE) {
                            char* s;
                            if (tree.filters_count == 1 && tree.filters[0].comp_list_count == 1 &&
                                strcmp(tree.filters[0].comp_list[0].fv.field, "id") == 0) {
                                s = (char*) result->value->data[iter];
                            } else {
                                read_str_from_tuple(result->value->data[iter], &s, pattern_size, file);
                            }
                            sprintf(tmp, "%-20s %s\n", pattern_names[iter], s);
                            response = concat_strings(response, tmp);
                            free_pointer(s);
                        } else if (pattern_types[iter] == INTEGER_TYPE || pattern_types[iter] == BOOLEAN_TYPE) {
                            sprintf(tmp, "%-20s %lu\n", pattern_names[iter], result->value->data[iter]);
                            response = concat_strings(response, tmp);
                        } else if (pattern_types[iter] == FLOAT_TYPE) {
                            double res;
                            memcpy(&res, &(result->value->data[iter]), sizeof(result->value->data[iter]));
                            sprintf(tmp, "%-20s %.6f\n", pattern_names[iter], res);
                            response = concat_strings(response, tmp);
                        }
                    }
                    result = result->previous;
                } while (result);
            } else {
                response = concat_strings(response, "no results\n");
            }
            break;
        case 1:
            err_code = find_by_conditions(file, tree.filters, tree.filters_count, &result, pattern_names);
            if (err_code == 3) {
                response = concat_strings(response, "at least one of your fields doesn't exist\n");
                break;
            }
            if (result) {
                response = concat_strings(response, "remove result:\n");
                do {
                    if (remove_tuple(result->tuple_id, 0, file) == CRUD_ERROR) {
                        response = concat_strings(response, "already removed ");
                    } else {
                        response = concat_strings(response, "removed successfully ");
                    }
                    sprintf(tmp, "id: %lu\n", (uint64_t) result->tuple_id);
                    response = concat_strings(response, tmp);
                    result = result->previous;
                } while (result != NULL);
            } else {
                response = concat_strings(response, "no results\n");
            }
            break;
        case 2:
            err_code = add_new(file, tree.settings, tree.settings_count,
                               tree.filters[0].comp_list[0].fv.int_val,
                               pattern_size, pattern_types, pattern_names);
            switch (err_code) {
                case 1:
                    sprintf(tmp, "fields must match the pattern\n");
                    response = concat_strings(response, tmp);
                    break;
                case 2:
                    sprintf(tmp, "value must be bool\n");
                    response = concat_strings(response, tmp);
                    break;
                case 3:
                    sprintf(tmp, "value must be float\n");
                    response = concat_strings(response, tmp);
                    break;
                case 4:
                    sprintf(tmp, "value must be int\n");
                    response = concat_strings(response, tmp);
                    break;
                case 5:
                    sprintf(tmp, "value must be string\n");
                    response = concat_strings(response, tmp);
                    break;
                case 6:
                    sprintf(tmp, "wrong number of params\n");
                    response = concat_strings(response, tmp);
                    break;
                default:
                    sprintf(tmp, "added successfully\n");
                    response = concat_strings(response, tmp);
            }
            break;
        case 3:
            err_code = find_by_conditions(file, tree.filters, tree.filters_count, &result, pattern_names);
            if (err_code == 3) {
                response = concat_strings(response, "at least one of your fields doesn't exist");
                break;
            }
            if (result) {
                response = concat_strings(response, "update result\n");
                do {
                    err_code = update(file, result->tuple_id, tree.settings, tree.settings_count, pattern_size, pattern_types, pattern_names);
                    if (err_code != 0) {
                        sprintf(tmp, "error code: %zu\n", err_code);
                        response = concat_strings(response, tmp);
                    }
                    sprintf(tmp, "updated id: %lu\n", (uint64_t) result->tuple_id);
                    response = concat_strings(response, tmp);
                    result = result->previous;
                } while (result);
            } else {
                response = concat_strings(response, "no results\n");
            }
            break;
    }
    *response_ = response;
    free_tree_header(header);
    free_pointer(pattern_names);
    free_pointer(pattern_types);
}
