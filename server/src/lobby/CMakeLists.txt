add_library(lobby STATIC
    lobby.c
    lobby_communication.c
	question.c
)

target_include_directories(lobby PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(lobby PRIVATE sockets sync_list)
