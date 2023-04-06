#include <stdint.h>

#define STRING_TYPE 0
#define INT_TYPE 1
#define FLOAT_TYPE 2

struct filter {
    struct filter* next;
    struct comparator* comparator_list;
};

struct query {
    uint8_t command;
    struct filter* filters;
    struct value_setting* settings;
};

struct field_value_pair {
    char* field;
    uint8_t val_type;
    uint64_t int_value;
    float real_value;
};

struct value_setting {
    struct value_setting* next;
    struct field_value_pair field_value;
};

struct comparator {
    struct comparator* next;
    uint8_t operation;
    struct field_value_pair field_value;
};

struct extended_comparator {
    struct extended_comparator* next;
    struct extended_comparator* connected;
    uint8_t operation;
    struct field_value_pair field_value;
};