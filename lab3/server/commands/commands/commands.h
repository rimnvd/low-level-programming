#ifndef LLP1_COMMANDS_H
#define LLP1_COMMANDS_H

#include "../../crud/api/api.h"
#include "../../utils/string_utils/string_utils.h"
#include "../../utils/file_utils/file_utils.h"

enum error_code {
    OK,
    PATTERN_ERROR,
    BOOL_ERROR,
    FLOAT_ERROR,
    INT_ERROR,
    STR_ERROR,
    COUNT_ERROR
};

size_t add(char** string, size_t pattern_size, const uint32_t* pattern_types, char** pattern_names, FILE* file);

size_t add_new(FILE* file, Query_tree_Value_setting* settings, size_t settings_count, uint64_t parent_id, size_t pattern_size,
               const uint32_t* pattern_types, char** pattern_names);

void find(char** array, size_t pattern_size, const uint32_t* pattern_types, char** pattern_names, size_t count, FILE* file);

size_t update(FILE* file, uint64_t id, Query_tree_Value_setting* settings, size_t settings_count, size_t pattern_size,
              const uint32_t* pattern_types, char** pattern_names);


#endif //LLP1_COMMANDS_H
