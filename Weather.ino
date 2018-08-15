void getWeather() { // Using openweasthermap.org
  wDelay = pNow + 600; // delay between weather updates
  display.setCursor(0, row4);   // any error displayed in red on bottom row
  display.setTextColor(myRED);
  HTTPClient http;
  String URL = "http://api.openweathermap.org/data/2.5/weather?zip="
               + location + "&units=imperial&appid=" + String(owKey);
  String payload;
  long offset;
  http.setUserAgent(UserAgent);
  Serial.printf("[%d] ", ESP.getFreeHeap());
  if (!http.begin(URL)) {
    display.print("http fail");
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
        display.setCursor(12, row1);
        if (temperature < 40) display.setTextColor(myBLUE);
        else if (temperature < 50) display.setTextColor(myCYAN);
        else if (temperature < 60) display.setTextColor(myYELLOW);
        else if (temperature < 80) display.setTextColor(myGREEN);
        else if (temperature >= 80) display.setTextColor(myORANGE);
        else if (temperature >= 90) display.setTextColor(myRED);
        else display.setTextColor(myColor);
        display.fillRect(0, 0, 64, 6, myBLACK);
        display.printf("%2dF  %2d%%  %2d %s", round(temperature), humidity, round(wind), dir.c_str());
        String description = weather["main"];
        int id = weather["id"];
        int i = round(id / 100);
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
        if (w > 64) w = 0;
        else w = round((64 - w) / 2);
        display.setCursor(w, row4);
        display.print(description);
#ifdef SYSLOG_SERVER
        syslog.logf(LOG_INFO, "%2dF, %2d%%RH, %d %s, %s",
                    round(temperature), humidity, round(wind), dir.c_str(), description.c_str());
#endif
        Serial.printf("%2dF, %2d%%, %d %s (%d), %s (%d)\r\n",
                      round(temperature), humidity, round(wind), dir.c_str(), deg, description.c_str(), id);
      } else {
        display.print("json fail");
        Serial.println(F("getWeather: JSON parse failed!"));
        Serial.println(payload);
      }
    } else {
      display.print(stat);
      Serial.printf("getWeather: GET failed: %d %s\r\n", stat, http.errorToString(stat).c_str());
    }
  }
  http.end();
} // getWeather

String degreeDir(int degrees) {
  char* caridnals[] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW", "N" };
  return caridnals[round((degrees % 360) / 45)];
} // degreeDir

