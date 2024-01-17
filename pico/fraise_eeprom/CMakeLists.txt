add_library(fraise_eeprom INTERFACE)

target_sources(fraise_eeprom INTERFACE 
	${CMAKE_CURRENT_LIST_DIR}/fraise_eeprom.c
)

target_include_directories(fraise_eeprom INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(fraise_eeprom INTERFACE
	pico_stdlib
	hardware_flash
)

