#ifndef LLP1_GENERATOR_H
#define LLP1_GENERATOR_H

#include <stdlib.h>
#include "../structs/tree_header.h"
#include "../configuration.h"
#include "../utils/structs_utils/structs_utils.h"

void generate_tree_header(char** pattern, size_t pattern_size, uint32_t* types, size_t* key_sizes, struct tree_header* header);

#endif //LLP1_GENERATOR_H
