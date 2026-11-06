cmake_minimum_required(VERSION 3.13)

# Use Pico 2 W by default; override with -DPICO_BOARD=pico_w or -DPICO_BOARD=pico2_w.
if (NOT DEFINED PICO_BOARD)
    set(PICO_BOARD pico2_w)
endif()

include(pico_sdk_import.cmake)

# Set distinct executable and UF2 names for each board.
if (PICO_BOARD STREQUAL "pico_w")
    set(TARGET_NAME "picow_wifi_led")
else()
    set(TARGET_NAME "pico2w_wifi_led")
endif()

project(${TARGET_NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(${TARGET_NAME}
    exemple_wifi_led.c
)

target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link the Wi-Fi and lwIP libraries for Pico W and Pico 2 W.
target_link_libraries(${TARGET_NAME}
    pico_stdlib
    pico_cyw43_arch_lwip_poll
)

pico_enable_stdio_usb(${TARGET_NAME} 1)
pico_enable_stdio_uart(${TARGET_NAME} 0)

pico_add_extra_outputs(${TARGET_NAME})
