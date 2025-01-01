add_subdirectory(sync_buffer)

add_executable(kahoot_server main.c)

target_link_libraries(kahoot_server sync_buffer)
