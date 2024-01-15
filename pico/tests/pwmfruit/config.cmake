add_subdirectory("${fraise_path}/pico/smooth_pwm")
target_link_libraries(${CMAKE_PROJECT_NAME}
	smooth_pwm
)

