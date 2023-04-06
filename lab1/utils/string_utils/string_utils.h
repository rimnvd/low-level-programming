#ifndef LLP1_STRING_UTILS_H
#define LLP1_STRING_UTILS_H

#include <stdbool.h>
#include "../../crud/api/api.h"
#include "../../commands/commands/commands.h"

char* concat_strings(const char* first_str, const char* second_str);

size_t split(char* string, char c, char*** array);

bool isNum(const char* string);

void parse_file(FILE* from, FILE* to);

int checkSubstring(char* str, char* sub);

#endif //LLP1_STRING_UTILS_H
