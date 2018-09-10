# myClock
## v0.9.2 PRE-RELEASE
### Copyright 2018 by David M Denney &lt;dragondaud@gmail.com&gt;

Master repository: https://github.com/dragondaud/myClock

Displays accurate time and current weather on a 64x32 pixel display. Sets timezone automatically from geoIP, uses the ESP's native NTP for time, and accepts OTA updates.

Display wired following https://www.instructables.com/id/Morphing-Digital-Clock/

Morphing digits code from https://github.com/hwiguna/HariFun_166_Morphing_Clock

Lookup TimeZone from IP using: http://ip-api.com/

Lookup Offset from TimeZone using: https://timezonedb.com/ which requires an API key to use.

Get current weather data from https://openweathermap.org/api which requires an API key to use.

### Requires

Arduino core for ESP8266: https://github.com/esp8266/Arduino

https://github.com/bblanchon/ArduinoJson/releases/tag/v5.13.2

https://github.com/adafruit/Adafruit-GFX-Library

https://github.com/2dom/PxMatrix

https://github.com/tzapu/WiFiManager

https://github.com/arcao/Syslog

