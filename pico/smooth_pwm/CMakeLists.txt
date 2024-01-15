add_library(smooth_pwm INTERFACE)

target_sources(smooth_pwm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/smooth_pwm.c)

target_include_directories(smooth_pwm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(smooth_pwm INTERFACE hardware_pwm)


