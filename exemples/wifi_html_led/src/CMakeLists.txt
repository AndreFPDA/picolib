cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2_w)
include(pico_sdk_import.cmake)

project(pico2w_wifi_exemplo C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(pico2w_wifi_exemplo
    exemple_wifi_led.c
)

target_include_directories(pico2w_wifi_exemplo PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Adiciona as bibliotecas necessárias para Wi-Fi e lwIP no Pico W / Pico 2 W
target_link_libraries(pico2w_wifi_exemplo
    pico_stdlib
    pico_cyw43_arch_lwip_poll
)

pico_enable_stdio_usb(pico2w_wifi_exemplo 1)
pico_enable_stdio_uart(pico2w_wifi_exemplo 0)

pico_add_extra_outputs(pico2w_wifi_exemplo)
