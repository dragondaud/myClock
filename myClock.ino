/*   myClock -- ESP8266 WiFi NTP Clock for pixel displays
     Copyright (c) 2018 David M Denney <dragondaud@gmail.com>
     distributed under the terms of the MIT License
*/

#include <ESP8266WiFi.h>        //https://github.com/esp8266/Arduino
#include <ESP8266HTTPClient.h>
#include <ArduinoOTA.h>
#include <time.h>
#include <FS.h>
#include <pgmspace.h>
#include <ArduinoJson.h>        // https://github.com/bblanchon/ArduinoJson/
#include <WiFiManager.h>        // https://github.com/tzapu/WiFiManager
#include "display.h"
#include "user_interface.h"

#define APPNAME "myClock"
#define VERSION "0.9.20"
//#define DS18                      // enable DS18B20 temperature sensor
//#define SYSLOG                    // enable SYSLOG support

String tzKey;                     // API key from https://timezonedb.com/register
String owKey;                     // API key from https://home.openweathermap.org/api_keys
String softAPpass = "ConFigMe";   // password for SoftAP config
uint8_t brightness = 255;         // 0-255 display brightness
bool milTime = true;              // set false for 12hour clock
String location;                  // zipcode or empty for geoIP location
String timezone;                  // timezone from https://timezonedb.com/time-zones or empty for geoIP
int threshold = 500;              // below this value display will dim, incrementally
bool celsius = false;             // set true to display temp in celsius
String language = "en";           // font does not support all languages
String countryCode = "US";        // default US, automatically set based on public IP address

// Syslog
#ifdef SYSLOG
#include <Syslog.h>             // https://github.com/arcao/Syslog
WiFiUDP udpClient;
Syslog syslog(udpClient, SYSLOG_PROTO_IETF);
String syslogSrv = "syslog";
uint16_t syslogPort = 514;
#endif

// DS18B20
#ifdef DS18
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS D3
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
int Temp;
#endif

ESP8266WebServer server(80);

static const char* UserAgent PROGMEM = "myClock/1.0 (Arduino ESP8266)";

time_t TWOAM, pNow, wDelay;
uint8_t pHH, pMM, pSS;
uint16_t light;
long offset;
char HOST[20];
bool saveConfig = false;
uint8_t dim;
bool LIGHT = true;

void setup() {
  system_update_cpu_freq(SYS_CPU_160MHZ);               // force 160Mhz to prevent display flicker
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY, 1);  // allow use of RX pin for gpio
  while (!Serial);
  delay(10);
  Serial.println();
  readSPIFFS();

  display.begin(16);
  display_ticker.attach(0.002, display_updater);
  display.clearDisplay();
  display.setFont(&TomThumb);
  display.setTextWrap(false);
  display.setTextColor(myColor);
  display.setBrightness(brightness);

  drawImage(0, 0); // display splash image while connecting

#ifdef DS18
  sensors.begin();
#endif

  startWiFi();
  if (saveConfig) writeSPIFFS();

#ifdef SYSLOG
  syslog.server(syslogSrv.c_str(), syslogPort);
  syslog.deviceHostname(HOST);
  syslog.appName(APPNAME);
  syslog.defaultPriority(LOG_INFO);
#endif

  if (location == "") location = getIPlocation();
  else while (timezone == "") getIPlocation();

  display.clearDisplay();
  display.setCursor(2, row1);
  display.setTextColor(myGREEN);
  display.print(HOST);
  display.setCursor(2, row2);
  display.setTextColor(myBLUE);
  display.print(WiFi.localIP());
  display.setCursor(2, row3);
  display.setTextColor(myMAGENTA);
  display.print(timezone);
  display.setCursor(2, row4);
  display.setTextColor(myLTBLUE);
  display.print(F("V"));
  display.print(VERSION);
  display.setCursor(32, row4);
  display.setTextColor(myBLUE);
  display.print(F("set ntp"));
  light = analogRead(A0);
  Serial.printf_P(PSTR("setup: %s, %s, %s, %d, %d \r\n"),
                  location.c_str(), timezone.c_str(), milTime ? "true" : "false", brightness, light);
#ifdef SYSLOG
  syslog.logf("setup: %s|%s|%s|%d|%d",
              location.c_str(), timezone.c_str(), milTime ? "true" : "false", brightness, light);
#endif
  setNTP(timezone);
  delay(1000);
  startWebServer();
  displayDraw(brightness);
  getWeather();
} // setup

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
  time_t now = time(nullptr);
  if (now != pNow) {
    if (now > TWOAM) setNTP(timezone);
    int ss = now % 60;
    int mm = (now / 60) % 60;
    int hh = (now / (60 * 60)) % 24;
    if ((!milTime) && (hh > 12)) hh -= 12;
    if (ss != pSS) {
      int s0 = ss % 10;
      int s1 = ss / 10;
      if (s0 != digit0.Value()) digit0.Morph(s0);
      if (s1 != digit1.Value()) digit1.Morph(s1);
      pSS = ss;
      getLight();
    }
    if (mm != pMM) {
      int m0 = mm % 10;
      int m1 = mm / 10;
      if (m0 != digit2.Value()) digit2.Morph(m0);
      if (m1 != digit3.Value()) digit3.Morph(m1);
      pMM = mm;
      Serial.printf_P(PSTR("%02d:%02d %3d %3d \r"), hh, mm, light, dim);
    }
    if (hh != pHH) {
      int h0 = hh % 10;
      int h1 = hh / 10;
      if (h0 != digit4.Value()) digit4.Morph(h0);
      if (h1 != digit5.Value()) digit5.Morph(h1);
      pHH = hh;
    }
#ifdef DS18
    sensors.requestTemperatures();
    int t;
    if (celsius) t = round(sensors.getTempC(0));
    else t = round(sensors.getTempF(0));
    if (t < -66 | t > 150) t = 0;
    if (Temp != t) {
      Temp = t;
      display.setCursor(0, row1);
      display.printf_P(PSTR("% 2d"), Temp);
    }
#endif
    pNow = now;
    if (now > wDelay) getWeather();
  }
}

void displayDraw(uint8_t b) {
  display.clearDisplay();
  display.setBrightness(b);
  dim = b;
  time_t now = time(nullptr);
  int ss = now % 60;
  int mm = (now / 60) % 60;
  int hh = (now / (60 * 60)) % 24;
  if ((!milTime) && (hh > 12)) hh -= 12;
  Serial.printf_P(PSTR("%02d:%02d\r"), hh, mm);
  digit1.DrawColon(myColor);
  digit3.DrawColon(myColor);
  digit0.Draw(ss % 10, myColor);
  digit1.Draw(ss / 10, myColor);
  digit2.Draw(mm % 10, myColor);
  digit3.Draw(mm / 10, myColor);
  digit4.Draw(hh % 10, myColor);
  digit5.Draw(hh / 10, myColor);
  pNow = now;
}

void getLight() {
  if (!LIGHT) return;
  int lt = analogRead(A0);
  if (lt > 20) {
    light = (light * 3 + lt) >> 2;
    if (light >= threshold) dim = brightness;
    else if (light < (threshold >> 3)) dim = brightness >> 4;
    else if (light < (threshold >> 2)) dim = brightness >> 3;
    else if (light < (threshold >> 1)) dim = brightness >> 2;
    else if (light < threshold) dim = brightness >> 1;
    display.setBrightness(dim);
  }
}
