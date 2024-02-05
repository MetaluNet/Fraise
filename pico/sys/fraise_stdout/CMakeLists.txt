add_library(fraise_stdout INTERFACE)

target_sources(fraise_stdout INTERFACE 
	${CMAKE_CURRENT_LIST_DIR}/fraise_stdout.c
)

target_include_directories(fraise_stdout INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(fraise_stdout INTERFACE
	pico_stdlib
)

