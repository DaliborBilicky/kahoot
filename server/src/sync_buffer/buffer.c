#include "buffer.h"

#include <string.h>

void buffer_init(Buffer* self, size_t capacity, size_t data_size) {
    self->data = calloc(capacity, sizeof(data_size));
    self->data_size = data_size;
    self->capacity = capacity;
    self->size = 0;
    self->in = 0;
    self->out = 0;
}

void buffer_destroy(Buffer* self) {
    free(self->data);
    self->data = NULL;
}

void buffer_push(Buffer* self, const void* input) {
    memcpy((char*)self->data + self->in * self->data_size, input,
           self->data_size);
    self->size++;
    self->in++;
    self->in %= self->capacity;
}

void buffer_pop(Buffer* self, void* output) {
    memcpy(output, (char*)self->data + self->out * self->data_size,
           self->data_size);
    self->size--;
    self->out++;
    self->out %= self->capacity;
}
