#include "generator.h"

static void generate_tree_subheader(struct tree_subheader* tree_subheader, size_t pattern_size) {
    tree_subheader->pattern_size = (uint64_t) pattern_size;
    tree_subheader->current_id = NULL_VALUE;
    tree_subheader->ASCII_signature = BIG_ENDIAN_SIGNATURE;
    tree_subheader->first_seq = NULL_VALUE;
    tree_subheader->second_seq = NULL_VALUE;
    tree_subheader->root_offset = NULL_VALUE;
}

static void copy_string(char* from, char* to, size_t from_size, size_t to_size) {
    while (to_size-- && from_size--) {
        *(to++) = *(from++);
    }
}

static void generate_pattern(struct key** key_pattern, size_t* key_sizes, char** pattern, size_t pattern_size, uint32_t* types) {
    struct key* pattern_key;
    size_t size;
    for (size_t i = pattern_size; i-- > 0; key_pattern++ && pattern++ && types++ && key_sizes++) {
        pattern_key = get_pointer(sizeof(struct key));
        size = (*key_sizes) / FILE_GRANULARITY * FILE_GRANULARITY + ((*key_sizes) % FILE_GRANULARITY ? FILE_GRANULARITY : 0);
        char* to_string = get_pointer(sizeof(char)* size);
        copy_string(*pattern, to_string, *key_sizes, size);
        pattern_key->key_value = to_string;
        pattern_key->header = get_pointer(sizeof(struct key_header));
        pattern_key->header->size = (uint32_t) size;
        pattern_key->header->type = *types;
        *key_pattern = pattern_key;
    }
}

void generate_tree_header(char** pattern, size_t pattern_size, uint32_t* types, size_t* key_sizes, struct tree_header* header) {
    header->subheader = get_pointer(sizeof(struct tree_subheader));
    generate_tree_subheader(header->subheader, pattern_size);
    header->pattern = get_pointer(sizeof(struct key*) * pattern_size);
    generate_pattern(header->pattern, key_sizes, pattern, pattern_size, types);
    size_t id_array_size = get_id_array_size(header->subheader->current_id, header->subheader->pattern_size);
    header->sequence_id = get_pointer(sizeof(uint64_t) * id_array_size);
    for (size_t i = 0; i < id_array_size; i++) {
        header->sequence_id[i] = 0;
    }
}
