#ifndef __LED_MANAGER_H__
#define __LED_MANAGER_H__

#include <Arduino.h>
#include <FastLED.h>

#include "GammaCorrection.h"

// The max brightness value is 255 as far as FastLED is concerned but it may
// be necessary to lower the max brightness since after a certain threshold
// colors start losing accuracy (also this can be used as an implicit power
// limitation).
#define DEFAULT_LED_MAX_BRIGHTNESS 255

template<uint16_t NUM_LEDS,
         uint8_t DATA_PIN,
         int16_t LED_MAX_BRIGHTNESS = DEFAULT_LED_MAX_BRIGHTNESS,
         EOrder RGB_ORDER = GRB,
         bool ENABLE_SERIAL_PRINT = true>
class LedManager {
public:
  LedManager() {
    FastLED.addLeds<WS2812B, DATA_PIN, RGB_ORDER>(this->leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  };

  void begin() {
    set_brightness(0);

    // Apply black during initialization and force a show and only after that
    // prepulate it with the correct first color as tp avoid a "blink" from
    // the strip when it's first powered up.
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

  void off() {
    const CRGB curr = current_color();

    fade_to(CRGB::Black, 50);
    set_brightness(0);

    fill_solid(curr);
  }

  void handle() {
    FastLED.show();
  }

private:
  CRGB leds[NUM_LEDS];

  uint8_t color_idx = 0;
  uint8_t brightness = 0;

  const std::vector<CRGB> rotation_colors = {
    gc_rgb(CRGB::White),
    gc_rgb(CRGB::Magenta),
    gc_rgb(CRGB::Red),
    gc_rgb(CRGB::Orange),
    gc_rgb(CRGB::Lime), // Actually green
    gc_rgb(CRGB::DeepSkyBlue),
    gc_rgb(CRGB::Blue),
  };

  inline CRGB current_color() {
    // Assumes all leds share the same color
    return leds[0];
  }

  inline void fill_solid(const CRGB color, uint8_t first = 0, uint8_t count = NUM_LEDS) {
    if (ENABLE_SERIAL_PRINT) {
      Serial.print("fill_solid(");
      Serial.print(color.r, HEX); Serial.print(",");
      Serial.print(color.g, HEX); Serial.print(",");
      Serial.print(color.b, HEX); Serial.print(",");
      Serial.print("first="); Serial.print(first); Serial.print(",");
      Serial.print("count="); Serial.print(count);
      Serial.println(")");
    }

    // Boundary checks: make sure we only fill ranges in 0..NUM_LEDS-1
    first = first >= NUM_LEDS ? NUM_LEDS - 1 : first;
    count = first + count > NUM_LEDS ? NUM_LEDS - first : count;

    std::fill_n(&leds[first], count, color);
  }

  /**
   * Fades to the target color. This is a blocking method, as such whilst
   * it's possible to get a slower animation, all input/output signals are
   * locked while the animation is taking place.
   *
   * @param step_amount how much to progress towards the target color for each step.
   *        This value is expressed in 256ths, so for example 75 is ~30%.
   */
  void fade_to(const CRGB target, uint8_t step_amount = 75) {
    CRGB curr = current_color();

    while (bool changed = crgb_blend_to_crgb(curr, target, step_amount)) {
      fill_solid(curr);
      FastLED.show();
      FastLED.delay(1);
    }
  }

  inline bool u8_blend_to_u8(uint8_t &curr,
                             const uint8_t target,
                             const uint8_t scale) {
    const uint8_t delta = scale8_video(abs(target - curr), scale);
    curr += curr < target ? delta : -delta;

    // Returns true when the value has changed.
    return delta != 0;
  }

  inline bool crgb_blend_to_crgb(CRGB &curr,
                                  CRGB target,
                                  const uint8_t scale) {
    const bool r = u8_blend_to_u8(curr.red,   target.red,   scale);
    const bool g = u8_blend_to_u8(curr.green, target.green, scale);
    const bool b = u8_blend_to_u8(curr.blue,  target.blue,  scale);

    // Returns true when any value has changed.
    return r == true || g == true || b == true;
  }
};

#endif // __LED_MANAGER_H__
