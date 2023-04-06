#include "commands.h"

size_t add(char **string, size_t pattern_size, const uint32_t *pattern_types, char** pattern_names, FILE *file) {
    size_t count;
    char** key_value;
    uint64_t fields[pattern_size];
    size_t par_pos = -1;
    if (!isNum(string[1])) {
        printf("id must be numeric\n");
        return 1;
    }
    for (size_t i = 2; i < pattern_size + 2; i++) {
        count = split(string[i], '=', &key_value);
        if (count != 2) {
            return 2;
        }
        for (size_t j = 0; j < pattern_size; j++) {
            if (strcmp(key_value[0], pattern_names[j]) == 0) {
                par_pos = j;
                break;
            }
        }
        if (par_pos == -1) {
            printf("'%s' field must match the pattern\n", string[i]);
            return 3;
        }
        double value;
        switch (pattern_types[par_pos]) {
            case BOOLEAN_TYPE:
                if (strcmp(key_value[1], "True") == 0)
                    fields[par_pos] = true;
                else if (strcmp(key_value[1], "False") == 0) fields[par_pos] = false;
                else {
                    printf("entered value ('%s') must be boolean\n", key_value[1]);
                    return 4;
                }
                break;
            case FLOAT_TYPE:
                value = strtod(key_value[1], NULL);
                if (value == 0.0) {
                    printf("entered value ('%s') must be float\n", key_value[1]);
                    return 4;
                }
                memcpy(&fields[par_pos], &value, sizeof(value));
                break;
            case INTEGER_TYPE:
                if (!isNum(key_value[1])) {
                    printf("entered value ('%s') must be integer\n", key_value[1]);
                    return 4;
                }
                fields[par_pos] = strtol(key_value[1], NULL, 10);
                break;
            case STRING_TYPE:
                fields[par_pos] = (uint64_t) key_value[1];
                break;
        }
        par_pos = -1;
        free_pointer(key_value);
    }
    add_tuple(fields, strtol(string[1], NULL, 10), file);
    return 0;
}

size_t add_new(FILE* file, Query_tree_Value_setting* settings, size_t settings_count, uint64_t parent_id, size_t pattern_size,
               const uint32_t* pattern_types, char** pattern_names) {
    uint64_t fields[pattern_size];
    size_t par_pos = -1;
    for (int s_idx = 0; s_idx < settings_count; ++s_idx) {
        for (size_t in_iter = 0; in_iter < pattern_size; in_iter++) {
            if (strcmp(settings[s_idx].fv.field, pattern_names[in_iter]) == 0) {
                par_pos = in_iter;
                break;
            }
        }
        if (par_pos == -1) {
            return PATTERN_ERROR;
        }
        double val;
        switch (pattern_types[par_pos]) {
            case BOOLEAN_TYPE:
                if (settings[s_idx].fv.int_val == 1) fields[par_pos] = true;
                else if (settings[s_idx].fv.int_val == 0) fields[par_pos] = false;
                else {
                    return BOOL_ERROR;
                }
                break;
            case FLOAT_TYPE:
                if (settings[s_idx].fv.val_type != 2) {
                    return FLOAT_ERROR;
                }
                val = settings[s_idx].fv.real_val;
                memcpy(&fields[par_pos], &val, sizeof(val));
                break;
            case INTEGER_TYPE:
                if (settings[s_idx].fv.val_type != 1) {
                    return INT_ERROR;
                }
                fields[par_pos] = settings[s_idx].fv.int_val;
                break;
            case STRING_TYPE:
                if (settings[s_idx].fv.val_type != 0) return {
                    STR_ERROR;
                }
                fields[par_pos] = (uint64_t) settings[s_idx].fv.str_val;
                break;
        }
        par_pos = -1;
    }
    if (settings_count == pattern_size) {
        add_tuple(fields, parent_id, file);
    }
    else {
        return COUNT_ERROR;
    }
    return OK;
}

void find_by(char **array, size_t pattern_size, const uint32_t *pattern_types, char **pattern_names, size_t count, FILE *file) {
    struct tuple_result_list* result = NULL;
    if (strcmp(array[1], "parent") == 0) {
        if (count == 3) {
            if (isNum(array[2])) {
                find_by_parent(atoi(array[2]), &result, file);
            } else {
                printf("parent id (%s) must be integer\n", array[2]);
            }
        } else {
            printf("%lu arguments entered, 3 expected\n", count - 1);
        }
    } else if (strcmp(array[1], "field") == 0) {
        if (count == 4) {
            int field_index = -1;
            for (int i = 0; i < pattern_size; i++) {
                if (strcmp(pattern_names[i], array[2]) == 0) {
                    field_index = i;
                }
            }
            if (field_index == -1)
                printf("no such field exists (%s)\n", array[2]);
            else {
                uint32_t type = pattern_types[field_index];
                switch (type) {
                    case BOOLEAN_TYPE:
                        if (strcmp(array[3], "True") == 0) {
                            bool condition = true;
                            find_by_field(field_index, (uint64_t *) &condition, &result, file);
                        } else if (strcmp(array[3], "False") == 0) {
                            bool condition = false;
                            find_by_field(field_index, (uint64_t *) &condition, &result, file);
                        } else
                            printf("value (%s) must be boolean\n", array[3]);
                        break;
                    case INTEGER_TYPE:
                        if (isNum(array[3])) {
                            uint64_t condition = atoi(array[3]);
                            find_by_field(field_index, &condition, &result, file);
                        } else
                            printf("value (%s) must be integer\n", array[3]);
                        break;
                    case FLOAT_TYPE:
                        if (strtod(array[3], NULL) != 0) {
                            double temp_condition = strtod(array[3], NULL);
                            uint64_t condition;
                            memcpy(&condition, &temp_condition, sizeof(temp_condition));
                            find_by_field(field_index, &condition, &result, file);
                        } else
                            printf("value (%s) must be float\n", array[3]);
                        break;
                    case STRING_TYPE:
                        find_by_field(field_index, (uint64_t *) array[3], &result, file);
                        break;
                    default:
                        printf("entered type is unknown\n");
                }
            }
        } else printf("%lu arguments entered, 3 expected\n", count - 1);
    } else if (strcmp(array[1], "id") == 0) {
        if (count == 3) {
            if (isNum(array[2])) {
                uint64_t id = atoi(array[2]);
                struct tree_header* header = get_pointer(sizeof(struct tree_header));
                read_tree_header(header, file);
                uint64_t* fields;
                if (header->subheader->current_id < id) {
                    printf("id is too large\n");
                    free_tree_header(header);
                }
                enum crud_operation_status status = get_tuple(id, &fields, file);
                if (status) {
                    printf("there is no result\n");
                    free_tree_header(header);
                }
                for (size_t i = 0; i < header->subheader->pattern_size; i++) {
                    double float_value;
                    switch (header->pattern[i]->header->type) {
                        case INTEGER_TYPE:
                            printf("%-20s: %ld\n", header->pattern[i]->key_value, fields[i]);
                            break;
                        case BOOLEAN_TYPE:
                            printf("%-20s: %d\n", header->pattern[i]->key_value, fields[i] != 0);
                            break;
                        case FLOAT_TYPE:
                            memcpy(&float_value, &(fields[i]), sizeof(fields[i]));
                            printf("%-20s: %lf\n", header->pattern[i]->key_value, float_value);
                            break;
                        default:
                            printf("%-20s: %s\n", header->pattern[i]->key_value, (char *) fields[i]);
                            free_pointer((char*) fields[i]);
                            break;
                    }
                }
                free_pointer(fields);
                free_tree_header(header);
            } else {
                printf("id (%s) must be integer\n", array[2]);
            }
        } else {
            printf("%lu arguments entered, 3 expected\n", count - 1);
        }
    } else {
        printf("only [find_by id]/[find_by parent]/[find_by field] are allowed\n");
    }
    if (result != NULL) {
        printf("result:\n");
        do {
            printf("id: %lu\n", (uint64_t) result->tuple_id);
            result = result->previous;
        } while (result != NULL);
    } else if (strcmp(array[1], "id") != 0) {
        printf("there is no result\n");
    }
}

size_t update(FILE* file, uint64_t id, Query_tree_Value_setting* settings, size_t settings_count, size_t pattern_size, const uint32_t* pattern_types, char** pattern_names) {
    int8_t par_pos = -1;
    uint64_t value;
    for (int s_idx = 0; s_idx < settings_count; ++s_idx) {
        for (size_t in_iter = 0; in_iter < pattern_size; in_iter++) {
            if (strcmp(settings[s_idx].fv.field, pattern_names[in_iter]) == 0) {
                par_pos = in_iter;
                break;
            }
        }
        if (par_pos == -1) {
            printf("'%s' field must match the pattern\n", settings->fv.field);
            return 3;
        }
        double double_val;
        switch (pattern_types[par_pos]) {
            case BOOLEAN_TYPE:
                if (settings[s_idx].fv.int_val == 1) {
                    value = true;
                }
                else if (settings[s_idx].fv.int_val == 0) {
                    value = false;
                }
                else {
                    printf("'%lu' must be bool\n", settings[s_idx].fv.int_val);
                    return 4;
                }
                break;
            case FLOAT_TYPE:
                if (settings[s_idx].fv.val_type != 2) {
                    printf("'%file' must be float\n", settings[s_idx].fv.real_val);
                    return 4;
                }
                double_val = settings[s_idx].fv.real_val;
                memcpy(&value, &double_val, sizeof(double_val));
                break;
            case INTEGER_TYPE:
                if (settings[s_idx].fv.val_type != 1) {
                    printf("'%lu' must be integer\n", settings[s_idx].fv.int_val);
                    return 4;
                }
                value = settings[s_idx].fv.int_val;
                break;
            case STRING_TYPE:
                if (settings[s_idx].fv.val_type != 0) {
                    printf("'%s' must be string\n", settings[s_idx].fv.str_val);
                    return 4;
                }
                char* temp = malloc(64);
                strcpy(temp, settings[s_idx].fv.str_val);
                value = (uint64_t) temp;
                break;
        }
        update_tuple(&value, par_pos, id, file);
        par_pos = -1;
    }
    return 0;
}


