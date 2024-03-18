# to be invoked from ${projDir}/build with:
# cmake -S . -B  . -Dfraise_path=path/to/Fraise -is_pied_fruit=[0/1]
cmake_minimum_required(VERSION 3.13)

# get source files
file(GLOB_RECURSE srcfiles FOLLOW_SYMLINKS CONFIGURE_DEPENDS ${projDir}/*.c ${projDir}/*.cpp ${projDir}/*.cc)
list(FILTER srcfiles EXCLUDE REGEX "build.*/*|board/*")
message("source files recursive: ${srcfiles}")
add_executable(${CMAKE_PROJECT_NAME} ${srcfiles})

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
if(EXISTS "${projDir}/boards/${fraise_board}")
	# user custom board
	get_filename_component(boardDir "${projDir}/boards/${fraise_board}" REALPATH)
else()
	# fraise defined board
	get_filename_component(boardDir "${fraise_path}/boards/${fraise_board}" REALPATH)
endif()
message("board: '${fraise_board}' boardDir: '${boardDir}'")
unset(main_c)
unset(board_pos_in_main)
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC "-I${boardDir}")

# add system modules
add_subdirectory("${fraise_path}/pico/sys/" sysmodules)
# add pico modules
add_subdirectory("${fraise_path}/pico/" modules)


pico_add_extra_outputs(${CMAKE_PROJECT_NAME})

target_link_options(${CMAKE_PROJECT_NAME} PUBLIC "-L${fraise_path}/pico/sys/")
target_link_libraries(${CMAKE_PROJECT_NAME} pico_stdlib)

pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 0)
pico_enable_stdio_uart(${CMAKE_PROJECT_NAME} 0)

if(DEFINED ENV{is_pied_fruit})
	set(is_pied_fruit $ENV{is_pied_fruit})
endif()

message("is_pied_fruit = ${is_pied_fruit}")
if(${is_pied_fruit} EQUAL 1)
	pico_set_linker_script(${CMAKE_PROJECT_NAME} "${fraise_path}/pico/sys/master_app.ld")
	target_link_libraries(${CMAKE_PROJECT_NAME} fraise fraise_master)
	pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 1)
else()
	pico_set_linker_script(${CMAKE_PROJECT_NAME} "${fraise_path}/pico/sys/device_app.ld")
	target_link_libraries(${CMAKE_PROJECT_NAME} fraise fraise_device)
endif()


if(${is_pied_fruit} EQUAL 1)
	set(target_file ${CMAKE_PROJECT_NAME}-0.hex)
else()
	set(target_file ${CMAKE_PROJECT_NAME}.hex)
endif()

add_custom_command(
	TARGET ${CMAKE_PROJECT_NAME} POST_BUILD 
	COMMAND ${CMAKE_COMMAND} -E echo "-- Calculating memory usage"
	COMMAND arm-none-eabi-size -G ${CMAKE_PROJECT_NAME}.elf > size.txt
	COMMAND ${CMAKE_COMMAND} -E echo "-- Copying hex file to source directory"
	COMMAND ${CMAKE_COMMAND} -E 
		copy "${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}.hex" "${PROJECT_SOURCE_DIR}/../${target_file}"
)

if(EXISTS "${projDir}/config.cmake")
	message("including custom config.cmake")
	include("${projDir}/config.cmake")
endif()

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-Wall")

