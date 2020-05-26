#include <Arduino.h>
#include <buttonctrl.h>

#include "LedManager.h"
#include "LedWeb.h"
#include "RotaryEncoder.h"

#ifndef NUM_LEDS
#  error "NUM_LEDS must be defined at build time"
#endif

#define LED_BRIGHTNESS_STEP_MULTIPLIER 5

LedManager led_manager;
RotaryEncoder<D1, D2> encoder;
ButtonCtrl<D3, HIGH, INPUT_PULLUP> encoder_button(800);
LedWeb led_web;

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(2000);

  while(!Serial) { }

  encoder.begin();
  encoder_button.begin();
  led_manager.begin();
  led_web.begin(led_manager.get_control());

  Serial.println(F("System start OK."));
}

void loop() {
  static long last_brightness_0_ms = -1;

  const int8_t offset = encoder.read_offset();
  if (offset != 0) {
    LedControl *led_control = led_manager.get_control();

    if (led_control->get_brightness() != 0 || millis() - last_brightness_0_ms >= 100) {
      // When the brightness gets to 0 keep it at that and ignore further rotary
      // encoder updates for up to 100ms. This is a workaround on a 'debounce'
      // effect on the rotary encoder, where it can sometimes read noise values
      // when turned too fast and bounce back to positive even though it's been
      // turning counter clockwise.
      led_control->adjust_brightness(offset * LED_BRIGHTNESS_STEP_MULTIPLIER);
      if (led_control->get_brightness() == 0) {
        last_brightness_0_ms = millis();
      }
    }
  }

  const ButtonEvent btn_ev = encoder_button.handle();
  if (btn_ev == Click) {
    led_manager.click();
  } else if (btn_ev == LongClick) {
    led_manager.next_effect();
  }

  led_manager.handle();
  led_web.handle();
}
