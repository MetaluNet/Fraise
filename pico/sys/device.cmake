# to be invoked from ${projDir}/build with:
# cmake -S . -B  . -Dfraise_path=path/to/Fraise -is_pied_fruit=[0/1]
cmake_minimum_required(VERSION 3.13)

# get source files
file(GLOB_RECURSE srcfiles FOLLOW_SYMLINKS CONFIGURE_DEPENDS ${projDir}/*.c ${projDir}/*.cpp ${projDir}/*.cc)
list(FILTER srcfiles EXCLUDE REGEX "build.*/*|board/*")
message("source files recursive: ${srcfiles}")
add_executable(${projName} ${srcfiles})

if(NOT DEFINED fraise_board)
    # get board from main.c or main.cpp:
    if(EXISTS ${projDir}/main.c)
	    file(READ ${projDir}/main.c main_c)
    elseif(EXISTS ${projDir}/main.cpp)
	    file(READ ${projDir}/main.cpp main_c)
    endif()
    string(REGEX MATCH "\n[ \t]*#define[ \t]*BOARD[ \t]*[A-Za-z0-9_]*" board_pos_in_main "${main_c}")
    string(REPLACE "#define" "" board_pos_in_main ${board_pos_in_main})
    string(REPLACE "BOARD" "" board_pos_in_main ${board_pos_in_main})
    string(STRIP ${board_pos_in_main} fraise_board)
    unset(main_c)
    unset(board_pos_in_main)
    set(fraise_board ${fraise_board} PARENT_SCOPE)
endif()

if(EXISTS "${projDir}/boards/${fraise_board}")
	# user custom board
	get_filename_component(boardDir "${projDir}/boards/${fraise_board}" REALPATH)
else()
	# fraise defined board
	get_filename_component(boardDir "${fraise_path}/boards/${fraise_board}" REALPATH)
endif()
set(boardDir ${boardDir} PARENT_SCOPE)

message("board: '${fraise_board}' boardDir: '${boardDir}'")
target_compile_options(${projName} PUBLIC "-I${boardDir}")

# add system modules
add_subdirectory("${fraise_path}/pico/sys/" sysmodules)
# add pico modules
add_subdirectory("${fraise_path}/pico/" modules)


pico_add_extra_outputs(${projName})

target_link_options(${projName} PUBLIC "-L${fraise_path}/pico/sys/")
target_link_libraries(${projName} pico_stdlib)

pico_enable_stdio_usb(${projName} 0)
pico_enable_stdio_uart(${projName} 0)

if(DEFINED ENV{is_pied_fruit})
	set(is_pied_fruit $ENV{is_pied_fruit})
endif()

message("is_pied_fruit = ${is_pied_fruit}")
if(${is_pied_fruit} EQUAL 1)
	pico_set_linker_script(${projName} "${fraise_path}/pico/sys/master_app.ld")
	target_link_libraries(${projName} fraise fraise_master)
	pico_enable_stdio_usb(${projName} 1)
else()
	pico_set_linker_script(${projName} "${fraise_path}/pico/sys/device_app.ld")
	target_link_libraries(${projName} fraise fraise_device)
endif()


if(${is_pied_fruit} EQUAL 1)
	set(target_file ${projName}-0.hex)
else()
	set(target_file ${projName}.hex)
endif()

if (NOT DEFINED no_hex_copy)
add_custom_command(
	TARGET ${projName} POST_BUILD 
	COMMENT "-- Calculating memory usage"
	COMMAND ${size_command} -G ${projName}.elf > size.txt
	COMMENT "-- Copying hex file to source directory"
	COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/${projName}.hex" "${PROJECT_SOURCE_DIR}/../${target_file}"
)
endif()

if(EXISTS "${projDir}/config.cmake")
	message("including custom config.cmake")
	include("${projDir}/config.cmake")
endif()

set_target_properties(${projName} PROPERTIES COMPILE_FLAGS "-Wall")
