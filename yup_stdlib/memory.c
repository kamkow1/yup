#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void *mem_alloc(size_t size) {
    return malloc(size);
}

void mem_free(void *ptr) {
    free(ptr);
}

void *mem_init(void *block, int value, size_t num_bytes) {
    return memset(block, value, num_bytes);
}
