#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void *Allocate(size_t size) {
    return malloc(size);
}

void Destroy(void *ptr) {
    free(ptr);
}

void *SetMemory(void *block, int value, size_t numBytes) {
    return memset(block, value, numBytes);
}
