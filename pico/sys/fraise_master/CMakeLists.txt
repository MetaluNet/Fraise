add_library(fraise_master INTERFACE)

pico_generate_pio_header(fraise_master ${CMAKE_CURRENT_LIST_DIR}/fraise.pio)
target_sources(fraise_master INTERFACE fraise_master.c fraise_bootload.c fraise_master_buffers.c main.c)

target_include_directories(fraise_master INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(fraise_master INTERFACE
	pico_stdlib
	hardware_irq
	hardware_watchdog
	hardware_structs
	hardware_pio
	pico_async_context_threadsafe_background
	pico_async_context_poll
	cmsis_core
	fraise_eeprom
)

# set_target_properties(fraise_device PROPERTIES INTERFACE_COMPILE_OPTIONS "-DFRAISE_MASTER_DEBUG")

