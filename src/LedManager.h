#ifndef __LED_MANAGER_H__
#define __LED_MANAGER_H__

#define FASTLED_ESP8266_RAW_PIN_ORDER

#include <Arduino.h>
#include <FastLED.h>

#include "LedControl.h"
#include "LedAnim.h"

class LedManager {
public:
  LedManager() {
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(this->leds, NUM_LEDS).setCorrection(TypicalSMD5050);

    swap_animation(AnimEffect::Initial);
  };

  void begin() {
    control.set_brightness(0);

    // The initial animation will have populated ever led with 'black'. Force a
    // show so to avoid a "blink" from the strip when it's first powered up.
    FastLED.show();

    swap_animation(AnimEffect::Solid);
  }

  LedControl *get_control() {
    return &control;
  }

  void click() {
    current_animation->click();
  }

  void handle() {
    current_animation->loop();

    // rendering a frame every 16ms is roughly equivalent to 60fps
    EVERY_N_MILLIS(16) {
      current_animation->draw();
      FastLED.show();
    }
  }

  void next_effect() {
    current_effect++;
    if (current_effect >= 3) {
      current_effect = 0;
    }

    swap_animation(current_effect);
  }

private:
  CRGB leds[NUM_LEDS];
  LedControl control = LedControl(leds);
  LedAnim *current_animation;

  int8_t current_effect = AnimEffect::Initial;

  uint8_t brightness = 0;

  void swap_animation(int8_t effect) {
    current_effect = effect;
    swap_animation(make_effect(effect));
  }

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
