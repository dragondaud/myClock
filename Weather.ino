// obtain and display weather information from openweathermap.org

void getWeather() {    // Using openweathermap.org
  wDelay = pNow + 900; // delay between weather updates
  display.setCursor(0, row4);   // any error displayed in red on bottom row
  display.setTextColor(myRED);
  HTTPClient http;
  String URL = PSTR("http://api.openweathermap.org/data/2.5/weather?zip=")
               + location + F("&units=%units%&lang=%lang%&appid=") + owKey;
  URL.replace("%units%", celsius ? "metric" : "imperial");
  URL.replace("%lang%", language);
  String payload;
  long offset;
  http.setUserAgent(UserAgent);
  if (!http.begin(URL)) {
#ifdef SYSLOG
    syslog.log(F("getWeather HTTP failed"));
#endif
    display.print(F("http fail"));
    Serial.println(F("getWeather: HTTP failed"));
  } else {
    int stat = http.GET();
    if (stat == HTTP_CODE_OK) {
      payload = http.getString();
      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(payload);
      if (root.success()) {
        String name = root["name"];
        JsonObject& weather = root["weather"][0];
        JsonObject& main = root["main"];
        float temperature = main["temp"];
        int humidity = main["humidity"];
        float wind = root["wind"]["speed"];
        int deg = root["wind"]["deg"];
        String dir = degreeDir(deg);
        int tc;
        if (celsius) tc = round(temperature * 1.8) + 32;
        else tc = round(temperature);
        if (tc <= 32) display.setTextColor(myCYAN);
        else if (tc <= 50) display.setTextColor(myLTBLUE);
        else if (tc <= 60) display.setTextColor(myBLUE);
        else if (tc <= 78) display.setTextColor(myGREEN);
        else if (tc <= 86) display.setTextColor(myYELLOW);
        else if (tc <= 95) display.setTextColor(myORANGE);
        else if (tc > 95) display.setTextColor(myRED);
        else display.setTextColor(myColor);
        display.fillRect(0, 0, 64, 6, myBLACK);
#ifdef DS18
        display.setCursor(0, row1);
        display.printf_P(PSTR("% 2d/% 2d%cF %2d%% %2d %s"),
                         Temp, round(temperature), 142, humidity, round(wind), dir.c_str());
#else
        display.setCursor(9, row1);
        display.printf_P(PSTR("% 2d%c%s %2d%% %2d %s"),
                         round(temperature), 142, celsius ? "C" : "F", humidity, round(wind), dir.c_str());
#endif
        String description = weather["description"];
        description.replace(F("intensity "), "");   // english description too long sometimes
        int id = weather["id"];
        int i = id / 100;
        switch (i) {
          case 2: // Thunderstorms
            display.setTextColor(myORANGE);
            break;
          case 3: // Drizzle
            display.setTextColor(myBLUE);
            break;
          case 5: // Rain
            display.setTextColor(myBLUE);
            break;
          case 6: // Snow
            display.setTextColor(myWHITE);
            break;
          case 7: // Atmosphere
            display.setTextColor(myYELLOW);
            break;
          case 8: // Clear/Clouds
            display.setTextColor(myGRAY);
            break;
        }
        int16_t  x1, y1, ww;
        uint16_t w, h;
        display.getTextBounds(description, 0, row4, &x1, &y1, &w, &h);
        display.fillRect(x1, y1, 64, 6, myBLACK);
        if (w < 64) x1 = (68 - w) >> 1;         // center weather description (getTextBounds returns too long)
        display.setCursor(x1, row4);
        display.print(description);
#ifdef SYSLOG
        syslog.logf("getWeather: %dF|%d%%RH|%d%s|%s",
                    round(temperature), humidity, round(wind), dir.c_str(), description.c_str());
#endif
        Serial.printf_P(PSTR("%2dF, %2d%%, %d %s (%d), %s (%d) \r\n"),
                        round(temperature), humidity, round(wind), dir.c_str(), deg, description.c_str(), id);
      } else {
        display.print(F("json fail"));
#ifdef SYSLOG
        syslog.log(F("getWeather JSON parse failed"));
        syslog.log(payload);
#endif
        Serial.println(F("getWeather: JSON parse failed!"));
        Serial.println(payload);
      }
    } else {
#ifdef SYSLOG
      syslog.logf("getWeather failed, GET reply %d", stat);
#endif
      display.print(stat);
      Serial.printf_P(PSTR("getWeather: GET failed: %d %s\r\n"), stat, http.errorToString(stat).c_str());
    }
  }
  http.end();
} // getWeather

String degreeDir(int degrees) {
  static const char* caridnals[] PROGMEM = { "N", "NE", "E", "SE", "S", "SW", "W", "NW", "N" };
  return caridnals[round((degrees % 360) / 45)];
} // degreeDir
