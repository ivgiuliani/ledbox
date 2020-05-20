# ledbox

ESP8266-based controller for a ws2812b LED strip with a rotary encoder.

## Build (the hardward part)

This was built on an ESP8266 microcontroller, but should work on any
arduino-compatible board provided there's enough pins of the right type.

The current code assumes that:

* the rotary encoder A/B pin is connected to pin D1 and D2 (aka GPIO5 and GPIO4)
* the rotary encoder button is connected to pin D3
* the ws2812b data pin is connected to pin D4 (aka GPIO2).
* a relatively short ws2812b strip (I made it work with 60 leds) can be powered
  directly via the vin input on the ESP8266 on 5v. For longer strips an external
  power source becomes necessary.

## Build (the software part)

This project is using [platformio](https://platformio.org/) as build system.
To build the firmware:

```
pio run
```

To upload it to the device:

```
pio run --target upload
```
