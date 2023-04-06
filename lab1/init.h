#ifndef LLP1_INIT_H
#define LLP1_INIT_H

#include "crud/basic.h"
#include "utils/string_utils/string_utils.h"
#include "utils/structs_utils/structs_utils.h"

void init_file(FILE* file);

FILE* initializer(int argc, char** argv);

enum commands {
    ADD_COMMAND = 0,
    REMOVE_COMMAND,
    UPDATE_COMMAND,
    FIND_ALL_COMMAND,
    FIND_BY_PARENT_COMMAND,
    FIND_BY_CONDITION_COMMAND,
    FIND_BY_ID_COMMAND,
    EXIT_COMMAND,
    WRONG_COMMAND
};

int32_t interactive_mode(FILE* file);

#endif //LLP1_INIT_H
