add_library(sync_buffer STATIC 
    sync_buffer.c 
    linked_list.c
)

target_include_directories(sync_buffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

