add_library(fraise_device INTERFACE)

pico_generate_pio_header(fraise_device ${CMAKE_CURRENT_LIST_DIR}/fraise.pio)
target_sources(fraise_device INTERFACE 
	${CMAKE_CURRENT_LIST_DIR}/fraise_device.c
	${CMAKE_CURRENT_LIST_DIR}/fraise_buffers.c
)

target_include_directories(fraise_device INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(fraise_device INTERFACE
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

#set_target_properties(fraise_device PROPERTIES INTERFACE_COMPILE_OPTIONS "-DFRAISE_DEVICE_DEBUG")

