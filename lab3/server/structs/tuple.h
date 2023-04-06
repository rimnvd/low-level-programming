#ifndef LLP1_TUPLE_H
#define LLP1_TUPLE_H

#include <inttypes.h>
#include "../utils/file_utils/file_utils.h"

union tuple_header {
    struct {
        uint64_t parent;
        uint64_t alloc;
    };
    struct {
        uint64_t previous;
        uint64_t next;
    };
};

struct tuple {
    union tuple_header header;
    uint64_t* data;
};

#endif //LLP1_TUPLE_H
