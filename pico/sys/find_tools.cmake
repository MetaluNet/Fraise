get_filename_component(fraise_toolchain_path ${fraise_path}/../Fraise-toolchain/ REALPATH)

if (CMAKE_HOST_WIN32)
    set(PICO_TOOLCHAIN_PATH ${fraise_toolchain_path}/pico-windows/gcc-arm-none-eabi)
    set(PICO_SDK_PATH ${fraise_toolchain_path}/pico-windows/pico-sdk)
    set(PIOASM_EXECUTABLE ${fraise_toolchain_path}/pico-windows/pico-sdk-tools/pioasm.exe)
    set(ELF2UF2_EXECUTABLE ${fraise_toolchain_path}/pico-windows/pico-sdk-tools/elf2uf2.exe)
else()
    set(PICO_TOOLCHAIN_PATH ${fraise_toolchain_path}/gcc)
    set(PICO_SDK_PATH ${fraise_toolchain_path}/pico-sdk)
    set(PIOASM_EXECUTABLE ${fraise_toolchain_path}/bin/pioasm)
    set(ELF2UF2_EXECUTABLE ${fraise_toolchain_path}/bin/elf2uf2)
endif()

message("PICO_TOOLCHAIN_PATH: '${PICO_TOOLCHAIN_PATH}'")
message("PICO_SDK_PATH: '${PICO_SDK_PATH}'")

set(size_command ${PICO_TOOLCHAIN_PATH}/bin/arm-none-eabi-size)

set(Pioasm_FOUND 1)
add_executable(Pioasm IMPORTED)
set_property(TARGET Pioasm PROPERTY IMPORTED_LOCATION ${PIOASM_EXECUTABLE})

set(ELF2UF2_FOUND 1)
add_executable(ELF2UF2 IMPORTED)
set_property(TARGET ELF2UF2 PROPERTY IMPORTED_LOCATION ${ELF2UF2_EXECUTABLE})

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

