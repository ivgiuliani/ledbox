#include <Arduino.h>
#include <FastLED.h>
#include <buttonctrl.h>

#include "LedManager.h"
#include "RotaryEncoder.h"

#define NUM_LEDS 60
#define DATA_PIN 4
#define LED_BRIGHTNESS_STEP_MULTIPLIER 5

LedManager<NUM_LEDS, DATA_PIN> led_manager;
RotaryEncoder encoder(D1, D2);
ButtonCtrl encoder_button(D3, 1000, HIGH);

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(2000);

  while(!Serial) { }

  encoder.begin();
  encoder_button.begin(true);
  led_manager.begin();

  Serial.println("OK.");
}

void loop() {
  const int8_t offset = encoder.read_offset();
  if (offset != 0) {
    led_manager.adjust_brightness(offset * LED_BRIGHTNESS_STEP_MULTIPLIER);
  }

  const ButtonEvent btn_ev = encoder_button.handle();
  if (btn_ev == Click) {
    led_manager.rotate_next_color();
  } else if (btn_ev == LongClick) {
    Serial.println("OFF");
    led_manager.set_brightness(0);
  }

  FastLED.show();
}
