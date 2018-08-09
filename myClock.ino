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
#include <WiFiManager.h>        // https://github.com/tzapu/WiFiManager

#include "display.h"
#include "userconfig.h"

// define these in userconfig.h or uncomment here
//#define tzKey "APIKEY"      // from https://timezonedb.com/register
//#define owKey "APIKEY"      // from https://home.openweathermap.org/api_keys

const char* UserAgent = "myClock/1.0 (Arduino ESP8266)";

time_t TWOAM, pNow, wDelay;
int pHH, pMM, pSS;
String timezone, location;

void setup() {
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  while (!Serial);
  Serial.println();
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

