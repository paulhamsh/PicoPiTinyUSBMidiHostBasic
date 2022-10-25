# PicoPiTinyUSBMidiHostBasic

Absolute basic bare API USB Midi Host.
It will run a PIO-based USB host and print output over UART.

Based completely on the TinyUSB examples:

https://github.com/hathach/tinyusb/tree/master/examples/host/bare_api

Installation:

Install Pico Pi SDK.  
Copy these files to a suitable location.  

```
export PICO_SDK_PATH=~/pico2/pico-sdk  # your SDK location

mkdir
cd build
cmake -DFAMILY=rp2040 ..
make

```

The CMakelists.txt has some key changes:

```
cmake_minimum_required(VERSION 3.5)

###include(~/pico2/pico-sdk/lib/tinyusb/hw/bsp/family_support.cmake)
**include($ENV{PICO_SDK_PATH}/lib/tinyusb/hw/bsp/family_support.cmake)**

# gets PROJECT name for the example
family_get_project_name(PROJECT ${CMAKE_CURRENT_LIST_DIR})

project(${PROJECT})

# Checks this example is valid for the family and initializes the project
family_initialize_project(${PROJECT} ${CMAKE_CURRENT_LIST_DIR})

add_executable(${PROJECT})

# Example source
target_sources(${PROJECT} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
        )

# Example include
target_include_directories(${PROJECT} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        )

# Configure compilation flags and libraries for the example... see the corresponding function
# in hw/bsp/FAMILY/family.cmake for details.
family_configure_host_example(${PROJECT})

# For rp2040, un-comment to enable pico-pio-usb
family_add_pico_pio_usb(${PROJECT})

# due to warnings from other net source, we need to prevent error from some of the warnings options
target_compile_options(${PROJECT} PUBLIC
        -Wno-error=shadow
        -Wno-error=cast-align
        -Wno-error=cast-qual
        -Wno-error=redundant-decls
        -Wno-error=sign-conversion
        -Wno-error=conversion
        -Wno-error=sign-compare
        -Wno-error=unused-function

        -Wno-cast-qual
        -Wno-cast-align
        -Wno-sign-compare
        -Wno-shadow
        -Wno-redundant-decls
        -Wno-unused-function
        )


**pico_enable_stdio_usb(${PROJECT} 0)
pico_enable_stdio_uart(${PROJECT} 1)**
```
