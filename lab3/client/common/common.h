#ifndef LLP3_COMMON_H
#define LLP3_COMMON_H

#include "../protos/nanopb.h"
#include "stdbool.h"
#include "inttypes.h"

pb_ostream_t pb_ostream_from_socket(int fd);
pb_istream_t pb_istream_from_socket(int fd);

#endif //LLP3_COMMON_H
