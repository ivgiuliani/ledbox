# ledbox

ESP8266-based controller for a ws2812b LED strip with a rotary encoder.

## Build (the hardward part)

This was built on an ESP8266 microcontroller, but should work on any
arduino-compatible board provided there's enough pins of the right type.

Required:

* an ESP8266 board
* a KY-040 rotary encoder (or any other 5-pins rotary encoder should work)
* a WS2812B LED strip

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

Add a file named `wifi.txt` where the first line is the wifi's essid and the
second line is the password.

## Schematics

Note that D4, VIN and GND is not connected in the PCB file as the WS2812b is
soldered directly to the ESP8266 dev board.

![Schematics](/_extra/schematic.png)

Also available the [EasyEDA project file](_extra/easyeda_prj.json).