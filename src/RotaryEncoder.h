#ifndef __ROTARY_H__
#define __ROTARY_H__

#include <Arduino.h>

// #define ROTARY_ENABLE_HIGH_PRECISION

class RotaryEncoder {
  /*
   * Decodes rotary encoder's input to avoid debounce effect. This
   * is based off a "valid input" table as described in:
   * https://www.best-microcontroller-projects.com/rotary-encoder.html
   * (http://web.archive.org/web/20200430143612/https://www.best-microcontroller-projects.com/rotary-encoder.html)
   */
  public:
    RotaryEncoder(const uint8_t clk_pin, const uint8_t dt_pin) {
      this->clk_pin = clk_pin;
      this->dt_pin = dt_pin;
    }

    bool begin() {
      pinMode(this->clk_pin, INPUT);
      pinMode(this->clk_pin, INPUT_PULLUP);
      pinMode(this->dt_pin, INPUT);
      pinMode(this->dt_pin, INPUT_PULLUP);

      return true;
    }

    int8_t read_offset() {
      // A valid clockwise or counter-clockwise move returns 1, invalid returns 0.
      static const int8_t rot_enc_table[] = {
        0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0
      };

      this->prev_next_code <<= 2;
      if (digitalRead(this->dt_pin)) this->prev_next_code |= 0x02;
      if (digitalRead(this->clk_pin)) this->prev_next_code |= 0x01;
      this->prev_next_code &= 0x0f;

      int8_t valid = rot_enc_table[(this->prev_next_code & 0x0f)];

      if (valid) {
        this->store <<= 4;
        this->store |= this->prev_next_code;

        return decode_offset(this->store);
      }

      return 0;
    }

  private:
    uint8_t clk_pin;
    uint8_t dt_pin;
    uint8_t btn_pin;

    uint8_t prev_next_code = 0;
    uint16_t store = 0;

    int8_t decode_offset(const int16_t st) {
      // Serial.print(this->store & 0xFF, HEX);
      #ifdef ROTARY_ENABLE_HIGH_PRECISION
        const int16_t v = st;
      #else
        const int16_t v = st & 0xFF;
      #endif

      switch(v) {
        #ifdef ROTARY_ENABLE_HIGH_PRECISION
        /* High precision variant (ROTARY_ENABLE_HIGH_PRECISION) */
        case 0x8117:
        case 0x4117:
        case 0x7EE8:
          return 1;
        case 0xBDD4:
        case 0x422B:
        case 0x4114:
          return -1;
        #endif

        /* Low precision variant */
        case 0x18:
        case 0x41:
        case 0x42:
        case 0x81:
        case 0x7E:
        case 0xBD:
        case 0xE4:
          // most-significant byte, ignored in low precision mode
          return 0;

        // Clockwise signals
        case 0x14:
        case 0x17:
        case 0x71:
        case 0xE8:
        case 0xEB:
          return 1;

        // Counter-clockwise signals
        case 0x24:
        case 0x4D:
        case 0xD4:
        case 0xD7:
        case 0x2B:
          return -1;

        default:
          Serial.print("unknown rotary code: ");
          Serial.println(st & 0xFF, HEX);
    }

    return 0;
  }
};

#endif
