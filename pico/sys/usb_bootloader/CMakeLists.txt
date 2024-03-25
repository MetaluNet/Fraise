cmake_minimum_required(VERSION 3.12)

get_filename_component(fraise_path ../../.. REALPATH)
# Pull in SDK (must be before project)
include(../find_tools.cmake)

project(usb_bootloader C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

#set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()

add_executable(${CMAKE_PROJECT_NAME} bootloader.c)

add_subdirectory("${PROJECT_SOURCE_DIR}/../../fraise_eeprom" fraise_eeprom)

# pull in common dependencies
target_link_libraries(${CMAKE_PROJECT_NAME} 
	pico_stdlib 
	hardware_flash
	hardware_structs
	hardware_resets
	hardware_sync
	cmsis_core
	fraise_eeprom
)

# create map/bin/hex file etc.
pico_add_extra_outputs(${CMAKE_PROJECT_NAME})

pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 1)
pico_enable_stdio_uart(${CMAKE_PROJECT_NAME} 0)

pico_set_float_implementation(${CMAKE_PROJECT_NAME} none)
pico_set_double_implementation(${CMAKE_PROJECT_NAME} none)

target_link_options(${CMAKE_PROJECT_NAME} PUBLIC "-L${PROJECT_SOURCE_DIR}/..")
pico_set_linker_script(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCE_DIR}/../master_bootloader.ld)

