target_link_libraries(${CMAKE_PROJECT_NAME} ws2812par)
set(NUM_PIXELS 100)
add_compile_definitions(NUM_PIXELS=${NUM_PIXELS})
add_compile_definitions(MAX_NUM_PIXELS=${NUM_PIXELS})
