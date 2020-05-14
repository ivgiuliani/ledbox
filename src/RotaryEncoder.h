#ifndef __ROTARY_H__
#define __ROTARY_H__

#include <Arduino.h>

// Uncomment to enable high-precision matching. Works well enough only with
// high-quality rotary encoders (i.e. of the magnetic kind).
// #define ROTARY_ENABLE_HIGH_PRECISION

template <uint8_t CLK_PIN, uint8_t DT_PIN>
class RotaryEncoder {
  /*
   * Decodes rotary encoder's input to avoid debounce effect. This is based off
   * a "valid input" table as described in
   * https://www.best-microcontroller-projects.com/rotary-encoder.html
   * (http://web.archive.org/web/20200430143612/https://www.best-microcontroller-projects.com/rotary-encoder.html),
   * however we also include noise mappings in the table here as to allow quick
   * adjustments of the encoder (seems like there's no significant overlap
   * between noise signals in CW and CCW which means we can do this with little
   * trouble).
   */
  public:
    RotaryEncoder() {}

    bool begin() {
      pinMode(CLK_PIN, INPUT);
      pinMode(CLK_PIN, INPUT_PULLUP);
      pinMode(DT_PIN, INPUT);
      pinMode(DT_PIN, INPUT_PULLUP);

      return true;
    }

    int8_t read_offset() {
      /**
       * MSB: most significant byte
       * LSB: least significant byte
       *            ____________            ___________
       *           |       ^    | ^      ^ |    ^
       * MSB       |       ^    | ^      ^ |    ^
       *      _____|       ^    |__________|    ^
       *                  ___________               ___
       *                 | ^      ^  |   ^      ^  |
       * LSB             | ^      ^  |   ^      ^  |
       *      ___________| ^      ^  |_____________|
       *                  (11)   (10)   (00)   (01)
       *
       * There's only 8 possible ways we can move:
       * 11->10, 10->00, 00->01 and 01->11 (similar counter-clockwise).
       *
       * The idea behind the rotary table is that you store the previous
       * and current state and just return an offset (+/-1) when this
       * combination ends up in a valid state. As there's plenty of noise
       * in a rotary encoder, we also allow some "noise" values, extracted
       * empirically during testing if these values only appear either when
       * turning the encoder clockwise or counter-clockwise (but not both).
       **/

      // A valid clockwise or counter-clockwise move returns 1, invalid returns 0.
      static const int8_t rot_enc_table[] = {
        0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0
      };

      this->encoder_code <<= 2;
      this->encoder_code |= digitalRead(DT_PIN) ? 0x02 : 0x00;
      this->encoder_code |= digitalRead(CLK_PIN) ? 0x01 : 0x00;
      this->encoder_code &= 0x0f;

      int8_t valid = rot_enc_table[(this->encoder_code & 0x0f)];

      if (valid) {
        this->enc_buffer <<= 4;
        this->enc_buffer |= this->encoder_code;

        return decode_offset(this->enc_buffer);
      }

      return 0;
    }

  private:
    uint8_t encoder_code = 0;
    uint16_t enc_buffer = 0;

    int8_t inline decode_offset(const int16_t st) {
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

        // Codes that appear in both clockwise and counter-clockwise signals
        // and therefore are just noise.
        case 0x18:
        case 0x28:
        case 0x41:
        case 0x42:
        case 0x81:
        case 0x7D:
        case 0x7E:
        case 0x82:
        case 0xBD:
        case 0xBE:
        case 0xE4:
          // most-significant byte, ignored in low precision mode
          return 0;

        // Clockwise signals
        case 0x14:
        case 0x17:
        case 0x71:
        case 0x8E:
        case 0xE8:
        case 0xEB:
        case 0xE7:
          return 1;

        // Counter-clockwise signals
        case 0x24:
        case 0x4D:
        case 0xD4:
        case 0xD7:
        case 0xDB:
        case 0x2B:
        case 0xB2:
          return -1;

        default:
          Serial.print("unknown rotary code: ");
          Serial.println(st & 0xFF, HEX);
    }

    return 0;
  }
};

#endif
