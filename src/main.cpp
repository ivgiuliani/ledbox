#include <Arduino.h>
#include <FastLED.h>
#include <buttonctrl.h>

#include "RotaryEncoder.h"

#define NUM_LEDS 60
#define DATA_PIN 4

#define LED_BRIGHTNESS_STEP_MULTIPLIER 5

CRGB leds[NUM_LEDS];
RotaryEncoder encoder(D1, D2);
ButtonCtrl encoder_button(D3, 1000, HIGH);

std::vector<CRGB> rotation_colors = {
  CRGB::White,
  CRGB::Red,
  CRGB::Magenta,
  CRGB::Orange, // Looks like yellow but a tad better...
  CRGB::Green,
  CRGB::Cyan,
  CRGB::Blue,
};

inline void fill_leds(CRGB color) {
  Serial.print("set color(");
  Serial.print(color.r, HEX); Serial.print(",");
  Serial.print(color.g, HEX); Serial.print(",");
  Serial.print(color.b, HEX);
  Serial.println(")");
  for(uint8_t i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
  }
}

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(2000);

  while(!Serial) { }

  encoder.begin();
  encoder_button.begin(true);

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(0);
  fill_leds(CRGB::Black);
  FastLED.show();
  fill_leds(rotation_colors[0]);

  Serial.println("OK.");
}

static uint8_t brightness = 0;
void set_brightness(uint8_t b) {
  brightness = b;
  FastLED.setBrightness(b);
  Serial.print("brightness: ");
  Serial.println(brightness);
}

void adjust_brightness(int8_t brightness_offset) {
  // Technically brightness is measured 0-255 and a uint8_t would
  // be enough. However we still use a int16_t as to avoid looping
  // outside the range (e.g. jump from 255 to 0);
  int16_t b = brightness;
  b += brightness_offset;
  b = std::max(std::min(b, (int16_t)255), (int16_t)0);

  set_brightness(b);
}


void loop() {
  static uint8_t color_idx = 0;

  int8_t offset = encoder.read_offset();
  if (offset != 0) {
    adjust_brightness(offset * LED_BRIGHTNESS_STEP_MULTIPLIER);
  }

  const ButtonEvent btn_ev = encoder_button.handle();
  if (btn_ev == Click) {
    color_idx++;
    color_idx %= rotation_colors.size();

    fill_leds(rotation_colors[color_idx]);
  } else if (btn_ev == LongClick) {
    Serial.println("OFF");
    set_brightness(0);
  }

  FastLED.show();
}
