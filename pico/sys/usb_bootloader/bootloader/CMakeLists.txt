add_executable(bootloader bootloader.c)

# pull in common dependencies
target_link_libraries(bootloader
    pico_stdlib 
    hardware_flash
    hardware_structs
    hardware_resets
    hardware_sync
    cmsis_core
    fraise_eeprom
)

# create map/bin/hex file etc.
pico_add_extra_outputs(bootloader)

pico_enable_stdio_usb(bootloader 1)
pico_enable_stdio_uart(bootloader 0)

pico_set_float_implementation(bootloader none)
pico_set_double_implementation(bootloader none)

target_link_options(bootloader PUBLIC "-L${fraise_path}/pico/sys")
pico_set_linker_script(bootloader ${fraise_path}/pico/sys/master_bootloader.ld)

