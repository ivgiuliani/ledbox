#ifndef __LED_MANAGER_H__
#define __LED_MANAGER_H__

#include <Arduino.h>
#include <FastLED.h>

#include "LedControl.h"
#include "LedAnim.h"

// The max brightness value is 255 as far as FastLED is concerned but it may
// be necessary to lower the max brightness since after a certain threshold
// colors start losing accuracy (also this can be used as an implicit power
// limitation).
#define DEFAULT_LED_MAX_BRIGHTNESS 255

template<uint16_t NUM_LEDS,
         uint8_t DATA_PIN,
         int16_t LED_MAX_BRIGHTNESS = DEFAULT_LED_MAX_BRIGHTNESS,
         EOrder RGB_ORDER = GRB>
class LedManager {
public:
  LedManager() {
    FastLED.addLeds<WS2812B, DATA_PIN, RGB_ORDER>(this->leds, NUM_LEDS).setCorrection(TypicalSMD5050);

    swap_animation(new InitialAnim());
  };

  void begin() {
    set_brightness(0);

    // The initial animation will have populated ever led with 'black'. Force a
    // show so to avoid a "blink" from the strip when it's first powered up.
    FastLED.show();

    swap_animation(new SolidAnim());
  }

  void set_brightness(uint8_t b) {
    this->brightness = b;
    FastLED.setBrightness(b);
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

  void click() {
    current_animation->click();
  }

  void off() {
    set_brightness(0);
  }

  void handle() {
    current_animation->handle();

    // rendering a frame every 16ms is roughly equivalent to 60fps
    EVERY_N_MILLIS(16) {
      current_animation->draw();
      FastLED.show();
    }
  }

private:
  LedControl control = LedControl(leds, NUM_LEDS);
  LedAnim *current_animation;
  CRGB leds[NUM_LEDS];

  uint8_t brightness = 0;

  void swap_animation(LedAnim *anim) {
    #ifdef ENABLE_SERIAL_DEBUG
      Serial.print("swap_animation(");
      Serial.print(anim->name());
      Serial.println(")");
    #endif
    const LedAnim *old_anim = current_animation;
    if (current_animation != NULL) {
      current_animation->end();
    }

    current_animation = anim;
    current_animation->begin(&control);

    if (old_anim != NULL) {
      delete old_anim;
    }
  }
};

#endif // __LED_MANAGER_H__
