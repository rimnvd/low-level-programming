#include <stdio.h>
#include "init.h"

const char *command_hint = ">>> ";

int32_t read_command_line(char *buffer) {
    printf("%s", command_hint);
    return !scanf("%s", buffer);
}

int main(int argc, char **argv) {
    FILE* file = initializer(argc, argv);
    interactive_mode(file);
    return 0;
}
