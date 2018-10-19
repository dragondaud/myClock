# myClock
## v0.9.19 PRE-RELEASE
### Copyright 2018 by David M Denney &lt;dragondaud@gmail.com&gt;

Master repository: https://github.com/dragondaud/myClock

![myClock](/docs/clock+board.png)

Displays accurate time and current weather on a 64x32 pixel display. Sets timezone automatically from geoIP, uses the ESP's native NTP for time, and accepts OTA updates. Local WebServer allows updating and configuration, including color-picker to set digits color.

Designed to run on a Wemos-D1-Mini or NodeMCU, configured for CPU Freq 160Mhz and Flash size 4M (1M SPIFFS). Known to work on D1-mini-lite, with Flash size 1M (64k SPIFFS).

Postal code is used to specify weather display location. Time Zone must be from [time zone list](https://timezonedb.com/time-zones).

Uses LDR, pulled low with a 10k resistor, on analog input to set display brightness when below threshold value set in config.

Supports display of indoor temperature with DS18B20 connected to D3 pin, pulled high with a 10K resistor. This pin is listed as 'E' on the input connector, should not be connected. If using matrix shield, you must cut the "E" trace right before the pad.

![Cut E Trace](/docs/E-trace-cut.png)

Display wired following https://www.instructables.com/id/Morphing-Digital-Clock/

Easy hookup using Wemos Matrix Shield from https://github.com/hallard/WeMos-Matrix-Shield

Morphing digits code from https://github.com/hwiguna/HariFun_166_Morphing_Clock

Lookup TimeZone from IP using: http://ip-api.com/ (no API key required).

Lookup Offset from TimeZone using: https://timezonedb.com/ which requires an API key to use.

Get current weather data from https://openweathermap.org/api which requires an API key to use.

### Requires

Arduino IDE 1.8.7, for your platform, from https://www.arduino.cc/en/Main/Software

Arduino core for ESP8266, github version: https://github.com/esp8266/Arduino

https://github.com/bblanchon/ArduinoJson/releases/tag/v5.13.2

https://github.com/adafruit/Adafruit-GFX-Library

https://github.com/2dom/PxMatrix

https://github.com/tzapu/WiFiManager

https://github.com/arcao/Syslog, if rsyslog functionality desired.

### Scripts

install-Arduino.sh will install necessary components to build myClock on a Linux system.

build.sh will build, and optionally OTA update, myClock on Linux, MacOS or Windows.

update.sh will update myClock, core, and required libraries.
