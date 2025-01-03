#include "buffer.h"

#include <string.h>

void buffer_init(Buffer* self, size_t capacity, size_t data_size) {
    self->data = calloc(capacity, data_size);
    self->data_size = data_size;
    self->capacity = capacity;
    self->size = 0;
}

void buffer_destroy(Buffer* self) {
    free(self->data);
    self->data = NULL;
}

void buffer_push(Buffer* self, const void* input) {
    memcpy((char*)self->data + self->size * self->data_size, input,
           self->data_size);
    self->size++;
}

void buffer_pop(Buffer* self, void* output) {
    self->size--;
    memcpy(output, (char*)self->data + self->size * self->data_size,
           self->data_size);
}
