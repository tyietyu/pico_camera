cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(pico_camera_lc C CXX ASM)
pico_sdk_init()

set(FAMILY rp2040)
set(BOARD pico_sdk)
set(TINYUSB_FAMILY_PROJECT_NAME_PREFIX "tinyusb_dev_")
add_subdirectory(${PICO_TINYUSB_PATH}/examples/device tinyusb_device_examples)

aux_source_directory(./main SOURCES)
aux_source_directory(./main/ov7725 OV7725_SRC)

add_executable(${PROJECT_NAME} ${SOURCES} ${OV7725_SRC})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/main)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/main/ov7725)

# pull in common dependencies
# pico_enable_stdio_usb(${PROJECT_NAME} 1)
# pico_enable_stdio_uart(${PROJECT_NAME} 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
    hardware_i2c
    hardware_uart
    hardware_resets
    hardware_irq
    pico_time
    pico_stdlib
    pico_multicore
    pico_unique_id
    tinyusb_device
    tinyusb_board
)