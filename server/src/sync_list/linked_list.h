#ifndef LINKED_LIST_H
#define LINKED_LIST_H
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct LinkedListNode {
    struct LinkedListNode *next;
    void *data;
} LinkedListNode;

typedef struct LinkedList {
    struct LinkedListNode *head;
    struct LinkedListNode *tail;
    size_t size;
    size_t data_size;
} LinkedList;

void linked_list_init(LinkedList *self, size_t data_size);
void linked_list_destroy(LinkedList *self);
size_t linked_list_get_size(const LinkedList *self);
void linked_list_add(LinkedList *self, void *data);
void linked_list_for_each(LinkedList *self,
                          void (*process_item)(void *, void *, void *, void *),
                          void *in, void *out, void *err);

#endif  // LINKED_LIST_H
