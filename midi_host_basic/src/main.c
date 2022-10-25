/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

// Edit pico-sdk/lib/tinyusb/hw/bsp/rp2040/family.c and change #define PICO_PIO_USB_PIN_DP 2 to 6 (or your own GPIO)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bsp/board.h"
#include "tusb.h"

tusb_desc_device_t desc_device;
uint8_t dev_addr;
uint8_t in_endpt;
uint8_t out_endpt;

uint8_t in_buf[64];
uint8_t out_buf[64];

uint8_t buf_setup[8] = {0x19, 0x9f, 0x0c, 0x7f, 0x19, 0x9f, 0x61, 0x21};


//--------------------------------------------------------------------+
// Blinking Task
//--------------------------------------------------------------------+

void led_blinking_task(void)
{
  const uint32_t interval_ms = 1000;
  static uint32_t start_ms = 0;

  static bool led_state = false;

  // Blink every interval ms
  if ( board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}

//--------------------------------------------------------------------+
// MIDI Interface
//--------------------------------------------------------------------+

void data_sent(tuh_xfer_t* xfer)
{
  if (xfer->result == XFER_RESULT_SUCCESS)
  {
    printf("Data sent success\n");
  }
}

void data_received(tuh_xfer_t* xfer)
{
  uint8_t* buf = (uint8_t*) xfer->user_data;

  if (xfer->result == XFER_RESULT_SUCCESS)
  {
    printf("[dev %u: ep %02x]: ", xfer->daddr, xfer->ep_addr);
    for(uint32_t i=0; i<xfer->actual_len; i++)
    {
      printf("%02X ", buf[i]);
    }
    printf("\n");
  }

  xfer->buflen = 64;
  xfer->buffer = buf;
  tuh_edpt_xfer(xfer);
}


//--------------------------------------------------------------------+
// Endpoint Descriptor
//--------------------------------------------------------------------+

void handle_endpoint(uint8_t daddr, tusb_desc_endpoint_t const* desc_ep) {
  if (tu_edpt_dir(desc_ep->bEndpointAddress) == TUSB_DIR_IN) {
    // skip if failed to open endpoint
    if (!tuh_edpt_open(daddr, desc_ep)) return;

    in_endpt = desc_ep->bEndpointAddress;

    tuh_xfer_t xfer =
    {
      .daddr       = dev_addr,
      .ep_addr     = in_endpt,
      .buflen      = 64,
      .buffer      = in_buf,
      .complete_cb = data_received,
      .user_data   = (uintptr_t) in_buf, 
    };

    // submit transfer for this EP
    tuh_edpt_xfer(&xfer);

    printf("Listen to [dev %u: ep %02x]\r\n", daddr, desc_ep->bEndpointAddress);
  }  

  if (tu_edpt_dir(desc_ep->bEndpointAddress) == TUSB_DIR_OUT)
  {
    // skip if failed to open endpoint
    if (!tuh_edpt_open(daddr, desc_ep) ) return;

    out_endpt = desc_ep->bEndpointAddress;
  }
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

void handle_config_descriptor(uint8_t daddr, tusb_desc_configuration_t const* desc_cfg) {
  printf("Config descriptor\n");

  uint8_t const* p_desc = (uint8_t const*) desc_cfg;
  uint8_t const* desc_end = p_desc + tu_le16toh(desc_cfg->wTotalLength);
 
  while (p_desc < desc_end) {
    switch (tu_desc_type(p_desc)) {
      case TUSB_DESC_CONFIGURATION:           //  0x02
      case TUSB_DESC_INTERFACE:               //  0x04
      case TUSB_DESC_INTERFACE_ASSOCIATION:   //  0x08
      case TUSB_DESC_CS_INTERFACE:            //  0x24
      case TUSB_DESC_CS_ENDPOINT:             //  0x25
        printf("Found descriptor type: %2x\n", tu_desc_type(p_desc));
        break;
      case TUSB_DESC_ENDPOINT:                //  0x05
        printf("Found a USB endpoint \n");
        handle_endpoint(daddr, (tusb_desc_endpoint_t const *) p_desc);
        break;
    }
    p_desc = tu_desc_next(p_desc);
  }
  printf("\n");
}

//--------------------------------------------------------------------+
// Device Descriptor
//--------------------------------------------------------------------+

void handle_device_descriptor(tuh_xfer_t* xfer)
{
  if (xfer->result != XFER_RESULT_SUCCESS)
  {
    printf("Failed to get device descriptor\n");
    return;
  }

  uint8_t const daddr = xfer->daddr;
  uint16_t temp_buf[500];
  
  printf("Device %u: ID %04x:%04x\r\n", daddr, desc_device.idVendor, desc_device.idProduct);

  // Get configuration descriptor with sync API
  if (tuh_descriptor_get_configuration_sync(daddr, 0, temp_buf, sizeof(temp_buf)) == XFER_RESULT_SUCCESS)
    handle_config_descriptor(daddr, (tusb_desc_configuration_t*) temp_buf);
  else 
    printf("Config descriptor failed\n");
}

//--------------------------------------------------------------------+
// Mount and unmount callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted (configured)
void tuh_mount_cb (uint8_t daddr)
{
  printf("Device attached, address = %d\n", daddr);
  dev_addr = daddr;
  in_endpt = 0;
  out_endpt = 0;

  tuh_descriptor_get_device(daddr, &desc_device, 18, handle_device_descriptor, 0);
}

/// Invoked when device is unmounted (bus reset/unplugged)
void tuh_umount_cb(uint8_t daddr)
{
  printf("Device removed, address = %d\n", daddr);
  dev_addr = 0;
  in_endpt = 0;
  out_endpt = 0;
}

//--------------------------------------------------------------------+
// Send config message to the Novation Launchkey 25
//--------------------------------------------------------------------+

void send_startup() {
  tuh_xfer_t xfer3 =
  {
    .daddr       = dev_addr,
    .ep_addr     = out_endpt,
    .buflen      = 8,
    .buffer      = buf_setup,
    .complete_cb = data_sent,
    .user_data   = (uintptr_t) buf_setup, 
    .actual_len  = 8,
  };

  tuh_edpt_xfer(&xfer3);
  printf("Output to [dev %u: ep %02x]\r\n", dev_addr, out_endpt);
}

//--------------------------------------------------------------------+
// Main
//--------------------------------------------------------------------+

int main(void)
{
  board_init();
  printf("MIDI Bare API\n");

  tuh_init(BOARD_TUH_RHPORT);

  while (1)
  {
    tuh_task();
    led_blinking_task();
  }

  return 0;
}

