#ifndef LLP1_FILE_UTILS_H
#define LLP1_FILE_UTILS_H

#include <stdio.h>
#include <inttypes.h>

enum read_file_status {
    READ_OK = 0,
    READ_END_OF_FILE,
    READ_ERROR
};

enum write_file_status {
    WRITE_OK = 0,
    WRITE_WRONG_INTEGRITY,
    WRITE_ERROR
};

enum open_file_status {
    OPEN_OK = 0,
    OPEN_FAILED
};

enum read_file_status read_from_file(void* buffer, FILE* file, size_t size);

enum write_file_status write_to_file(void* buffer, FILE* file, size_t size);

enum open_file_status open_exist_file(FILE** file, char* filename);

enum open_file_status open_empty_file(FILE** file, char* filename);

enum open_file_status open_parse_file(FILE** file, char* filename);

void close_file(FILE* file);

#endif //LLP1_FILE_UTILS_H
