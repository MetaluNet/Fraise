target_link_options(${CMAKE_PROJECT_NAME} PUBLIC "-L${projDir}/../")
pico_set_linker_script(${CMAKE_PROJECT_NAME} "${projDir}/../device_app_dbg.ld")
pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 1)
set_target_properties(fraise_device PROPERTIES INTERFACE_COMPILE_OPTIONS "-DFRAISE_DEVICE_DEBUG")

