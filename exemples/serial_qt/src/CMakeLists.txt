cmake_minimum_required(VERSION 3.13)

# Importa as definições padrão do SDK do Raspberry Pi Pico
include(pico_sdk_import.cmake)

project(pico_usb_serial C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Inicializa o SDK
pico_sdk_init()

add_executable(pico_usb_serial
    main.cpp
)

# Vincula as bibliotecas básicas da arquitetura
target_link_libraries(pico_usb_serial pico_stdlib)

# -------------------------------------------------------------
# ATIVAÇÃO DO REDIRECIONAMENTO USB SERIAL
# -------------------------------------------------------------
# 1 = Ativa a saída do terminal padrão (printf / cout / getchar) via USB
pico_enable_stdio_usb(pico_usb_serial 1)

# 0 = Desativa a comunicação paralela pelos pinos UART físicos de hardware (GPIO 0 e 1)
pico_enable_stdio_uart(pico_usb_serial 0)
# -------------------------------------------------------------

# Gera arquivos finais de compilação (como o .uf2 para arrastar para o Pico)
pico_add_extra_outputs(pico_usb_serial)
