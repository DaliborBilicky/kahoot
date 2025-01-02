#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>

typedef struct Buffer {
    void* data;
    size_t data_size;
    size_t capacity;
    size_t size;
    size_t in;
    size_t out;
} Buffer;

void buffer_init(Buffer* self, size_t capacity, size_t data_size);
void buffer_destroy(Buffer* self);
void buffer_push(Buffer* self, const void* input);
void buffer_pop(Buffer* self, void* output);

#endif  // BUFFER_H
