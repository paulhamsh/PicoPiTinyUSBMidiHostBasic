# PicoPiTinyUSBMidiHostBasic

Absolute basic bare API USB Midi Host

Based completely on the TinyUSB examples:

https://github.com/hathach/tinyusb/tree/master/examples/host/bare_api

Installation:

Install Pico Pi SDK.  
Copy these files to a suitable location.  

```
export PICO_SDK_PATH=~/pico2/pico-sdk  # your SDK location

cd build
cmake -DFAMILY=rp2040 ..
make

```
