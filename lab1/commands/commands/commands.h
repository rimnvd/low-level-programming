#ifndef LLP1_COMMANDS_H
#define LLP1_COMMANDS_H

#include "../../crud/api/api.h"
#include "../../utils/string_utils/string_utils.h"
#include "../find_utils/find_utils.h"

enum crud_operation_status add_element(FILE* file);

enum crud_operation_status find_all_results(FILE *file);

enum crud_operation_status find_parent(FILE *file);

enum crud_operation_status find_condition(FILE *file);

enum crud_operation_status find_id(FILE* file);

enum crud_operation_status update_element(FILE* file);

enum crud_operation_status remove_element(FILE* file);

#endif //LLP1_COMMANDS_H
