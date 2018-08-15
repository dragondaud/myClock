String UrlEncode(const String url) {
  String e;
  for (int i = 0; i < url.length(); i++) {
    char c = url.charAt(i);
    if (c == 0x20) {
      e += "%20";
    } else if (isalnum(c)) {
      e += c;
    } else {
      e += "%";
      if (c < 0x10) e += "0";
      e += String(c, HEX);
    }
  }
  return e;
}

String getIPlocation() { // Using ip-api.com to discover public IP's location and timezone
  HTTPClient http;
  String URL = "http://ip-api.com/json";
  String payload;
  http.setUserAgent(UserAgent);
  Serial.printf("[%d] ", ESP.getFreeHeap());
  if (!http.begin(URL)) {
    Serial.println(F("getIPlocation: HTTP failed"));
  } else {
    int stat = http.GET();
    if (stat > 0) {
      if (stat == HTTP_CODE_OK) {
        payload = http.getString();
        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(payload);
        if (root.success()) {
          String isp = root["isp"];
          String region = root["regionName"];
          String country = root["countryCode"];
          String tz = root["timezone"];
          String zip = root["zip"];
          timezone = tz;
          http.end();
          Serial.println("getIPlocation: " + isp + ", " + region + ", " + country + ", " + tz);
          return zip;
        } else {
          Serial.println(F("getIPlocation: JSON parse failed!"));
          Serial.println(payload);
        }
      } else {
        Serial.printf("getIPlocation: GET reply %d\r\n", stat);
      }
    } else {
      Serial.printf("getIPlocation: GET failed: %s\r\n", http.errorToString(stat).c_str());
    }
  }
  http.end();
} // getIPlocation

long getOffset(const String tz) { // using timezonedb.com, return offset for zone name
  HTTPClient http;
  String URL = "http://api.timezonedb.com/v2/list-time-zone?key=" + String(tzKey)
               + "&format=json&zone=" + UrlEncode(tz);
  String payload;
  long offset;
  http.setUserAgent(UserAgent);
  Serial.printf("[%d] ", ESP.getFreeHeap());
  if (!http.begin(URL)) {
    Serial.println(F("getOffset: HTTP failed"));
  } else {
    int stat = http.GET();
    if (stat > 0) {
      if (stat == HTTP_CODE_OK) {
        payload = http.getString();
        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(payload);
        if (root.success()) {
          JsonObject& zones = root["zones"][0];
          offset = zones["gmtOffset"];
          Serial.printf("getOffset: %d (%d)\r\n", int(offset / 3600), offset);
        } else {
          Serial.println(F("getOffset: JSON parse failed!"));
          Serial.println(payload);
        }
      } else {
        Serial.printf("getOffset: GET reply %d\r\n", stat);
      }
    } else {
      Serial.printf("getOffset: GET failed: %s\r\n", http.errorToString(stat).c_str());
    }
  }
  http.end();
  return offset;
} // getOffset

void setNTP(const String tz) {
  long offset = getOffset(tz);
  Serial.print(F("setNTP: configure NTP ..."));
  configTime(offset, 0, "pool.ntp.org", "time.nist.gov");
  while (time(nullptr) < (30 * 365 * 24 * 60 * 60)) {
    delay(1000);
    Serial.print(F("."));
  }
  delay(5000);
  Serial.println(" OK");
  time_t now = time(nullptr);
  struct tm * calendar;
  calendar = localtime(&now);
  calendar->tm_mday++;
  calendar->tm_hour = 2;
  calendar->tm_min = 0;
  calendar->tm_sec = 0;
  TWOAM = mktime(calendar);
  String t = ctime(&TWOAM);
  t.trim();
  Serial.print("setNTP: next timezone check @ ");
  Serial.println(t);
  syslog.logf(LOG_INFO, "%s, %s (%d), %s", location.c_str(), timezone.c_str(), int(offset / 3600), t.c_str());
} // setNTP

