add_library(dcmotor INTERFACE)

target_sources(dcmotor INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/dcmotor.c)

target_include_directories(dcmotor INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dcmotor INTERFACE hardware_pwm hardware_adc)


