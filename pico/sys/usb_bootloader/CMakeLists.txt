cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

#include(pico_extras_import_optional.cmake)

set(projectName "bootloader")

project(${projectName} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

#set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()

#include(example_auto_set_url.cmake)


add_executable(${projectName}
        ${projectName}.c
        )

# pull in common dependencies
target_link_libraries(${projectName} 
	pico_stdlib 
	hardware_flash
	hardware_structs
	hardware_resets
	hardware_sync
	cmsis_core)

# create map/bin/hex file etc.
pico_add_extra_outputs(${projectName})

pico_enable_stdio_usb(${projectName} 1)
pico_enable_stdio_uart(${projectName} 0)

pico_set_float_implementation(${projectName} none)
pico_set_double_implementation(${projectName} none)

# add url via pico_set_program_url
#example_auto_set_url(auto_inter)
