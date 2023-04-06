#include "find_utils.h"

void print_result_list(FILE *file, struct tuple_result_list* list) {
    if (!list) {
        printf("No result\n");
    }
    struct tree_header* header = malloc(sizeof(struct tree_header));
    read_tree_header(header, file);
    uint64_t count = 0;
    char* str;
    while(list) {
        printf("--- TUPLE %3ld ---\n", count++);
        for (uint64_t iter = 0; iter < header->subheader->pattern_size; iter++) {
            switch (header->pattern[iter]->header->type) {
                case INTEGER_TYPE:
                    printf("%-20s: %ld\n", header->pattern[iter]->key_value, list->value->data[iter]);
                    break;
                case BOOLEAN_TYPE:
                    printf("%-20s: %d\n", header->pattern[iter]->key_value, list->value->data[iter] != 0);
                    break;
                case FLOAT_TYPE:
                    printf("%-20s: %lf\n", header->pattern[iter]->key_value, (float) list->value->data[iter]);
                    break;
                default:
                    read_string_from_tuple(list->value->data[iter], &str, header->subheader->pattern_size, file);
                    printf("%-20s: %str\n", header->pattern[iter]->key_value, str);
                    free(str);
                    break;
            }
        }
        list = list->previous;
    }
    free_tree_header(header);
}


