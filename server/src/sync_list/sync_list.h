#ifndef SYNC_LIST_H
#define SYNC_LIST_H

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>

#include "linked_list.h"

typedef struct {
    LinkedList list;
    pthread_mutex_t lock;
} SyncLinkedList;

void sync_list_init(SyncLinkedList *self, size_t data_size);
void sync_list_destroy(SyncLinkedList *self);
void sync_list_add(SyncLinkedList *self, void *data);
bool sync_list_remove(SyncLinkedList *self, size_t index);
void sync_list_for_each(SyncLinkedList *self,
                        void (*process_item)(void *, void *, void *, void *),
                        void *in, void *out, void *err);

#endif  // SYNC_LIST_H
