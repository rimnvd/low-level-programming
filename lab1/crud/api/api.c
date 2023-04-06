#include "api.h"

size_t add_tuple(uint64_t* fields, uint64_t parent_id, FILE* file) {
    uint32_t* types;
    size_t size;
    get_types(&size, &types, file);
    struct tree_header* header = malloc(sizeof(struct tree_header));
    read_tree_header(header, file);
    struct tuple* new_tuple = get_pointer(sizeof(struct tuple));
    union tuple_header new_tuple_header = {.parent = parent_id, .alloc = header->subheader->current_id};
    new_tuple->header = new_tuple_header;
    new_tuple->data = get_pointer(get_tuple_size(size));
    uint64_t link;
    for (size_t iter = 0; iter < size; iter++) {
        if (types[iter] == STRING_TYPE) {
            add_string_tuple((char*) fields[iter], &link, get_tuple_size(size), file);
            new_tuple->data[iter] = link;
        } else {
            new_tuple->data[iter] = (uint64_t) fields[iter];
        }
    }
    size_t full_tuple_size = sizeof(union tuple_header) + get_tuple_size(size);
    enum crud_operation_status status = add_new_tuple(new_tuple, full_tuple_size, &link, file);
    set_tuple_strings(new_tuple, link, file);
    add_to_id_array(link, file);
    free_tuple(new_tuple);
    free_pointer(types);
    free_tree_header(header);
    return status;
}

static void append_to_result_list(struct tuple** tuple_to_add, struct tuple_result_list** result) {
    if (!(*result)) {
        *result = get_pointer(sizeof(struct tuple_result_list));
        (*result)->previous = NULL;
    } else {
        struct tuple_result_list* new_result = get_pointer(sizeof(struct tuple_result_list));
        new_result->previous = *result;
        *result = new_result;
    }
    (*result)->value = *tuple_to_add;
}

enum crud_operation_status find_by_parent(uint64_t parent_id, struct tuple_result_list** result, FILE* file) {
    struct tree_header* header = get_pointer(sizeof(struct tree_header));
    read_tree_header(header, file);
    struct tuple* current_tuple = get_pointer(sizeof(struct tuple));
    for (size_t i = 0; i < header->subheader->current_id; i++) {
        if (header->sequence_id[i] == NULL_VALUE) {
            continue;
        }
        fseek(file, header->sequence_id[i], SEEK_SET);
        read_default_tuple(header->subheader->pattern_size, &current_tuple, file);
        if (current_tuple->header.parent == parent_id) {
            append_to_result_list(&current_tuple, result);
        }
    }
    free_tree_header(header);
    return 0;
}

enum crud_operation_status find_by_condition(uint64_t field_numb, uint64_t* condition, struct tuple_result_list** result, FILE* file) {
    uint32_t* types;
    size_t size;
    get_types(&size, &types, file);
    uint64_t type = types[field_numb];
    struct tree_header* header = get_pointer(sizeof(struct tree_header));
    read_tree_header(header, file);
    struct tuple* current_tuple = NULL;
    for (size_t i = 0; i < header->subheader->current_id; i++) {
        if (header->sequence_id[i] == NULL_VALUE) continue;
        fseek(file, header->sequence_id[i], SEEK_SET);
        read_default_tuple(size, &current_tuple, file);
        if (type == STRING_TYPE) {
            char* s;
            read_string_from_tuple(current_tuple->data[field_numb], &s, size, file);
            if (!strcmp(s, (char*) condition)) {
                append_to_result_list(&current_tuple, result);
            }
            free_pointer(s);
        } else if (current_tuple->data[field_numb] == *condition) {
            append_to_result_list(&current_tuple, result);
        }
    }
    free_tree_header(header);
    free_pointer(types);
    return 0;
}

enum crud_operation_status get_tuple(uint64_t id, uint64_t** fields, FILE* file) {
    uint64_t offset;
    if (convert_id(id, &offset, file) == CRUD_ERROR) {
        return CRUD_ERROR;
    }
    struct tuple* current_tuple;
    uint32_t* types;
    size_t size;
    get_types(&size, &types, file);
    fseek(file, offset, SEEK_SET);
    read_default_tuple((uint64_t) size, &current_tuple, file);
    *fields = get_pointer(sizeof(uint64_t) * size);
    for (size_t iter = 0; iter < size; iter++) {
        if (types[iter] == STRING_TYPE) {
            char* s;
            read_string_from_tuple(current_tuple->data[iter], &s, size, file);
            memcpy(&(*fields)[iter], &s, sizeof(s));
        } else {
            (*fields)[iter] = current_tuple->data[iter];
        }
    }
    free_tuple(current_tuple);
    free_pointer(types);
    return CRUD_OK;
}

enum crud_operation_status update_tuple(uint64_t* new_value, uint64_t field_number, uint64_t id, FILE* file) {
    uint32_t* types;
    size_t size;
    get_types(&size, &types, file);
    uint64_t type = types[field_number];
    uint64_t offset;
    convert_id(id, &offset, file);
    struct tuple* current_tuple;
    fseek(file, offset, SEEK_SET);
    read_default_tuple(size, &current_tuple, file);
    if (type == STRING_TYPE) {
        change_string_tuple((char*) *new_value, get_tuple_size(size), current_tuple->data[field_number], file);
    } else {
        memcpy(&(current_tuple->data[field_number]), new_value, sizeof(*new_value));
        fseek(file, offset, SEEK_SET);
        write_tuple(get_tuple_size(size), current_tuple, file);
    }
    free_tuple(current_tuple);
    free_pointer(types);
    return 0;
}

enum crud_operation_status remove_tuple(uint64_t id, uint8_t string_flag, FILE* file) {
    uint32_t* types;
    size_t size;
    get_types(&size, &types, file);
    if (!string_flag) {
        uint64_t offset;
        if (remove_from_id_array(id, &offset, file) == CRUD_ERROR) {
            return CRUD_ERROR;
        }
        for (size_t field_num = 0; field_num < size; field_num++) {
            if (types[field_num] == STRING_TYPE) {
                struct tuple* tuple;
                fseek(file, (long) offset, SEEK_SET);
                read_default_tuple(size, &tuple, file);
                remove_tuple(tuple->data[field_num], 1, file);
                free_tuple(tuple);
            }
        }
        move_last_tuple(offset, get_tuple_size(size), file);
        struct tuple_result_list* children = NULL;
        find_by_parent(id, &children, file);
        while (children) {
            remove_tuple(children->tuple_id, 0, file);
            children = children->previous;
        }
    } else {
        struct tuple* str_tuple;
        while (id) {
            fseek(file, id, SEEK_SET);
            read_str_tuple(size, &str_tuple, file);
            move_last_tuple(id, get_tuple_size(size) + sizeof(union tuple_header), file);
            id = str_tuple->header.next;
            free_tuple(str_tuple);
        }
    }
    free_pointer(types);
    return CRUD_OK;
}

enum crud_operation_status find_all(FILE* file, struct tuple_result_list** result) {
    struct tree_header *header = malloc(sizeof(struct tree_header));
    size_t pos;
    read_tree_header(header, file);
    struct tuple* current_tuple;
    for (size_t i = 0; i < header->subheader->current_id; i++){
        if (header->sequence_id[i] == NULL_VALUE) {
            continue;
        }
        fseek(file, header->sequence_id[i], SEEK_SET);
        read_tuple(header->subheader->pattern_size, &current_tuple, file);
        append_to_result_list(&current_tuple, result);
    }
    free_tree_header(header);
    return 0;
}

