#ifndef __LED_ANIM_H__
#define __LED_ANIM_H__

#include <FastLED.h>
#include "LedControl.h"
#include "GammaCorrection.h"

class LedAnim {
public:
  LedAnim() {}
  virtual ~LedAnim() {
    control = NULL;
  }

  virtual void begin(LedControl *control) { this->control = control; }
  virtual void end() {}
  virtual void click() {}
  virtual void handle() {}
  virtual void draw() {}
  virtual const char *name() = 0;

protected:
  LedControl *control;
  uint16_t led_count = 0;
};

// Animation used during the initialization of the strip. Will output black as
// to avoid a "blink" from the strip when it's first powered up.
class InitialAnim : public LedAnim {
  const char *name() { return "initial"; }

  void begin(LedControl *control) {
    LedAnim::begin(control);
    control->fill_solid(CRGB::Black);
  }
};

// Simple solid color that rotates on click
class SolidAnim : public LedAnim {
public:
  const char *name() { return "solid"; }

  void begin(LedControl *control) {
    LedAnim::begin(control);

    color_idx = 0;
    current_color = rotation_colors[color_idx];
    target_color = rotation_colors[color_idx];

    control->fill_solid(current_color);
  }

  void click() {
    this->color_idx++;
    this->color_idx %= rotation_colors.size();
    target_color = rotation_colors[color_idx];
  }

  void handle() {
    if (current_color == target_color) {
      return;
    }

    const bool changed = control->crgb_blend_to_crgb(current_color, target_color, 75);
    if (changed) {
      control->fill_solid(current_color);
    }
  }

  void draw() {}

private:
  uint8_t color_idx = 0;
  CRGB target_color;
  CRGB current_color;

  const std::vector<CRGB> rotation_colors = {
    gc_rgb(CRGB::White),
    gc_rgb(CRGB::Magenta),
    gc_rgb(CRGB::Red),
    gc_rgb(CRGB::Orange),
    gc_rgb(CRGB::Lime), // Actually green
    gc_rgb(CRGB::DeepSkyBlue),
    gc_rgb(CRGB::Blue),
  };
};

#endif // __LED_ANIM_H__
