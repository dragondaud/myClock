void getWeather() { // Using openweasthermap.org
  wDelay = pNow + 300; // delay between weather updates
  display.fillRect(0, 0, 64, 10, myBLACK);
  display.setCursor(3, row1);
  display.setTextColor(myRED);
  HTTPClient http;
  String URL = "http://api.openweathermap.org/data/2.5/weather?zip="
               + location + "&units=imperial&appid=" + String(owKey);
  String payload;
  long offset;
  http.setUserAgent(UserAgent);
  if (!http.begin(URL)) {
    display.print("http fail");
    Serial.println(F("getOffset: HTTP failed"));
  } else {
    int stat = http.GET();
    if (stat == HTTP_CODE_OK) {
      payload = http.getString();
      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(payload);
      if (root.success()) {
        String name = root["name"];
        JsonObject& weather = root["weather"][0];
        String description = weather["main"];
        if (description.startsWith("Thunder")) {
          display.setTextColor(myCYAN);
          description = "Thunder";
        }
        if (description.startsWith("Driz")) display.setTextColor(myYELLOW);
        if (description.startsWith("Rain")) display.setTextColor(myORANGE);
        if (description.startsWith("Snow")) display.setTextColor(myWHITE);
        if (description.startsWith("Mist")) display.setTextColor(myMAGENTA);
        if (description.startsWith("Haze")) display.setTextColor(myGRAY);
        if (description.startsWith("Clear")) display.setTextColor(myBLUE);
        if (description.startsWith("Clouds")) display.setTextColor(myGREEN);
        JsonObject& main = root["main"];
        float temperature = main["temp"];
        int humidity = main["humidity"];
        display.printf("%2dF %2d%%", round(temperature), humidity);
        int16_t  x1, y1, ww;
        uint16_t w, h;
        display.getTextBounds(description, 0, 0, &x1, &y1, &w, &h);
        if (w > 32) w = 32;
        display.setCursor(64 - w, row1);
        display.print(description);
        Serial.printf("%s, %2dF, %2d%%, %s\r\n", name.c_str(), round(temperature), humidity, description.c_str());
      } else {
        display.print("json fail");
        Serial.println(F("getOffset: JSON parse failed!"));
        Serial.println(payload);
      }
    } else {
      display.print(stat);
      Serial.printf("getOffset: GET failed: %d %s\r\n", stat, http.errorToString(stat).c_str());
    }
  }
  http.end();
} // getWeather

