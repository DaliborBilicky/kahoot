add_library(sync_buffer STATIC 
    sync_buffer.c 
    linked_list.c
)

target_include_directories(sync_buffer PUBLIC src/sync_buffer)
