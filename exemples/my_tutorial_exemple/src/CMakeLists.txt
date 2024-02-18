cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

#project(picolib)

project(picolib
        LANGUAGES ASM C CXX
        VERSION 1.0
        DESCRIPTION "picolib"
)

set(FILE_ELF ${CMAKE_PROJECT_NAME}.elf)
set(FILE_HEX ${CMAKE_PROJECT_NAME}.hex)

# Initialize the SDK
pico_sdk_init()

add_executable(main
        main.cpp
)

target_sources(main
        PUBLIC
            ${CMAKE_CURRENT_LIST_DIR}/../../../lib/oledfx/SSD1306.cpp
            ${CMAKE_CURRENT_LIST_DIR}/../../../lib/oledfx/oledfx.cpp
)

 target_include_directories(main
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/../
)

# pull in common dependencies
target_link_libraries(main
        pico_stdlib
        hardware_i2c
)

# enable usb output, disable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(main)


