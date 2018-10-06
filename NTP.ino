//  NTP and location functions

String getIPlocation() { // Using ip-api.com to discover public IP's location and timezone
  HTTPClient http;
  static const char URL[] PROGMEM = "http://ip-api.com/json";
  String payload;
  http.setUserAgent(UserAgent);
  if (!http.begin(URL)) {
#ifdef SYSLOG
    syslog.log(F("getIPlocation HTTP failed"));
#endif
    Serial.println(F("getIPlocation: HTTP failed"));
  } else {
    int stat = http.GET();
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
#ifdef SYSLOG
        syslog.logf("getIPlocation: %s, %s, %s, %s",
                    isp.c_str(), region.c_str(), country.c_str(), tz.c_str());
#endif
        Serial.println(PSTR("getIPlocation: ") + isp + ", " + region + ", " + country + ", " + tz);
        return zip;
      } else {
#ifdef SYSLOG
        syslog.log(F("getIPlocation JSON parse failed"));
        syslog.log(payload);
#endif
        Serial.println(F("getIPlocation: JSON parse failed!"));
        Serial.println(payload);
      }
    } else {
#ifdef SYSLOG
      syslog.logf("getIPlocation failed, GET reply %d", stat);
#endif
      Serial.printf_P(PSTR("getIPlocation: GET reply %d\r\n"), stat);
    }
  }
  http.end();
  delay(5000);
} // getIPlocation

int getOffset(const String tz) { // using timezonedb.com, return offset for zone name
  HTTPClient http;
  String URL = PSTR("http://api.timezonedb.com/v2/list-time-zone?key=")
               + tzKey + F("&format=json&zone=") + tz;
  String payload;
  int stat;
  http.setUserAgent(UserAgent);
  if (!http.begin(URL)) {
#ifdef SYSLOG
    syslog.log(F("getOffset HTTP failed"));
#endif
    Serial.println(F("getOffset: HTTP failed"));
  } else {
    stat = http.GET();
    if (stat == HTTP_CODE_OK) {
      payload = http.getString();
      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(payload);
      if (root.success()) {
        JsonObject& zones = root["zones"][0];
        offset = zones["gmtOffset"];
      } else {
#ifdef SYSLOG
        syslog.log(F("getOffset JSON parse failed"));
        syslog.log(payload);
#endif
        Serial.println(F("getOffset: JSON parse failed!"));
        Serial.println(payload);
      }
    } else {
#ifdef SYSLOG
      syslog.logf("getOffset failed, GET reply %d", stat);
#endif
      Serial.printf_P(PSTR("getOffset: GET reply %d\r\n"), stat);
    }
  }
  http.end();
  return stat;
} // getOffset

void setNTP(const String tz) {
  while (getOffset(tz) != HTTP_CODE_OK) {
    delay(1000);
  }
  Serial.print(F("setNTP: configure NTP ..."));
  configTime(offset, 0, "pool.ntp.org", "time.nist.gov");
  while (time(nullptr) < (30 * 365 * 24 * 60 * 60)) {
    delay(1000);
    Serial.print(F("."));
  }
  Serial.println(F(" OK"));
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
  Serial.print(F("setNTP: next timezone check @ "));
  Serial.println(t);
#ifdef SYSLOG
  syslog.logf("setNTP: %s (%d)", timezone.c_str(), int(offset / 3600));
#endif
} // setNTP
