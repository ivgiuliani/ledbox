#include <Arduino.h>
#include <FastLED.h>
#include <buttonctrl.h>

#include "RotaryEncoder.h"

#define NUM_LEDS 60
#define DATA_PIN 6

#define LED_BRIGHTNESS_STEP_MULTIPLIER 5

CRGB leds[NUM_LEDS];
RotaryEncoder encoder(D1, D2);
ButtonCtrl encoder_button(D3);

void setup() {
  delay(100);

  Serial.begin(9600);
  Serial.setTimeout(2000);

  while(!Serial) { }

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(0);

  encoder.begin();
  encoder_button.begin(true);
  
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }

  FastLED.show();
  Serial.println("OK.");
}

inline void fill_leds(uint8_t hue) {
  for(uint8_t i = 0; i < NUM_LEDS; i++) {
    leds[i].setHue(hue);
  }
}

static int16_t brightness = 0;
void inc_brightness(int8_t brightness_offset) {
  // Technically brightness is measured 0-255 and a uint8_t would
  // be enough. However we still use a int16_t as to avoid looping
  // within the range (e.g. jump from 255 to 0);
  brightness += brightness_offset;
  brightness = std::max(std::min(brightness, (int16_t)255), (int16_t)0);

  FastLED.setBrightness(brightness);

  Serial.print("brightness: ");
  Serial.println(brightness);
}

void loop() {
  static uint8_t color_idx = 0;

  CRGB::HTMLColorCode colors[] = {
    CRGB::White, CRGB::Red, CRGB::Green, CRGB::Blue
  };

  int8_t offset = encoder.read_offset();
  if (offset != 0) {
    inc_brightness(offset * LED_BRIGHTNESS_STEP_MULTIPLIER);
  }

  for(uint8_t i = 0; i < NUM_LEDS; i++) {
    leds[i] = colors[color_idx];
  }

  // fill_leds(CRGB::White);
  FastLED.show();

  if (encoder_button.handle() == Click) {
    color_idx++;
    color_idx %= 4;
  }
}
