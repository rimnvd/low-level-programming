#include "file_utils.h"

enum read_file_status read_from_file(void* buffer, FILE* file, size_t size) {
    size_t len = fread(buffer, size, 1, file);
    enum read_file_status status = READ_OK;
    if (len < 1) {
        status = READ_ERROR;
    } else if (sizeof(buffer) < len){
        status = READ_END_OF_FILE;
    }
    return status;
}

enum write_file_status write_to_file(void* buffer, FILE* file, size_t size) {
    size_t len = fwrite(buffer, size, 1, file);
    enum write_file_status status = WRITE_OK;
    if (len < 1) {
        status = WRITE_WRONG_INTEGRITY;
    } else if (sizeof(buffer) < len) {
        status = WRITE_ERROR;
    }
    return status;
}

static enum open_file_status open_file(char* name, FILE** file, char *open_descriptor) {
    *file = fopen(name, open_descriptor);
    enum open_file_status status = OPEN_OK;
    if (!*file) status = OPEN_FAILED;
    return status;
}

enum open_file_status open_exist_file(FILE** file, char* filename) {
    return open_file(filename, file, "r+b");
}

enum open_file_status open_parse_file(FILE** file, char* filename) {
    return open_file(filename, file, "r");
}

enum open_file_status open_empty_file(FILE** file, char* filename) {
    open_file(filename, file, "w");
    close_file(*file);
    return open_file(filename, file, "r+b");
}

void close_file(FILE* file) {
    fclose(file);
}

