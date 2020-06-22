/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ha Thach for Adafruit Industries
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
 */

#include "boards.h"

#include "driver/gpio.h"
#include "driver/periph_ctrl.h"
#include "driver/rmt.h"
#include "led_strip.h"

#include "hal/usb_hal.h"
#include "soc/usb_periph.h"

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

//--------------------------------------------------------------------+
// MACRO TYPEDEF CONSTANT ENUM DECLARATION
//--------------------------------------------------------------------+

#ifdef PIN_NEOPIXEL
static led_strip_t *strip;
#endif

void board_init(void)
{

#ifdef PIN_NEOPIXEL
  // WS2812 Neopixel driver with RMT peripheral
  rmt_config_t config = RMT_DEFAULT_CONFIG_TX(PIN_NEOPIXEL, RMT_CHANNEL_0);
  config.clk_div = 2; // set counter clock to 40MHz

  rmt_config(&config);
  rmt_driver_install(config.channel, 0, 0);

  led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(NEOPIXEL_NUMBER, (led_strip_dev_t) config.channel);
  strip = led_strip_new_rmt_ws2812(&strip_config);
  strip->clear(strip, 100); // off led
  strip->set_brightness(strip, NEOPIXEL_BRIGHTNESS);
#endif

}

void board_teardown(void)
{

}

//--------------------------------------------------------------------+
// LED pattern
//--------------------------------------------------------------------+

#ifdef PIN_NEOPIXEL
TimerHandle_t blinky_tm = NULL;

static void neopixel_set(uint8_t r, uint8_t g, uint8_t b)
{
  strip->set_pixel(strip, 0, r, g, b);
  strip->refresh(strip, 100);
}

void led_blinky_cb(TimerHandle_t xTimer)
{
  (void) xTimer;
  static bool led_state = false;
  led_state = 1 - led_state; // toggle

  if ( led_state )
  {
    neopixel_set(0xff, 0x80, 0x00);
  }else
  {
    neopixel_set(0x00, 0x00, 0x00);
  }
}

void board_led_state(uint32_t state)
{
  switch(state)
  {
    case STATE_BOOTLOADER_STARTED:
    case STATE_USB_UNMOUNTED:
      neopixel_set(0xff, 0x00, 0x00);
    break;

    case STATE_USB_MOUNTED:
      neopixel_set( 0x00, 0xff, 0x00);
    break;

    case STATE_WRITING_STARTED:
      // soft timer for blinky
      blinky_tm = xTimerCreate(NULL, pdMS_TO_TICKS(50), true, NULL, led_blinky_cb);
      xTimerStart(blinky_tm, 0);
    break;

    case STATE_WRITING_FINISHED:
      xTimerStop(blinky_tm, 0);
      neopixel_set(0xff, 0x80, 0x00);
    break;

    default:
      neopixel_set(0x00, 0x00, 0x00);
    break;
  }
}

#endif
