add_library(sync_list STATIC 
	sync_list.c
	linked_list.c
)

target_include_directories(sync_list PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

