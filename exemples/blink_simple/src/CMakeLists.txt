cmake_minimum_required(VERSION 3.12)

set(PICO_BOARD pico_w)

include(pico_sdk_import.cmake)

project(picolib
        LANGUAGES ASM C CXX
        VERSION 1.0
        DESCRIPTION "picolib"
)

set(FILE_ELF ${CMAKE_PROJECT_NAME}.elf)
set(FILE_HEX ${CMAKE_PROJECT_NAME}.hex)
set(FILE_UF2 ${CMAKE_PROJECT_NAME}.uf2)

# Initialize the SDK
pico_sdk_init()
add_executable(blink_simple
        blink_simple.c
)

# pull in common dependencies
target_link_libraries(blink_simple pico_stdlib pico_cyw43_arch_none)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(blink_simple)
