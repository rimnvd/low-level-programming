#include "basic.h"

enum crud_operation_status move_tuple(uint64_t from, uint64_t to, size_t tuple_size, FILE* file) {
    uint32_t* types;
    size_t size;
    get_types(&size, &types, file);
    if (from != to) {
        fseek(file, from, SEEK_SET);
        void* buffer = get_pointer(tuple_size);
        read_from_file(buffer, file, tuple_size);
        fseek(file, to, SEEK_SET);
        write_to_file(buffer, file, tuple_size);
        free_pointer(buffer);
        fseek(file, 0, SEEK_SET);
        struct tree_header* header = get_pointer(sizeof(struct tree_header));
        read_tree_header(header, file);
        uint64_t id;
        struct tuple* tuple;
        enum crud_operation_status operation_status = convert_offset(&id, from, file);
        if (operation_status == CRUD_ERROR) {
            fseek(file, from, SEEK_SET);
            read_str_tuple(size, &tuple, file);
            union tuple_header* temp_header = get_pointer(sizeof(union tuple_header));
            if (tuple->header.next != 0) {
                fseek(file, tuple->header.next, SEEK_SET);
                read_from_file(temp_header, file, sizeof(union tuple_header));
                temp_header->previous = to;
                fseek(file, tuple->header.next, SEEK_SET);
                write_to_file(temp_header, file, sizeof(union tuple_header));
            }
            fseek(file, tuple->header.previous, SEEK_SET);
            read_from_file(temp_header, file, sizeof(union tuple_header));
            if (temp_header->next == from) {
                temp_header->next = to;
                fseek(file, tuple->header.previous, SEEK_SET);
                write_to_file(temp_header, file, sizeof(union tuple_header));
            } else {
                struct tuple* parent;
                fseek(file, tuple->header.previous, SEEK_SET);
                read_default_tuple(size, &parent, file);
                for (size_t iter = 0; iter < size; iter++) {
                    if (types[iter] == STRING_TYPE && parent->data[iter] == from) {
                        parent->data[iter] = to;
                        break;
                    }
                }
                fseek(file, (tuple->header.previous), SEEK_SET);
                write_tuple(get_tuple_size(size), parent, file);
                free_tuple(parent);
            }
            free_pointer(temp_header);
        } else {
            fseek(file, from, SEEK_SET);
            read_default_tuple(size, &tuple, file);
            set_tuple_strings(tuple, to, file);
            header->sequence_id[id] = to;
            write_tree_header(header, file);
        }
        free_tuple(tuple);
        free_tree_header(header);
    }
    free_pointer(types);
    return CRUD_OK;
}

enum crud_operation_status move_last_tuple(uint64_t to, size_t tuple_size, FILE* file) {
    uint32_t* types;
    size_t size;
    get_types(&size, &types, file);
    fseek(file, (long) -(get_tuple_size(size) + sizeof(union tuple_header)), SEEK_END);
    uint64_t pos_from = ftell(file);
    free_pointer(types);
    enum crud_operation_status operation_status = move_tuple(pos_from, to, tuple_size, file);
    ftruncate(fileno(file), (long) pos_from);
    return operation_status;
}

enum crud_operation_status add_new_tuple(struct tuple* tuple, size_t full_tuple_size, uint64_t* tuple_pos, FILE* file) {
    fseek(file, 0, SEEK_END);
    *tuple_pos = ftell(file);
    int fd = fileno(file);
    ftruncate(fd, ftell(file) + full_tuple_size);
    enum write_file_status status = write_tuple(full_tuple_size - sizeof(union tuple_header), tuple, file);
    return status == WRITE_OK ? CRUD_OK : CRUD_ERROR;
}

enum crud_operation_status add_str_tuple(char* string, uint64_t* str_position, size_t tuple_size, FILE* file) {
    size_t len = strlen(string);
    size_t cnt = len / tuple_size + (len % tuple_size ? 1 : 0);
    struct tuple* temp_tuple = get_pointer(sizeof(struct tuple));
    char* temp_tuple_content = string;
    size_t position = (size_t) ftell(file);
    uint64_t fake_position;
    fseek(file, 0, SEEK_END);
    *str_position = ftell(file);
    for (size_t i = 0; cnt > i; i++) {
        if (cnt - 1 == i) temp_tuple->header.next = 0;
        else temp_tuple->header.next = position + (tuple_size + sizeof(union tuple_header)) * (i + 1);
        if (0 == i) temp_tuple->header.previous = 0;
        else temp_tuple->header.previous = position + (tuple_size + sizeof(union tuple_header)) * (i - 1);
        temp_tuple->data = (uint64_t*) (temp_tuple_content + tuple_size* i);
        add_new_tuple(temp_tuple, tuple_size + sizeof(union tuple_header), &fake_position, file);
    }
    free_pointer(temp_tuple);
    return 0;
}

enum crud_operation_status set_tuple_strings(struct tuple*tuple, uint64_t tuple_offset, FILE*file) {
    uint32_t* types;
    size_t size;
    get_types(&size, &types, file);
    struct tuple* string;
    for (uint64_t i = 0; i < size; i++) {
        if (types[i] == STRING_TYPE) {
            fseek(file, tuple->data[i], SEEK_SET);
            read_str_tuple(size, &string, file);
            string->header.previous = tuple_offset;
            fseek(file, tuple->data[i], SEEK_SET);
            write_tuple(size, string, file);
            free_tuple(string);
        }
    }
    free_pointer(types);
    return CRUD_OK;
}

void get_types(size_t* size, uint32_t** types, FILE* file) {
    fseek(file, 0, SEEK_SET);
    struct tree_header* header = get_pointer(sizeof(struct tree_header));
    read_tree_header(header, file);
    uint32_t* temp_types = get_pointer(header->subheader->pattern_size* sizeof(uint32_t));
    for (size_t i = 0; i < header->subheader->pattern_size; i++) {
        temp_types[i] = header->pattern[i]->header->type;
    }
    *types = temp_types;
    *size = header->subheader->pattern_size;
    free_tree_header(header);
}


size_t add_to_id_array(uint64_t offset, FILE* file) {
    size_t id;
    struct tree_header* header = get_pointer(sizeof(struct tree_header));
    read_tree_header(header, file);
    uint64_t from = ftell(file);
    uint64_t tuple_size = get_id_array_size(header->subheader->current_id, header->subheader->pattern_size);
    if (!((header->subheader->current_id + 1) % tuple_size)) {
        fseek(file, 0, SEEK_END);
        uint64_t current_end = ftell(file);
        ftruncate(fileno(file),
                  current_end + get_tuple_size(header->subheader->pattern_size) + sizeof(union tuple_header));
        move_tuple(from, current_end, get_tuple_size(header->subheader->pattern_size), file);
        free_tree_header(header);
        header = get_pointer(sizeof(struct tree_header));
        read_tree_header(header, file);
    }
    header->sequence_id[header->subheader->current_id] = offset;
    header->subheader->current_id++;
    id = header->subheader->current_id - 1;
    fseek(file, 0, SEEK_SET);
    if (write_tree_header(header, file) != WRITE_OK) printf("WRITE ERROR\n");
    free_tree_header(header);
    return id;
}


enum crud_operation_status remove_from_id_array(uint64_t id, uint64_t* offset, FILE* file) {
    fseek(file, 0, SEEK_SET);
    struct tree_header* header = get_pointer(sizeof(struct tree_header));
    read_tree_header(header, file);
    if (header->sequence_id[id] == 0) {
        free_tree_header(header);
        return CRUD_ERROR;
    } else {
        *offset = header->sequence_id[id];
        if (header->subheader->current_id - 1 == id) header->subheader->current_id--;
        header->sequence_id[id] = 0;
        write_tree_header(header, file);
        free_tree_header(header);
        return CRUD_OK;
    }
}

enum crud_operation_status convert_id(uint64_t id, uint64_t* offset, FILE* file) {
    fseek(file, 0, SEEK_SET);
    struct tree_header* header = get_pointer(sizeof(struct tree_header));
    read_tree_header(header, file);
    if (header->sequence_id[id] == NULL_VALUE) {
        free_tree_header(header);
        return CRUD_ERROR;
    } else {
        *offset = header->sequence_id[id];
        free_tree_header(header);
        return CRUD_OK;
    }
}

enum crud_operation_status convert_offset(uint64_t* id, uint64_t offset, FILE* file) {
    struct tree_header* header = get_pointer(sizeof(struct tree_header));
    read_tree_header(header, file);
    struct tuple* tuple;
    fseek(file, offset, SEEK_SET);
    read_default_tuple(header->subheader->pattern_size, &tuple, file);
    if (header->sequence_id[tuple->header.alloc] == offset) {
        *id = tuple->header.alloc;
        free_tree_header(header);
        free_tuple(tuple);
        return CRUD_OK;
    } else {
        free_tree_header(header);
        free_tuple(tuple);
        return CRUD_ERROR;
    }
}

enum crud_operation_status change_string_tuple(char* new_string, uint64_t size, uint64_t offset, FILE* file) {
    struct tuple* current_tuple = NULL;
    uint64_t length = strlen(new_string);
    uint64_t prev_offset = offset;
    do {
        offset = prev_offset;
        fseek(file, offset, SEEK_SET);
        read_str_tuple(size, &current_tuple, file);
        fseek(file, offset, SEEK_SET);
        current_tuple->data = (uint64_t*) (new_string);
        new_string = new_string + size;
        write_tuple(size, current_tuple, file);
        prev_offset = current_tuple->header.next;
        if (length > size) {
            length -= size;
        }
        else length = 0;
    } while (current_tuple->header.next && length > 0);
    uint64_t fpos;
    if (length > 0) {
        add_str_tuple(new_string, &fpos, size, file);
        current_tuple->header.next = fpos;
        fseek(file, offset, SEEK_SET);
        write_tuple(size, current_tuple, file);
    }
    return CRUD_OK;
}
