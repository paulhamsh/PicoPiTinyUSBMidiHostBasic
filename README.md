# PicoPiTinyUSBMidiHostBasic

Absolute basic bare API USB Midi Host.
It will run a PIO-based USB host and print output over UART.

Based completely on the TinyUSB examples:

https://github.com/hathach/tinyusb/tree/master/examples/host/bare_api

Installation:

This is a full fresh installation of Pico SDK and this program.  

```
cd ~
mkdir pico_base
cd pico_base

sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
git clone https://github.com/raspberrypi/pico-sdk

# For USB support
cd pico-sdk
git submodule update --init
cd ~

export PICO_SDK_PATH=~/pico_base/pico-sdk  # your SDK location

# And for this program - start here but amend for your PICO_SDK_PATH

git clone https://github.com/paulhamsh/PicoPiTinyUSBMidiHostBasic

cd PicoPiTinyUSBMidiHostBasic/midi_host_basic

cp ~/pico_base/pico-sdk/external/pico_sdk_import.cmake .
mkdir build
cd build

cmake -DFAMILY=rp2040 ..
make

```


The CMakelists.txt has some key changes:

```

###include(~/pico2/pico-sdk/lib/tinyusb/hw/bsp/family_support.cmake)
include($ENV{PICO_SDK_PATH}/lib/tinyusb/hw/bsp/family_support.cmake)

```

This refers to the TinyUSB family cmake support and uses the PICO_SDK_PATH as root rather than is being relative to the TinyUSB library.  
