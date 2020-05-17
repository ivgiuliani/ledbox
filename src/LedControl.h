#ifndef __LED_CONTROL_H__
#define __LED_CONTROL_H__

#include <Arduino.h>
#include <FastLED.h>

// The max brightness value is 255 as far as FastLED is concerned but it may
// be necessary to lower the max brightness since after a certain threshold
// colors start losing accuracy (also this can be used as an implicit power
// limitation).
#define LED_MAX_BRIGHTNESS 255

/**
 * Collection of methods to control leds and ranges of leds
 */
class LedControl {
public:
  LedControl(CRGB leds[], const uint16_t num_leds) :
    leds(leds), num_leds(num_leds) {}

  void set_brightness(uint8_t brightness) {
    this->brightness = brightness;
    FastLED.setBrightness(brightness);
    #ifdef ENABLE_SERIAL_DEBUG
      Serial.print("set_brightness(");
      Serial.print(this->brightness);
      Serial.println(")");
    #endif
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

  uint8_t get_brightness() {
    return brightness;
  }

  /**
   * A more flexible version of the fill_solid method that FastLED provides
   * that allows to fill ranges of LEDs. By default, it will fill the whole
   * strip.
   */
  inline void fill_solid(const CRGB color, uint8_t first = 0, uint8_t count = -1) {
    if (count < 0) count = num_leds;
    #ifdef ENABLE_SERIAL_DEBUG
      Serial.print("fill_solid(");
      Serial.print(color.r, HEX); Serial.print(",");
      Serial.print(color.g, HEX); Serial.print(",");
      Serial.print(color.b, HEX); Serial.print(",");
      Serial.print("first="); Serial.print(first); Serial.print(",");
      Serial.print("count="); Serial.print(count);
      Serial.println(")");
    #endif

    // Boundary checks: make sure we only fill ranges in 0..num_leds-1
    first = first >= num_leds ? num_leds - 1 : first;
    count = first + count > num_leds ? num_leds - first : count;

    std::fill_n(&leds[first], count, color);
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

private:
  CRGB *leds;
  const uint16_t num_leds;
  uint8_t brightness = 0;
};

#endif // __LED_CONTROL_H__
