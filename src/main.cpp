#include <Arduino.h>
#include <FastLED.h>
#include <buttonctrl.h>

#include "RotaryEncoder.h"

#define NUM_LEDS 60
#define DATA_PIN 6
#define COLOR_ORDER GRB

static const uint8_t brightness = 100;
CRGB leds[NUM_LEDS];

RotaryEncoder encoder(D1, D2);
ButtonCtrl encoder_button(D3);

void setup() {
  delay(100);

  Serial.begin(9600);
  Serial.setTimeout(2000);

  while(!Serial) { }

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(brightness);

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

void loop() {
  static uint8_t brightness = 0;
  static uint8_t color_idx = 0;

  CRGB::HTMLColorCode colors[] = {
    CRGB::White, CRGB::Red, CRGB::Green, CRGB::Blue
  };

  FastLED.setBrightness(brightness);

  int8_t offset = encoder.read_offset();
  if (offset != 0) {
    brightness += offset * 3;
    Serial.print("brightness: ");
    Serial.println(brightness);
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
