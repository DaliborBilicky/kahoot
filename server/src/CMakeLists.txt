add_subdirectory(sync_buffer)
add_subdirectory(server)
add_subdirectory(sockets)
add_subdirectory(lobby)
add_subdirectory(sync_list)

add_executable(kahoot_server main.c)

target_link_libraries(kahoot_server sync_buffer server sockets pthread lobby sync_list)
