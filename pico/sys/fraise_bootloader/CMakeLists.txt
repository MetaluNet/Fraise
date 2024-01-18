cmake_minimum_required(VERSION 3.13)
set(pico_sys_path "${CMAKE_CURRENT_SOURCE_DIR}/..")
include(${pico_sys_path}/pico_sdk_import.cmake)

project("fraise_bootloader" C CXX ASM)
get_filename_component(board "${fraise_board}" NAME)
get_filename_component(boardDir "${fraise_board}" DIRECTORY)
if( "${boardDir}" STREQUAL "") 
	get_filename_component(boardDir "${pico_sys_path}/../../boards/${fraise_board}" REALPATH)
else()
	get_filename_component(boardDir "${fraise_board}" REALPATH)
endif()
message("board: ${board} ; board directory: ${boardDir}")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(${CMAKE_PROJECT_NAME} main.c fraise_bootdevice.c bootloader.c)

pico_generate_pio_header(${CMAKE_PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/fraise.pio)

pico_add_extra_outputs(${CMAKE_PROJECT_NAME})

add_subdirectory("${pico_sys_path}/../fraise_eeprom" fraise_eeprom)

#set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-Wall")
#set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES COMPILE_OPTIONS "-DFRAISE_BLD_DEBUG")

# get every asm intermediate output
#target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -save-temps -fverbose-asm)

target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC -I${boardDir})

target_link_options(${CMAKE_PROJECT_NAME} PUBLIC "-L${pico_sys_path}")
pico_set_linker_script(${CMAKE_PROJECT_NAME} ${pico_sys_path}/device_bootloader.ld)

target_link_libraries(${CMAKE_PROJECT_NAME} 
    pico_stdlib
    hardware_irq
    hardware_watchdog
    hardware_structs
    hardware_pio
	hardware_resets
	hardware_sync
	hardware_flash
	#pico_unique_id
	cmsis_core
	fraise_eeprom
)

pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 0)
pico_enable_stdio_uart(${CMAKE_PROJECT_NAME} 0)

add_custom_command(
	TARGET ${CMAKE_PROJECT_NAME} POST_BUILD 
	COMMAND ${CMAKE_COMMAND} -E echo "-- Copying uf2 file to board directory"
	COMMAND ${CMAKE_COMMAND} -E
		copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.uf2" "${boardDir}/${board}.uf2"
)

