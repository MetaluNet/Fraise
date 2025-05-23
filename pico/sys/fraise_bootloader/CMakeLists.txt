cmake_minimum_required(VERSION 3.12)

get_filename_component(fraise_path ../../.. REALPATH)
# Pull in SDK (must be before project)
include(../find_board.cmake)
include(../find_tools.cmake)

message("fraise_board: ${fraise_board}")
message("boardDir: ${boardDir}")
message("PICO_BOARD: ${PICO_BOARD}")
message("PICO_PLATFORM: ${PICO_PLATFORM}")

project(fraise_bootloader C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "2.1.1")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 2.1.1 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_subdirectory("${PROJECT_SOURCE_DIR}/blink" blink)
add_subdirectory("${PROJECT_SOURCE_DIR}/bootloader" bootloader)

set(APP_BIN ${CMAKE_CURRENT_BINARY_DIR}/blink/blink.bin)
set(BLD_ELF ${CMAKE_CURRENT_BINARY_DIR}/bootloader/bootloader.elf)

add_custom_target(fraise_bootloader ALL
    DEPENDS blink bootloader
    COMMAND ${CMAKE_OBJCOPY} --update-section .app_bin=${APP_BIN} ${BLD_ELF} fraise_bootloader.elf
    COMMAND picotool uf2 convert fraise_bootloader.elf fraise_bootloader.uf2
    COMMAND ${CMAKE_COMMAND} -E copy fraise_bootloader.uf2 "${boardDir}/fraise_bootloader.uf2"
)

