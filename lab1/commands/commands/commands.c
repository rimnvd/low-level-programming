#include "commands.h"

enum crud_operation_status add_element(FILE* file) {
    struct tree_header* header = malloc(sizeof(struct tree_header));
    read_tree_header(header, file);
    printf("Enter fields of new tuple\n");
    uint64_t* pattern = malloc(header->subheader->pattern_size);
    enum crud_operation_status status = CRUD_OK;
    for (uint64_t iter = 0; iter < header->subheader->pattern_size; iter++) {
        printf("%-20s:", header->pattern[iter]->key_value);
        char* s;
        switch (header->pattern[iter]->header->type) {
            case BOOLEAN_TYPE:
            case INTEGER_TYPE:
            case FLOAT_TYPE:
                status |= !scanf("%ld", pattern + iter);
                break;
            default:
                s = malloc(INPUT_BUFFER_SIZE);
                status |= !scanf("%s", s);
                pattern[iter] = (uint64_t) s;
                break;
        }
    }
    uint64_t parent_id;
    printf("%-20s:", "Write parent id");
    status |= !scanf("%ld", &parent_id);
    status |= add_tuple(pattern, parent_id, file);
    if (status == CRUD_OK) {
        printf("TUPLE %3ld INSERTED\n", header->subheader->current_id);
    } else {
        printf("NOT INSERTED\n");
    }
    free_tree_header(header);
    return status;
}

void find_element(char **array, size_t pattern_size, const uint32_t *pattern_types, char **pattern_names, size_t cnt, FILE *file) {

}

enum crud_operation_status update_element(FILE *file) {
    printf("Enter id\n");
    printf("%-20s:", "Id");
    uint64_t id;
    enum crud_operation_status status = !scanf("%ld", &id);
    printf("Choose field\n");
    struct tree_header *header = malloc(sizeof(struct tree_header));
    read_tree_header(header, file);
    for (uint64_t iter = 0; iter < header->subheader->pattern_size; iter++) {
        printf("%ld. %s\n", iter, header->pattern[iter]->key_value);
    }
    uint64_t  number;
    printf("Field: ");
    status |= !scanf("%ld", &number);
    printf("Enter new value:\n");
    printf("Value: ");
    uint64_t value;
    char s[INPUT_BUFFER_SIZE];
    switch (header->pattern[number]->header->type) {
        case INTEGER_TYPE:
            status |= !scanf("%ld", &value);
            break;
        case BOOLEAN_TYPE:
            status |= !scanf("%ld", &value);
            break;
        case FLOAT_TYPE:
            status |= !scanf("%lf", (double *) &value);
            break;
        default:
            status |= !scanf("%s", s);
            value = (uint64_t) s;
            break;
    }
    status |= update_tuple(&value, number, id, file);
    if (status == CRUD_OK) {
        printf("TUPLE %3ld UPDATED\n", header->subheader->current_id);
    }
    else {
        printf("NOT UPDATED\n");
    }
    free_tree_header(header);
    return status;
}

enum crud_operation_status find_all_results(FILE *file) {
    struct tuple_result_list* list = NULL;
    find_all(file, &list);
    print_result_list(file, list);
    free_pointer(list);
    return CRUD_OK;
}

enum crud_operation_status find_parent(FILE *file) {
    printf("Enter parent id\n");
    printf("%-20s:", "Id");
    uint64_t  id;
    enum crud_operation_status status = !scanf("%ld", &id);
    struct tuple_result_list *list = NULL;
    find_by_parent(id, &list, file);
    print_result_list(file, list);
    free_pointer(list);
    return status ? CRUD_ERROR : CRUD_OK;
}

enum crud_operation_status find_condition(FILE *file) {
    printf("Choose field\n");
    struct tree_header *header = malloc(sizeof(struct tree_header));
    read_tree_header(header, file);
    for (uint64_t iter = 0; iter < header->subheader->pattern_size; iter++) {
        printf("%ld. %s\n", iter, header->pattern[iter]->key_value);
    }
    uint64_t  number;
    printf("Field: ");
    enum crud_operation_status status = !scanf("%ld", &number);
    if (number > header->subheader->pattern_size - 1) {
        return CRUD_ERROR;
    }
    printf("Enter expression to equaling\n");
    printf("Exp: ");
    uint64_t value;
    char s[INPUT_BUFFER_SIZE];
    switch (header->pattern[number]->header->type) {
        case INTEGER_TYPE: status |= !scanf("%ld", &value);; break;
        case BOOLEAN_TYPE: status |= !scanf("%ld", &value);; break;
        case FLOAT_TYPE: status |= !scanf("%lf", (double *) &value);; break;
        default: status |= !scanf("%s", s); value = (uint64_t) s; break;
    }
    struct tuple_result_list *list = NULL;
    find_by_condition(number, &value, &list, file);
    print_result_list(file, list);
    free_tree_header(header);
    free_pointer(list);
    return status ? CRUD_ERROR : CRUD_OK;
}

enum crud_operation_status find_id(FILE* file) {
    printf("Enter id\n");
    printf("%-20s:", "Id");
    uint64_t  id;
    struct tree_header *header = malloc(sizeof(struct tree_header));
    read_tree_header(header, file);
    enum crud_operation_status status = !scanf("%ld", &id);
    uint64_t *fields;
    if (header->subheader->current_id < id){
        printf("Too large id\n");
        free_tree_header(header);
        return CRUD_OK;
    }
    status = get_tuple(id, &fields, file);
    if (status) {
        printf("No result\n");
        free_tree_header(header);
        return CRUD_ERROR;
    }
    for (uint64_t iter = 0; iter < header->subheader->pattern_size; iter++) {
        switch (header->pattern[iter]->header->type) {
            case INTEGER_TYPE:
                printf("%-20s: %ld\n", header->pattern[iter]->key_value, fields[iter]);
                break;
            case BOOLEAN_TYPE:
                printf("%-20s: %d\n", header->pattern[iter]->key_value, fields[iter] != 0);
                break;
            case FLOAT_TYPE:
                printf("%-20s: %lf\n", header->pattern[iter]->key_value, (double) fields[iter]);
                break;
            default:
                printf("%-20s: %s\n", header->pattern[iter]->key_value, (char *)fields[iter]);
                free((char *)fields[iter]);
                break;
        }
    }
    free(fields);
    free_tree_header(header);
    return CRUD_OK;
}

//enum crud_operation_status remove_element(FILE* file){
//    printf("Enter id\n");
//    printf("%-20s:", "Id");
//    uint64_t  id;
//    enum crud_operation_status status = !scanf("%ld", &id);
//    remove_tuple(id, file);
//    return status ? CRUD_INVALID : CRUD_OK;
//}

