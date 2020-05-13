#ifndef __LED_MANAGER_H__
#define __LED_MANAGER_H__

#include <Arduino.h>
#include <FastLED.h>

#include "GammaCorrection.h"
         
// Even though the max brightness value is 255 as far as FastLED
// is concerned, we limit it to 160 by default as over this value
// the colors start losing accuracy (e.g. white leans towards a yellow).
#define DEFAULT_LED_MAX_BRIGHTNESS 160

template<uint16_t NUM_LEDS,
         uint8_t DATA_PIN,
         int16_t LED_MAX_BRIGHTNESS = DEFAULT_LED_MAX_BRIGHTNESS,
         EOrder RGB_ORDER = GRB,
         bool ENABLE_SERIAL_PRINT = true>
class LedManager {
  public:
    LedManager() {
      FastLED.addLeds<WS2812B, DATA_PIN, RGB_ORDER>(this->leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    };

    void begin() {
      set_brightness(0);

      // Apply black during initialization and force a show and only after that
      // prepulate it with the correct first color as tp avoid a "blink" from
      // the strip.
      fill_solid(CRGB::Black);
      FastLED.show();

      fill_solid(rotation_colors[0]);
    }

    void set_brightness(uint8_t b) {
      this->brightness = b;
      FastLED.setBrightness(b);
      if (ENABLE_SERIAL_PRINT) {
        Serial.print("brightness: ");
        Serial.println(this->brightness);
      }
    }

    void adjust_brightness(int8_t brightness_offset) {
      // Technically brightness is measured 0-255 and a uint8_t would
      // be enough. However we still use a int16_t as to avoid looping
      // outside the range (e.g. jump from 255 to 0);
      int16_t b = this->brightness;

      b += brightness_offset;
      b = std::max(std::min(b, (int16_t)LED_MAX_BRIGHTNESS), (int16_t)0);

      set_brightness(b);
    }

    void rotate_next_color() {
      this->color_idx++;
      this->color_idx %= this->rotation_colors.size();

      fill_solid(rotation_colors[this->color_idx]);
    }
  
  private:
    CRGB leds[NUM_LEDS];

    uint8_t color_idx = 0;
    uint8_t brightness = 0;

    std::vector<CRGB> rotation_colors = {
      gc_rgb(CRGB::White),
      gc_rgb(CRGB::Red),
      gc_rgb(CRGB::Magenta),
      gc_rgb(CRGB::Orange),
      gc_rgb(CRGB::Lime), // Actually green
      gc_rgb(CRGB::DeepSkyBlue),
      gc_rgb(CRGB::Blue),
    };

  inline void fill_solid(CRGB color) {
    if (ENABLE_SERIAL_PRINT) {
      Serial.print("fill_solid(");
      Serial.print(color.r, HEX); Serial.print(",");
      Serial.print(color.g, HEX); Serial.print(",");
      Serial.print(color.b, HEX);
      Serial.println(")");
    }
    for(uint8_t i = 0; i < NUM_LEDS; i++) {
      leds[i] = color;
    }
  }
};

#endif // __LED_MANAGER_H__
