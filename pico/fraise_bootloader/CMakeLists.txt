cmake_minimum_required(VERSION 3.13)
set(fruit_device_path "${CMAKE_CURRENT_SOURCE_DIR}/../fraise_device")
include(${fruit_device_path}/pico_sdk_import.cmake)

project("fraise_bootloader" C CXX ASM)

#include(pico_sdk_import.cmake)

#project(${CMAKE_PROJECT_NAME} C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

#set(PICO_BARE_METAL 1)

pico_sdk_init()


add_executable(${CMAKE_PROJECT_NAME} main.c fraise_bootdevice.c bootloader.c)
pico_generate_pio_header(${CMAKE_PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/fraise.pio)

pico_add_extra_outputs(${CMAKE_PROJECT_NAME})

#set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-Wall")
#set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES COMPILE_OPTIONS "-DFRAISE_BLD_DEBUG")

# get every asm intermediate output
#target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -save-temps -fverbose-asm)

target_link_options(${CMAKE_PROJECT_NAME} PUBLIC "-L${fruit_device_path}")
pico_set_linker_script(${CMAKE_PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/fraise_bootloader.ld)

target_include_directories(${CMAKE_PROJECT_NAME} INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	)

target_link_libraries(${CMAKE_PROJECT_NAME} 
    pico_stdlib
    hardware_irq
    hardware_watchdog
    hardware_structs
    hardware_pio
	hardware_resets
	hardware_sync
	hardware_flash
	pico_unique_id
	cmsis_core
)

pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 1)
pico_enable_stdio_uart(${CMAKE_PROJECT_NAME} 0)

