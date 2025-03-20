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

include(${boardDir}/board.cmake)


