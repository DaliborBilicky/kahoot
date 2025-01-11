#include "sync_list.h"

void sync_list_init(SyncLinkedList *self, size_t data_size) {
    linked_list_init(&self->list, data_size);
    pthread_mutex_init(&self->lock, NULL);
}

void sync_list_destroy(SyncLinkedList *self) {
    pthread_mutex_lock(&self->lock);
    linked_list_destroy(&self->list);
    pthread_mutex_unlock(&self->lock);
    pthread_mutex_destroy(&self->lock);
}

size_t sync_list_get_size(SyncLinkedList *self) {
    pthread_mutex_lock(&self->lock);
    size_t size = linked_list_get_size(&self->list);
    pthread_mutex_unlock(&self->lock);
    return size;
}

void sync_list_add(SyncLinkedList *self, void *data) {
    pthread_mutex_lock(&self->lock);
    linked_list_add(&self->list, data);
    pthread_mutex_unlock(&self->lock);
}

void sync_list_for_each(SyncLinkedList *self,
                        void (*process_item)(void *, void *, void *, void *),
                        void *in, void *out, void *err) {
    pthread_mutex_lock(&self->lock);
    linked_list_for_each(&self->list, process_item, in, out, err);
    pthread_mutex_unlock(&self->lock);
}
