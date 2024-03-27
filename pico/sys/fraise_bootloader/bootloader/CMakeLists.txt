add_executable(bootloader main.c fraise_bootdevice.c bootloader.c)

pico_generate_pio_header(bootloader ${CMAKE_CURRENT_LIST_DIR}/fraise.pio)

pico_add_extra_outputs(bootloader)

#set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-Wall")
#set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES COMPILE_OPTIONS "-DFRAISE_BLD_DEBUG")

# get every asm intermediate output
#target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -save-temps -fverbose-asm)

target_compile_options(bootloader PUBLIC -I${boardDir})

target_link_libraries(bootloader
    pico_stdlib
    hardware_irq
    hardware_watchdog
    hardware_structs
    hardware_pio
    hardware_resets
    hardware_sync
    hardware_flash
    #pico_unique_id
    cmsis_core
    fraise_eeprom
)

pico_enable_stdio_usb(bootloader 0)
pico_enable_stdio_uart(bootloader 0)

target_link_options(bootloader PUBLIC "-L${fraise_path}/pico/sys")
pico_set_linker_script(bootloader ${fraise_path}/pico/sys/device_bootloader.ld)
