#include "common.h"
#include <sys/socket.h>
#include <pb_encode.h>
#include <pb_decode.h>

static bool read_callback(pb_istream_t *istream, uint8_t *buf, size_t count) {
    int fd = (intptr_t) istream->state;
    int result;
    if (count == 0) {
        return true;
    }
    result = recv(fd, buf, count, MSG_WAITALL);
    if (result == 0) {
        istream->bytes_left = 0;
    }
    return result == count;
}

static bool write_callback(pb_ostream_t *ostream, const uint8_t *buf, size_t count) {
    int fd = (intptr_t) ostream->state;
    return send(fd, buf, count, 0) == count;
}

pb_istream_t pb_istream_from_socket(int fd) {
    pb_istream_t istream = {&read_callback, (void*) (intptr_t) fd, SIZE_MAX};
    return istream;
}

pb_ostream_t pb_ostream_from_socket(int fd) {
    pb_ostream_t ostream = {&write_callback, (void*) (intptr_t) fd, SIZE_MAX, 0};
    return ostream;
}
