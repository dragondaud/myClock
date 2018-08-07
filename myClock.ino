/*   myClock -- ESP8266 WiFi NTP Clock for pixel displays
     Copyright (c) 2018 David M Denney <dragondaud@gmail.com>
     distributed under the terms of the MIT License
*/

#include <ESP8266WiFi.h>        //https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoOTA.h>
#include <time.h>
#include <ArduinoJson.h>        // https://github.com/bblanchon/ArduinoJson/
#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager
#include "display.h"
#include "userconfig.h"

// define these in userconfig.h or uncomment here
//#define tzKey "APIKEY"      // from https://timezonedb.com/register
//#define owKey "APIKEY"      // from https://home.openweathermap.org/api_keys

const char* UserAgent = "myClock/1.0 (Arduino ESP8266)";

time_t TWOAM, pNow, wDelay;
int pHH, pMM, pSS;
String timezone, location;

void configModeCallback (WiFiManager *myWiFiManager) {
  display.clearDisplay();
  display.setFont(&Picopixel);
  display.setCursor(2, row1);
  display.print("AP: myClock");
  display.setCursor(2, row2);
  display.print("Pass: ConFigMe");
  display.setCursor(2, row3);
  display.print("IP: 192.168.11.1");
}

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
          Serial.print(F("getOffset: "));
          Serial.println(offset);
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
} // setNTP

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
        if (description.startsWith("Haze")) display.setTextColor(myMAGENTA);
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
        Serial.printf("%s, %2dF, %2d%%, %s", name.c_str(), round(temperature), humidity, description.c_str());
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

void setup() {
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  while (!Serial);
  display.begin(16);
  display_ticker.attach(0.002, display_updater);
  display.clearDisplay();
  display.setCursor(2, row1);
  display.setTextColor(myGREEN);
  display.print("Connecting");

  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setDebugOutput(false);
  wifiManager.setMinimumSignalQuality(10);
  if (!wifiManager.autoConnect("myClock", "ConFigMe")) ESP.reset();

  location = getIPlocation();

  display.clearDisplay();
  display.setFont(&Picopixel);
  display.setTextWrap(false);
  display.setCursor(2, row1);
  display.setTextColor(myGREEN);
  display.print(WiFi.hostname());
  display.setCursor(2, row2);
  display.setTextColor(myBLUE);
  display.print(WiFi.localIP());
  display.setCursor(2, row3);
  display.setTextColor(myMAGENTA);
  display.print(timezone);
  display.setCursor(2, row4);
  display.setTextColor(myCYAN);
  display.print("waiting for ntp");

  setNTP(timezone);

  ArduinoOTA.onStart([]() {
    display.clearDisplay();
    display_ticker.detach();
    Serial.println("\nOTA: Start");
  } );
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA: End");
  } );
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.print("OTA Progress: " + String((progress / (total / 100))) + " \r");
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.print("\nError[" + String(error) + "]: ");
    if (error == OTA_AUTH_ERROR) Serial.println(F("Auth Failed"));
    else if (error == OTA_BEGIN_ERROR) Serial.println(F("Begin Failed"));
    else if (error == OTA_CONNECT_ERROR) Serial.println(F("Connect Failed"));
    else if (error == OTA_RECEIVE_ERROR) Serial.println(F("Receive Failed"));
    else if (error == OTA_END_ERROR) Serial.println(F("End Failed"));
    else Serial.println(F("unknown error"));
  });
  ArduinoOTA.begin();

  Serial.println();
  Serial.print(F("Last reset reason: "));
  Serial.println(ESP.getResetReason());
  Serial.print(F("WiFi Hostname: "));
  Serial.println(WiFi.hostname());
  Serial.print(F("WiFi IP addr: "));
  Serial.println(WiFi.localIP());
  Serial.print(F("WiFi gw addr: "));
  Serial.println(WiFi.gatewayIP());
  Serial.print(F("WiFi MAC addr: "));
  Serial.println(WiFi.macAddress());
  Serial.print(F("ESP Sketch size: "));
  Serial.println(ESP.getSketchSize());
  Serial.print(F("ESP Flash free: "));
  Serial.println(ESP.getFreeSketchSpace());
  Serial.print(F("ESP Flash Size: "));
  Serial.println(ESP.getFlashChipRealSize());

  delay(1000);
  display.clearDisplay();
  time_t now = time(nullptr);
  int ss = now % 60;
  int mm = (now / 60) % 60;
  int hh = (now / (60 * 60)) % 24;
  digit1.DrawColon(myColor);
  digit3.DrawColon(myColor);
  digit0.Draw(ss % 10);
  digit1.Draw(ss / 10);
  digit2.Draw(mm % 10);
  digit3.Draw(mm / 10);
  digit4.Draw(hh % 10);
  digit5.Draw(hh / 10);
  pNow = now;
  getWeather();
} // setup

void loop() {
  ArduinoOTA.handle();
  time_t now = time(nullptr);
  if (now != pNow) {
    if (now > TWOAM) setNTP(timezone);
    int ss = now % 60;
    int mm = (now / 60) % 60;
    int hh = (now / (60 * 60)) % 24;
    if (ss != pSS) {
      int s0 = ss % 10;
      int s1 = ss / 10;
      if (s0 != digit0.Value()) digit0.Morph(s0);
      if (s1 != digit1.Value()) digit1.Morph(s1);
      pSS = ss;
    }

    if (mm != pMM) {
      int m0 = mm % 10;
      int m1 = mm / 10;
      if (m0 != digit2.Value()) digit2.Morph(m0);
      if (m1 != digit3.Value()) digit3.Morph(m1);
      pMM = mm;
    }

    if (hh != pHH) {
      int h0 = hh % 10;
      int h1 = hh / 10;
      if (h0 != digit4.Value()) digit4.Morph(h0);
      if (h1 != digit5.Value()) digit5.Morph(h1);
      pHH = hh;
    }
    pNow = now;
    if (now > wDelay) getWeather();
  }
}

