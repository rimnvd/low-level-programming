#include "structs_utils.h"

size_t get_tuple_size(uint64_t pattern_size) {
    return pattern_size * SINGLE_TUPLE_VALUE_SIZE < MINIMAL_TUPLE_SIZE ? MINIMAL_TUPLE_SIZE : pattern_size * SINGLE_TUPLE_VALUE_SIZE;
}

static uint64_t max(uint64_t first, uint64_t second) {
    return first > second ? first : second;
}

size_t get_id_array_size(uint64_t current_id, uint64_t pattern_size) {
    size_t tuple_size = get_tuple_size(pattern_size) + sizeof(union tuple_header);
    if (current_id == 0) {
        current_id++;
    }
    size_t whole = (current_id* OFFSET_VALUE_SIZE / tuple_size);
    size_t fractional = (current_id* OFFSET_VALUE_SIZE % tuple_size ? 1 : 0);
    size_t size = max((whole + fractional) * tuple_size / OFFSET_VALUE_SIZE,MIN_ID_ARRAY_SIZE * tuple_size / OFFSET_VALUE_SIZE);
    return size;
}

static enum read_file_status read_tree_subheader(struct tree_subheader* subheader, FILE* file) {
    return read_from_file(subheader, file, sizeof(struct tree_subheader));
}

static enum read_file_status read_key(struct key* key, FILE* file) {
    struct key_header* header = get_pointer(sizeof(struct key_header));
    enum read_file_status status = read_from_file(header, file, sizeof(struct key_header));
    key->header = header;
    char* key_value = (char*) get_pointer(
            header->size / FILE_GRANULARITY + (header->size % FILE_GRANULARITY ? FILE_GRANULARITY : 0));
    status |= read_from_file(key_value, file, header->size);
    key->key_value = key_value;
    return status;
}

enum read_file_status read_tree_header(struct tree_header* header, FILE* file) {
    fseek(file, 0, SEEK_SET);
    struct tree_subheader* subheader = get_pointer(sizeof(struct tree_subheader));
    enum read_file_status status = read_tree_subheader(subheader, file);
    header->subheader = subheader;
    struct key** key_array = get_pointer(sizeof(struct key*) * subheader->pattern_size);
    header->pattern = key_array;
    for (size_t i = subheader->pattern_size; i-- > 0; key_array++) {
        struct key* element_pattern = get_pointer(sizeof(struct key));
        status |= read_key(element_pattern, file);
        *key_array = element_pattern;
    }
    size_t id_array_size = get_id_array_size(header->subheader->current_id, header->subheader->pattern_size);
    uint64_t* id_array = (uint64_t*) get_pointer(id_array_size * sizeof(uint64_t));
    header->sequence_id = id_array;
    status |= read_from_file(id_array, file, id_array_size * sizeof(uint64_t));
    return status;
}

enum read_file_status read_default_tuple(uint64_t pattern_size, struct tuple** tuple, FILE* file) {
    union tuple_header* header = get_pointer(sizeof(union tuple_header));
    enum read_file_status status = read_from_file(header, file, sizeof(union tuple_header));
    struct tuple* temp_tuple = get_pointer(sizeof(struct tuple));
    temp_tuple->header = *header;
    free_pointer(header);
    uint64_t* data = get_pointer(get_tuple_size(pattern_size));
    status |= read_from_file(data, file, get_tuple_size(pattern_size));
    temp_tuple->data = data;
    *tuple = temp_tuple;
    return status;
}

enum read_file_status read_str_tuple(uint64_t pattern_size, struct tuple** tuple, FILE* file) {
    union tuple_header* header = get_pointer(sizeof(union tuple_header));
    enum read_file_status status = read_from_file(header, file, sizeof(union tuple_header));
    struct tuple* temp_tuple = get_pointer(sizeof(struct tuple));
    temp_tuple->header = *header;
    free_pointer(header);
    uint64_t* data = (uint64_t*) get_pointer(get_tuple_size(pattern_size));
    status |= read_from_file(data, file, get_tuple_size(pattern_size));
    temp_tuple->data = data;
    *tuple = temp_tuple;
    return status;
}

static size_t get_string_len(uint64_t offset, FILE* file) {
    fseek(file, offset, SEEK_SET);
    size_t length = 1;
    union tuple_header* temp_header = get_pointer(sizeof(union tuple_header));
    read_from_file(temp_header, file, sizeof(union tuple_header));
    while (temp_header->next) {
        fseek(file, temp_header->next, SEEK_SET);
        read_from_file(temp_header, file, sizeof(union tuple_header));
        length++;
    }
    free_pointer(temp_header);
    return length;
}

enum read_file_status read_str_from_tuple(uint64_t offset, char** string, uint64_t pattern_size, FILE* file) {
    size_t length = get_string_len(offset, file);
    size_t tuple_size = get_tuple_size(pattern_size);
    *string = get_pointer(length * tuple_size);
    struct tuple* temp_header;
    for (size_t iter = 0; iter < length; iter++) {
        fseek(file, offset, SEEK_SET);
        read_str_tuple(pattern_size, &temp_header, file);
        offset = temp_header->header.next;
        strncpy((*string) + tuple_size * iter, (char *) temp_header->data, tuple_size);
        free_tuple(temp_header);
    }
    return 0;
}

enum write_file_status write_tree_subheader(struct tree_subheader* tree_subheader, FILE* file) {
    return write_to_file(tree_subheader, file, sizeof(struct tree_subheader));
}

enum write_file_status write_pattern(size_t pattern_size, struct key** pattern, FILE* f) {
    enum write_file_status status = NULL_VALUE;
    for (; pattern_size-- > 0; pattern++) {
        status |= write_to_file((*pattern)->header, f, sizeof(struct key_header));
        status |= write_to_file((*pattern)->key_value, f, (*pattern)->header->size);
    }
    return status;
}

enum write_file_status write_id_seq(size_t size, uint64_t* id_seq, FILE* file) {
    return write_to_file(id_seq, file, size);
}

enum write_file_status write_tree_header(struct tree_header* header, FILE* file) {
    fseek(file, 0, SEEK_SET);
    size_t pattern_size = header->subheader->pattern_size;
    enum write_file_status status = write_tree_subheader(header->subheader, file);
    if (status != WRITE_OK) {
        printf("write error occurred\n");
    }
    fseek(file, sizeof(struct tree_subheader), SEEK_SET);
    status |= write_pattern(pattern_size, header->pattern, file);
    size_t id_array_size = get_id_array_size(header->subheader->current_id, header->subheader->pattern_size);
    status |= write_id_seq(id_array_size * sizeof(uint64_t), header->sequence_id, file);
    if (status == WRITE_ERROR) printf("write error occurred\n");
    return status;
}

enum write_file_status init_empty_file(FILE* file, char** pattern, size_t pattern_size, uint32_t* types, size_t* key_sizes) {
    fseek(file, 0, SEEK_SET);
    struct tree_header* header = get_pointer(sizeof(struct tree_header));
    generate_tree_header(pattern, pattern_size, types, key_sizes, header);
    enum write_file_status status = write_tree_header(header, file);
    free_tree_header(header);
    return status;
}

enum write_file_status write_tuple(size_t tuple_size, struct tuple* tuple, FILE* file) {
    union tuple_header* header = get_pointer(sizeof(union tuple_header));
    *header = tuple->header;
    enum write_file_status status = write_to_file(header, file, sizeof(union tuple_header));
    free_pointer(header);
    status |= write_to_file(tuple->data, file, tuple_size);
    return status;
}

void free_tree_header(struct tree_header* header) {
    for (size_t i = 0; i < header->subheader->pattern_size; i++) {
        free_pointer(header->pattern[i]->key_value);
        free_pointer(header->pattern[i]->header);
        free_pointer(header->pattern[i]);
    }
    free_pointer(header->pattern);
    free_pointer(header->sequence_id);
    free_pointer(header->subheader);
    free_pointer(header);
}

void free_tuple(struct tuple* tuple) {
    free_pointer(tuple->data);
    free_pointer(tuple);
}

void* get_pointer(size_t size) {
    return malloc(size);
}

void free_pointer(void* pointer) {
    free(pointer);
}
