add_library(sockets STATIC
    sockets.c
)

target_include_directories(sockets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
