add_library(audio INTERFACE)

#target_sources(audio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/smooth_pwm.c)

add_subdirectory(pico_audio)
add_subdirectory(pico_audio_pwm)
add_subdirectory(pico_util_buffer)

target_include_directories(audio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(audio INTERFACE pico_audio pico_audio_pwm)


