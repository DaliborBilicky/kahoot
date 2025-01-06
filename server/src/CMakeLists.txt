add_subdirectory(sync_buffer)
add_subdirectory(server)
add_subdirectory(sockets)

add_executable(kahoot_server main.c)

target_link_libraries(kahoot_server sync_buffer server sockets pthread)
