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
  virtual void loop() {}
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

  void loop() {
    if (current_color == target_color) {
      return;
    }

    const bool changed = control->crgb_blend_to_crgb(current_color, target_color, 75);
    if (changed) {
      control->fill_solid(current_color);
    }
  }

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

class HueAnim : public LedAnim {
public:
  const char *name() { return "hue"; }

  void begin(LedControl *control) {
    LedAnim::begin(control);
    hue = 0;
  }

  void loop() {
    EVERY_N_MILLISECONDS(200) {
      hue++;
      const CRGB c = CHSV(hue, 255, 255);
      control->fill_solid(c);
    }
  }

private:
  uint8_t hue = 0;
};

/**
 * A port of "pacifica" from the FastLED examples with multi palette support.
 *
 * The trick here is to have 4 "waves" of light, each animated independently
 * and overlayed to each other.
 *
 * https://github.com/FastLED/FastLED/blob/master/examples/Pacifica/Pacifica.ino
 */
class WaveAnim : public LedAnim {
public:
  const char *name() { return "wave"; }

  void begin(LedControl *control) {
    LedAnim::begin(control);
  }

  void draw() {
    // Increment the four "color index start" counters, one for each wave layer.
    // Each is incremented at a different speed, and the speeds vary over time.
    const long now = millis();
    const uint32_t delta_ms = now - last_run_ms;

    const uint16_t speedfactor1 = beatsin16(3, 179, 269);
    const uint16_t speedfactor2 = beatsin16(4, 179, 269);

    const uint32_t delta_ms_1 = (delta_ms * speedfactor1) / 256;
    const uint32_t delta_ms_2 = (delta_ms * speedfactor2) / 256;
    const uint32_t delta_ms_21 = (delta_ms_1 + delta_ms_2) / 2;

    color_idx_start_1 += (delta_ms_1 * beatsin88(1011, 10, 13));
    color_idx_start_2 -= (delta_ms_21 * beatsin88(777, 8, 11));
    color_idx_start_3 -= (delta_ms_1 * beatsin88(501, 5, 7));
    color_idx_start_4 -= (delta_ms_2 * beatsin88(257, 4, 6));

    // Clear out the LED array to a dim background blue-green
    control->fill_solid(base_color);

    // Render each of four layers, with different scales and speeds, that vary over time
    layer(palette_1, color_idx_start_1, beatsin16(3, 11 * 256, 14 * 256), beatsin8(10, 70, 130), -beat16(301));
    layer(palette_2, color_idx_start_2, beatsin16(4,  6 * 256,  9 * 256), beatsin8(17, 40,  80), beat16(401));
    layer(palette_3, color_idx_start_3, 6 * 256, beatsin8(9, 10,38), 0-beat16(503));
    layer(palette_3, color_idx_start_4, 5 * 256, beatsin8(8, 10,28), beat16(601));

    // Add brighter 'whitecaps' where the waves lines up more
    add_whitecaps();

    // Deepen the blues and greens a bit
    deepen_colors();

    last_run_ms = now;
  }

private:
  const CRGB base_color = CRGB(2, 6, 10);

  CRGBPalette16 palette_1 = {
    0x000507, 0x000409, 0x00030B, 0x00030D,
    0x000210, 0x000212, 0x000114, 0x000117,
    0x000019, 0x00001C, 0x000026, 0x000031,
    0x00003B, 0x000046, 0x14554B, 0x28AA50
  };
  CRGBPalette16 palette_2 = {
    0x000507, 0x000409, 0x00030B, 0x00030D,
    0x000210, 0x000212, 0x000114, 0x000117,
    0x000019, 0x00001C, 0x000026, 0x000031,
    0x00003B, 0x000046, 0x0C5F52, 0x19BE5F
  };
  CRGBPalette16 palette_3 = {
    0x000208, 0x00030E, 0x000514, 0x00061A,
    0x000820, 0x000927, 0x000B2D, 0x000C33,
    0x000E39, 0x001040, 0x001450, 0x001860,
    0x001C70, 0x002080, 0x1040BF, 0x2060FF
  };

  uint16_t color_idx_start_1, color_idx_start_2, color_idx_start_3, color_idx_start_4;
  uint32_t last_run_ms = 0;

  void layer(CRGBPalette16& palette,
             uint16_t color_idx_start,
             uint16_t wavescale,
             uint8_t brightness,
             uint16_t ioff) {
    uint16_t c_idx = color_idx_start;
    uint16_t waveangle = ioff;
    uint16_t wavescale_half = (wavescale / 2) + 20;

    for(uint16_t i = 0; i < control->num_leds; i++) {
      waveangle += 250;

      uint16_t s16 = sin16(waveangle) + 32768;
      uint16_t cs = scale16(s16 , wavescale_half) + wavescale_half;

      c_idx += cs;

      uint16_t sindex16 = sin16(c_idx) + 32768;
      uint8_t sindex8 = scale16(sindex16, 240);
      CRGB palette_color = ColorFromPalette(palette, sindex8, brightness, LINEARBLEND);

      control->leds[i] += palette_color;
    }
  }

  // Add extra 'white' to areas where the 4 layers of light have lined up brightly
  inline void add_whitecaps() {
    const uint8_t base_threshold = beatsin8(9, 55, 65);
    uint8_t wave = beat8(7);

    for(uint16_t i = 0; i < control->num_leds; i++) {
      const uint8_t threshold = scale8(sin8(wave), 20) + base_threshold;
      const uint8_t avg_light = control->leds[i].getAverageLight();
      wave += 7;

      if (avg_light > threshold) {
        const uint8_t overage = avg_light - threshold;
        const uint8_t overage2 = qadd8(overage, overage);

        control->leds[i] += CRGB(overage, overage2, qadd8(overage2, overage2));
      }
    }
  }

  // Deepen the blues and greens
  inline void deepen_colors() {
    for(uint16_t i = 0; i < control->num_leds; i++) {
      control->leds[i].blue = scale8(control->leds[i].blue, 145);
      control->leds[i].green = scale8(control->leds[i].green, 200);
      control->leds[i] |= CRGB(2, 5, 7);
    }
  }
};

enum AnimEffect {
  Initial = -1,

  Solid = 0,
  Wave = 1,
  Hue = 2,
};

extern LedAnim* make_effect(int8_t effect) {
  switch(effect) {
    case AnimEffect::Initial:
      return new InitialAnim();
    case AnimEffect::Solid:
      return new SolidAnim();
    case AnimEffect::Hue:
      return new HueAnim();
    case AnimEffect::Wave:
      return new WaveAnim();
    default:
      #ifdef ENABLE_SERIAL_DEBUG
        Serial.print("Attempted to create invalid effect: ");
        Serial.println(effect);
      #endif
      return NULL;
  }
}

#endif // __LED_ANIM_H__
