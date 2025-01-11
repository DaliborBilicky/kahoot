add_library(server STATIC
    server.c
    server_communication.c
)

target_include_directories(server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(server PRIVATE sync_buffer sockets lobby)
