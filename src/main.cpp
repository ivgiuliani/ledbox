#include <Arduino.h>
#include <FastLED.h>
#include <buttonctrl.h>

#include "GammaCorrection.h"
#include "RotaryEncoder.h"

#define NUM_LEDS 60
#define DATA_PIN 4

// Even though the max brightness value is 255 as far as FastLED
// is concerned, we limit it to 190 as over this value the colors
// start losing accuracy (e.g. white leans towards a yellow).
#define LED_MAX_BRIGHTNESS 190
#define LED_BRIGHTNESS_STEP_MULTIPLIER 5

CRGB leds[NUM_LEDS];
RotaryEncoder encoder(D1, D2);
ButtonCtrl encoder_button(D3, 1000, HIGH);

std::vector<CRGB> rotation_colors = {
  gc_rgb(CRGB::White),
  gc_rgb(CRGB::Red),
  gc_rgb(CRGB::Magenta),
  gc_rgb(CRGB::Orange),
  gc_rgb(CRGB::Green),
  gc_rgb(CRGB::Cyan),
  gc_rgb(CRGB::Blue),
};

struct {
  uint8_t color_idx = 0;
} current_state;

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
  b = std::max(std::min(b, (int16_t)LED_MAX_BRIGHTNESS), (int16_t)0);

  set_brightness(b);
}


void loop() {
  int8_t offset = encoder.read_offset();
  if (offset != 0) {
    adjust_brightness(offset * LED_BRIGHTNESS_STEP_MULTIPLIER);
  }

  const ButtonEvent btn_ev = encoder_button.handle();
  if (btn_ev == Click) {
    current_state.color_idx++;
    current_state.color_idx %= rotation_colors.size();

    fill_leds(rotation_colors[current_state.color_idx]);
  } else if (btn_ev == LongClick) {
    Serial.println("OFF");
    set_brightness(0);
  }

  FastLED.show();
}
