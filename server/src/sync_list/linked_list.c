#include "linked_list.h"

#include <stdlib.h>
#include <string.h>

#include "linked_list.h"

void linked_list_init(LinkedList *self, size_t data_size) {
    self->head = NULL;
    self->tail = NULL;
    self->size = 0;
    self->data_size = data_size;
}

void linked_list_destroy(LinkedList *self) {
    LinkedListNode *current = self->head;
    while (current != NULL) {
        LinkedListNode *next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
}

size_t linked_list_get_size(const LinkedList *self) { return self->size; }

void linked_list_add(LinkedList *self, void *data) {
    LinkedListNode *new_node = (LinkedListNode *)malloc(sizeof(LinkedListNode));
    new_node->data = malloc(self->data_size);
    memcpy(new_node->data, data, self->data_size);
    new_node->next = NULL;

    if (self->tail != NULL) {
        self->tail->next = new_node;
    }
    self->tail = new_node;
    if (self->head == NULL) {
        self->head = new_node;
    }

    self->size++;
}

void linked_list_for_each(LinkedList *self,
                          void (*process_item)(void *, void *, void *, void *),
                          void *in, void *out, void *err) {
    LinkedListNode *current = self->head;
    while (current != NULL) {
        process_item(current, in, out, err);
        current = current->next;
    }
}
