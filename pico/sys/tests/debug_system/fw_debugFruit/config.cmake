target_link_options(${CMAKE_PROJECT_NAME} PUBLIC "-L${projDir}/../")
pico_set_linker_script(${CMAKE_PROJECT_NAME} "${projDir}/../device_app_dbg.ld")

