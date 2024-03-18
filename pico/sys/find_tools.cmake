message("fraise_path: '${fraise_path}'")

get_filename_component(fraise_toolchain_path ${fraise_path}/../Fraise-toolchain/ REALPATH)
set(PICO_TOOLCHAIN_PATH ${fraise_toolchain_path}/gcc)
set(PICO_SDK_PATH ${fraise_toolchain_path}/pico-sdk)

set(Pioasm_FOUND 1)
if (CMAKE_HOST_WIN32)
    set(Pioasm_EXECUTABLE ${fraise_toolchain_path}/bin/pioasm.exe)
else()
    set(Pioasm_EXECUTABLE ${fraise_toolchain_path}/bin/pioasm)
endif()
add_executable(Pioasm IMPORTED)
set_property(TARGET Pioasm PROPERTY IMPORTED_LOCATION ${Pioasm_EXECUTABLE})

set(ELF2UF2_FOUND 1)
set(ELF2UF2_EXECUTABLE ${fraise_toolchain_path}/bin/elf2uf2)
add_executable(ELF2UF2 IMPORTED)
set_property(TARGET ELF2UF2 PROPERTY IMPORTED_LOCATION ${ELF2UF2_EXECUTABLE})

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

