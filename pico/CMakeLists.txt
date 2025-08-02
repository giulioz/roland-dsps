cmake_minimum_required(VERSION 3.12)
set(PICO_DEFAULT_BINARY_TYPE copy_to_ram)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

project(picodsp VERSION 1.0.0)
pico_sdk_init()

add_executable(picodsp
  main.cpp
  ../emulator/compiled.cpp
  ../emulator/runtime.h
)

# target_include_directories(picodsp INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(picodsp PRIVATE ../emulator/)

target_link_libraries(picodsp
  pico_stdlib
  # hardware_dma
  # hardware_pio
  # hardware_spi
  # hardware_i2c
  # pico_multicore
  # tinyusb_device
  # tinyusb_board
)

pico_enable_stdio_usb(picodsp 1)
pico_enable_stdio_uart(picodsp 0)
pico_add_extra_outputs(picodsp)
