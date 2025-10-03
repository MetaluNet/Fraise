add_library(ws2812par INTERFACE)

target_sources(ws2812par INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/ws2812par.c)
pico_generate_pio_header(ws2812par ${CMAKE_CURRENT_LIST_DIR}/ws2812par.pio)

target_include_directories(ws2812par INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ws2812par INTERFACE hardware_pio hardware_dma fraise)

