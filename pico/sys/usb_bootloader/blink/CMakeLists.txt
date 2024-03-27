get_filename_component(projDir ${CMAKE_CURRENT_SOURCE_DIR} REALPATH)

project(blink C CXX ASM)
pico_sdk_init()

set(is_pied_fruit 1)
set(projName blink)
set(no_hex_copy 1)

include(${fraise_path}/pico/sys/device.cmake)

