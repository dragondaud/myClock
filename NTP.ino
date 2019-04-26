//  NTP and location functions

String getIPlocation() { // Using ip-api.com to discover public IP's location and timezone
  WiFiClient wifi;
  HTTPClient http;
  static const char URL[] PROGMEM = "http://ip-api.com/json";
  String payload;
  http.setUserAgent(UserAgent);
  if (!http.begin(wifi, URL)) {
#ifdef SYSLOG
    syslog.log(F("getIPlocation HTTP failed"));
#endif
    OUT.println(F("getIPlocation: HTTP failed"));
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
        countryCode = country;
        http.end();
#ifdef SYSLOG
        syslog.logf("getIPlocation: %s, %s, %s, %s",
                    isp.c_str(), region.c_str(), country.c_str(), tz.c_str());
#endif
        OUT.println(PSTR("getIPlocation: ") + isp + PSTR(", ")
                    + region + PSTR(", ") + country + PSTR(", ") + tz);
        return zip;
      } else {
#ifdef SYSLOG
        syslog.log(F("getIPlocation JSON parse failed"));
        syslog.log(payload);
#endif
        OUT.println(F("getIPlocation: JSON parse failed!"));
        OUT.println(payload);
      }
    } else {
#ifdef SYSLOG
      syslog.logf("getIPlocation failed, GET reply %d", stat);
#endif
      OUT.printf_P(PSTR("getIPlocation: GET reply %d\r\n"), stat);
    }
  }
  http.end();
  delay(1000);
} // getIPlocation

int getOffset(const String tz) { // using timezonedb.com, return offset for zone name
  WiFiClient wifi;
  HTTPClient http;
  String URL = PSTR("http://api.timezonedb.com/v2/list-time-zone?key=")
               + tzKey + PSTR("&format=json&zone=") + tz;
  String payload;
  int stat;
  http.setUserAgent(UserAgent);
  if (!http.begin(wifi, URL)) {
#ifdef SYSLOG
    syslog.log(F("getOffset HTTP failed"));
#endif
    OUT.println(F("getOffset: HTTP failed"));
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
        OUT.println(F("getOffset: JSON parse failed!"));
        OUT.println(payload);
      }
    } else {
#ifdef SYSLOG
      syslog.logf("getOffset failed, GET reply %d", stat);
#endif
      OUT.printf_P(PSTR("getOffset: GET reply %d\r\n"), stat);
    }
  }
  http.end();
  return stat;
} // getOffset

void setNTP(const String tz) {
  while (getOffset(tz) != HTTP_CODE_OK) { // wait for valid timezone offset
    delay(1000);
  }
  OUT.print(F("setNTP: configure NTP ..."));
  configTime(offset, 0, PSTR("0.pool.ntp.org"), PSTR("1.pool.ntp.org"));
  while (time(nullptr) < (30 * 365 * 24 * 60 * 60)) { // wait for NTP sync
    delay(1000);
    OUT.print(F("."));
  }
  OUT.println(F(" OK"));
  time_t now = time(nullptr);
  struct tm * calendar;
  calendar = localtime(&now);
  calendar->tm_mday++;    // calculate tomorrow
  calendar->tm_hour = 2;  // at 2am
  calendar->tm_min = 0;
  calendar->tm_sec = 0;
  TWOAM = mktime(calendar);
  String t = ctime(&TWOAM);
  t.trim();
  OUT.print(F("setNTP: next timezone check @ "));
  OUT.println(t);
#ifdef SYSLOG
  syslog.logf("setNTP: %s (%d)", timezone.c_str(), int(offset / 3600));
#endif
} // setNTP
