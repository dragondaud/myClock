# myClock
## v0.10.4 PRE-RELEASE
### Copyright 2019 by David M Denney &lt;dragondaud@gmail.com&gt;

Master repository: https://github.com/dragondaud/myClock

![myClock](/docs/clock+board.png)

Recommended enclosure for myClock made from a [shadowbox frame from Michaels](https://www.michaels.com/black-belmont-shadowbox-by-studio-decor/M10468367.html?dwvar_M10468367_size=9%22%20x%209%22&dwvar_M10468367_color=Black), and [two-way mirror from TAP Plastics](https://www.tapplastics.com/product/plastics/cut_to_size_plastic/two_way_mirrored_acrylic/558).

Displays accurate time and current weather on a 64x32 pixel display. Sets timezone automatically from geoIP, uses the ESP's native NTP for time, and accepts OTA updates. Local WebServer allows updating and configuration, including color-picker to set digits color.

Designed to run on a Wemos-D1-Mini or NodeMCU, configured for CPU Freq 160Mhz and Flash size 4M (1M SPIFFS). Known to work on D1-mini-lite, with Flash size 1M (64k SPIFFS), and now also supports ESP32.

Unwanted Serial output may be directed to NullStream, or on ESP32 output can be sent to Bluetooth.

Postal code is used to specify weather display location. Time Zone must be from [time zone list](https://timezonedb.com/time-zones).

Uses LDR, pulled low with a 10k resistor, on analog input to set display brightness when below threshold value set in config.

Supports display of indoor temperature with DS18B20 connected to D3 pin, pulled high with a 10K resistor.

Display wired following https://www.instructables.com/id/Morphing-Digital-Clock/

Easy hookup using Wemos Matrix Shield from https://github.com/hallard/WeMos-Matrix-Shield

Morphing digits code from https://github.com/hwiguna/HariFun_166_Morphing_Clock

Lookup TimeZone from IP using: http://ip-api.com/ (no API key required).

Lookup Offset from TimeZone using: https://timezonedb.com/ which requires an API key to use.

Get current weather data from https://openweathermap.org/api which requires an API key to use.

### Requires

Arduino IDE 1.8.9, for your platform, from https://www.arduino.cc/en/Main/Software

Arduino core for ESP8266, version 2.5.0: https://github.com/esp8266/Arduino

or Arduino core for ESP32, from: https://github.com/espressif/arduino-esp32/releases/tag/1.0.2

https://github.com/bblanchon/ArduinoJson/tree/5.x

https://github.com/adafruit/Adafruit-GFX-Library

https://github.com/2dom/PxMatrix

https://github.com/tzapu/WiFiManager

https://github.com/arcao/Syslog if rsyslog functionality desired.

https://github.com/milesburton/Arduino-Temperature-Control-Library if DS18B20 used.

https://github.com/PaulStoffregen/OneWire if DS18B20 used.

### Scripts

install-Arduino.sh will install necessary components to build myClock on a Linux system.

build.sh will build, update, flash OTA or Serial, myClock on Linux, MacOS or Windows.

### Notes

Switching between ESP8266 and ESP32 platform requires deleting preferences.txt which the update script will do automatically when updating core.
