# myClock
## v0.9.1 EXPERIMENTAL
### Copyright 2018 by David M Denney <dragondaud@gmail.com>

Master repository: https://github.com/dragondaud/myClock

Displays accurate time and current weather on a 64x32 pixel display. Sets timezone automatically from geoIP, uses the ESP's native NTP for time, and accepts OTA updates.

Display wired following https://www.instructables.com/id/Morphing-Digital-Clock/

Morphing digits code from https://github.com/hwiguna/HariFun_166_Morphing_Clock

Lookup TimeZone from IP using: http://ip-api.com/

Lookup Offset from TimeZone using: https://timezonedb.com/ which requires an API key to use.

Get current weather data from https://openweathermap.org/api which requires an API key to use.

Built on Arduino core for ESP8266: https://github.com/esp8266/Arduino

Requires
https://github.com/bblanchon/ArduinoJson/releases/tag/v5.13.2
https://github.com/2dom/PxMatrix
https://github.com/adafruit/Adafruit-GFX-Library
https://github.com/tzapu/WiFiManager
