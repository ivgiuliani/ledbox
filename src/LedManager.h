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

      fade_to(rotation_colors[this->color_idx]);
    }
  
  private:
    CRGB leds[NUM_LEDS];

    uint8_t color_idx = 0;
    uint8_t brightness = 0;

    std::vector<CRGB> rotation_colors = {
      gc_rgb(CRGB::White),
      gc_rgb(CRGB::Magenta),
      gc_rgb(CRGB::Red),
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

    void fade_to(const CRGB target, uint8_t step_amount = 75) {
      // Assumes all leds have the same color
      CRGB curr = this->leds[0];

      bool changed = true;
      while (changed) {
        const bool c1 = u8_blend_to_u8(curr.red,   target.red,   step_amount);
        const bool c2 = u8_blend_to_u8(curr.green, target.green, step_amount);
        const bool c3 = u8_blend_to_u8(curr.blue,  target.blue,  step_amount);
        changed = c1 == true || c2 == true || c3 == true;

        if (changed) {
          fill_solid(curr);
          FastLED.show();
          FastLED.delay(1);
        }
      }
    }

    bool u8_blend_to_u8(uint8_t &cur, const uint8_t target, const uint8_t amount) {
      const uint8_t delta = scale8_video(abs(target - cur), amount);

      if (cur < target) {
        cur += delta;
      } else {
        cur -= delta;
      }

      // Returns true when the value has changed.
      return delta != 0;
    }

};

#endif // __LED_MANAGER_H__
