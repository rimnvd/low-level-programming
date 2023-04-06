#include "init.h"

void init_file(FILE* file) {
    printf("trying to initialize the pattern\nnumber of fields: ");
    char *cnt_string = malloc(INPUT_BUFFER_SIZE);
    scanf("%s", cnt_string);
    while (!isNum(cnt_string)) {
        printf("number of fields must be integer\nnumber of fields: ");
        scanf("%s", cnt_string);
    }
    size_t count = strtol(cnt_string, NULL, 10);
    char* string;
    char** string_arr = get_pointer(count * sizeof(char *));
    char* type = malloc(INPUT_BUFFER_SIZE);
    uint32_t* types = get_pointer(count * sizeof(uint32_t));
    size_t* sizes = get_pointer(count * sizeof(size_t));
    size_t temp_size;
    for (size_t i = 0; i < count; i++) {
        printf("field %-3zu:\n", i);
        string = get_pointer(INPUT_BUFFER_SIZE);
        printf("field name: ");
        scanf("%s", string);
        string_arr[i] = string;
        temp_size = strlen(string);
        sizes[i] = temp_size + (!(temp_size % FILE_GRANULARITY) ? 1 : 0);
        printf("[%d] Boolean\n", BOOLEAN_TYPE);
        printf("[%d] Integer\n", INTEGER_TYPE);
        printf("[%d] Float\n", FLOAT_TYPE);
        printf("[%d] String\n", STRING_TYPE);
        printf("field type: ");
        scanf("%s", type);
        while (strlen(type) != 1) {
            printf("incorrect format\nfield type: ");
            scanf("%s", type);
        }
        types[i] = strtol(type, NULL, 10);
    }
    init_empty_file(file, string_arr, count, types, sizes);
    for (size_t i = 0; i < count; i++) {
        free_pointer(string_arr[i]);
    }
    free_pointer(string_arr);
    free_pointer(sizes);
    free_pointer(types);
    free_pointer(cnt_string);
    free_pointer(type);
}


FILE* initializer(int argc, char** argv) {
    char* init_filename;
    char* data_file;
    FILE* file;
    FILE* data;
    char flag;
    if (argc < 3 || argc > 4) {
        printf("wrong number of args\n");
        return NULL;
    } else if (argc == 4) {
        data_file = argv[3];
    }
    init_filename = argv[2];
    flag = argv[1][1];
    switch (flag) {
        case 'i':
            open_exist_file(&file, init_filename);
            break;
        case 'm':
            open_empty_file(&file, init_filename);
            init_file(file);
            break;
        case 'n':
            open_parse_file(&data, data_file);
            open_empty_file(&file, init_filename);
            init_file(file);
            parse_file(data, file);
            break;
        default:
            printf("there is no such flag: -%c", flag);
            return NULL;
    }
    return file;
}

int32_t read_command(char* buffer) {
    printf("enter the command: ");
    return !scanf("%s", buffer);
}

enum commands parse_command(char* command) {
    if (strcmp(command, "add") == 0)  {
        return 0;
    }
    if (strcmp(command, "remove") == 0)  {
        return 1;
    }
    if (strcmp(command, "update") == 0)  {
        return 2;
    }
    if (strcmp(command, "find_all") == 0)  {
        return 3;
    }
    if (strcmp(command, "find_by_parent") == 0)  {
        return 4;
    }
    if (strcmp(command, "find_by_condition") == 0)  {
        return 5;
    }
    if (strcmp(command, "find_by_id") == 0)  {
        return 6;
    }
    if (strcmp(command, "exit") == 0)  {
        return 7;
    }
    return 8;

}

int32_t interactive_mode(FILE* file) {
    uint8_t loop = NULL_VALUE;
    char command[BUFFER_COMMAND_SIZE];
    while(!loop) {
        read_command(command);
        switch (parse_command(command)) {
            case ADD_COMMAND:
                add_element(file);
                break;
            case UPDATE_COMMAND:
                update_element(file);
                break;
            case FIND_ALL_COMMAND:
                find_all_results(file);
                break;
            case FIND_BY_PARENT_COMMAND:
                find_parent(file);
                break;
            case FIND_BY_CONDITION_COMMAND:
                find_condition(file);
                break;
            case FIND_BY_ID_COMMAND:
                find_id(file);
                break;
            case EXIT_COMMAND:
                loop++;
                break;
            default:
                printf("The command is wrong\n");
                break;
        }
    }
    close_file(file);
    return 0;
}




