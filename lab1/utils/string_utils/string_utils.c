#include "string_utils.h"

char* concat_strings(const char* first_str, const char* second_str) {
    char* result_str = get_pointer(strlen(first_str) + strlen(second_str) + 1);
    strcpy(result_str, first_str);
    strcat(result_str, second_str);
    return result_str;
}

size_t split(char* string, char c, char*** array) {
    int cnt = 1;
    char* pos;
    pos = string;
    while (*pos != '\0') {
        if (*pos == c) {
            cnt++;
        }
        pos++;
    }
    *array = (char**) get_pointer(sizeof(char*)* cnt);
    if (!*array) exit(1);
    int i = 0;
    char* start = string;
    for (pos = string; *pos != '\0'; pos++) {
        if (*pos == c) {
            *pos = '\0';
            (*array)[i++] = start;
            start = pos + 1;
        }
    }
    (*array)[i++] = start;
    return cnt;
}

bool isNum(const char* string) {
    while (*string != '\0' && *string != 13) {
        if (*string < '0' || *string > '9') {
            return false;
        }
        string++;
    }
    return true;
}

void parse_file(FILE* from, FILE* to) {
    char line[INPUT_BUFFER_SIZE];
    char** args = NULL;
    size_t pattern_size;
    struct tree_header* header = get_pointer(sizeof(struct tree_header));
    read_tree_header(header, to);
    pattern_size = header->subheader->pattern_size;
    uint32_t* pattern_types = get_pointer(sizeof(uint32_t)* pattern_size);
    char** pattern_names = get_pointer(sizeof(char*) * pattern_size);
    for (int i = 0; i < pattern_size; i++) {
        pattern_types[i] = header->pattern[i]->header->type;
        pattern_names[i] = header->pattern[i]->key_value;
    }
    uint64_t parent;
    uint64_t code;
    uint64_t fields[2];
    char s[INPUT_BUFFER_SIZE];
    fgets(line, INPUT_BUFFER_SIZE, from);
    while (!feof(from)) {
        if (strlen(line) == 0) {
            break;
        }
        line[strlen(line) - 1] = '\0';
        if (fscanf(from, "%ld code=%ld name=%s\n", &parent, &code, s)) {
            fields[0] = (uint64_t) s;
            fields[1] = code;
            add_tuple(fields, parent, to);
        }
    }
    free_tree_header(header);
    fclose(from);
    fflush(to);
    free_pointer(pattern_types);
    free_pointer(pattern_names);
}

int checkSubstring(char* str, char* sub) {
    int flag;
    int len1 = strlen(str);
    int len2 = strlen(sub);
    for (int i = 0; i <= len1 - len2; i++) {
        flag = 1;
        for (int j = 0; j < len2; j++) {
            if(str[i + j] != sub[j]) {
                flag = 0;
                break;
            }
        }
        if (flag) {
            return 0;
        }
    }
    return 1;
}
