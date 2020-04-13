/*   myClock -- ESP8266 WiFi NTP Clock for pixel displays
     Copyright (c) 2019 David M Denney <dragondaud@gmail.com>
     distributed under the terms of the MIT License
*/

//  ESP8266 requires https://github.com/esp8266/Arduino
//  https://github.com/esp8266/Arduino#using-git-version-basic-instructions

//  ESP32 requires https://github.com/espressif/arduino-esp32
//  https://github.com/espressif/arduino-esp32#installation-instructions

#include <ArduinoJson.h>        // https://github.com/bblanchon/ArduinoJson/releases/latest
#include <WiFiManager.h>        // https://github.com/tzapu/WiFiManager/tree/development
#include <ArduinoOTA.h>
#include <FS.h>
#include <time.h>
#include "display.h"

#if defined(ESP8266)
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
ESP8266WebServer server(80);
#else
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include <HTTPClient.h>
WebServer server(80);
#endif

#define APPNAME "myClock"
#define VERSION "0.10.5"
#define ADMIN_USER "admin"    // WebServer logon username
//#define DS18                // enable DS18B20 temperature sensor
//#define SYSLOG              // enable SYSLOG support
#define LIGHT               // enable LDR light sensor
#define WDELAY 900          // delay 15 min between weather updates
#define myOUT 1             // {0 = NullStream, 1 = Serial, 2 = Bluetooth}

#if myOUT == 0                    // NullStream output
NullStream NullStream;
Stream & OUT = NullStream;
#elif myOUT == 2                  // Bluetooth output, only on ESP32
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;
Stream & OUT = SerialBT;
#else                             // Serial output default
Stream & OUT = Serial;
#endif

String tzKey;                     // API key from https://timezonedb.com/register
String owKey;                     // API key from https://home.openweathermap.org/api_keys
String softAPpass = "ConFigMe";   // password for SoftAP config and WebServer logon, minimum 8 characters
uint8_t brightness = 255;         // 0-255 display brightness
bool milTime = true;              // set false for 12hour clock
String location;                  // zipcode or empty for geoIP location
String timezone;                  // timezone from https://timezonedb.com/time-zones or empty for geoIP
int threshold = 500;              // below this value display will dim, incrementally
bool celsius = false;             // set true to display temp in celsius
String language = "en";           // font does not support all languages
String countryCode = "US";        // default US, automatically set based on public IP address

// Syslog server wireless debugging and monitoring
#ifdef SYSLOG
#include <Syslog.h>             // https://github.com/arcao/Syslog
WiFiUDP udpClient;
Syslog syslog(udpClient, SYSLOG_PROTO_IETF);
String syslogSrv = "syslog";
uint16_t syslogPort = 514;
#endif

// DS18B20 temperature sensor for local/indoor temp display
#ifdef DS18
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS D3
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
int Temp;
#endif

static const char* UserAgent PROGMEM = "myClock/1.0 (Arduino ESP8266)";

time_t TWOAM, pNow, wDelay;
uint8_t pHH, pMM, pSS;
long offset;
char HOST[20];
uint8_t dim;

#ifdef LIGHT
uint16_t light = threshold;
#endif

void setup() {
#if myOUT == 0
  Serial.end(); // close serial if not used
#else
  Serial.begin(115200);
  while (!Serial) delay(10);  // wait for Serial to start
  Serial.println();
#endif

#if myOUT == 2
  if (!SerialBT.begin(APPNAME)) {
    Serial.println(F("Bluetooth failed"));
    delay(5000);
    ESP.restart();
  }
  delay(5000); // wait for client to connect
#endif

  readSPIFFS(); // fetch stored configuration

  display.begin(16);
  display_ticker.attach(0.002, display_updater);
  display.clearDisplay();
  display.setFont(&TomThumb);
  display.setTextWrap(false);
  display.setTextColor(myColor);
  display.setBrightness(brightness);

  drawImage(0, 0); // display splash image while connecting and setting time

#ifdef DS18
  sensors.begin();  // start temp sensor
#endif

  startWiFi();

#ifdef SYSLOG
  syslog.server(syslogSrv.c_str(), syslogPort); // configure syslog server
  syslog.deviceHostname(HOST);
  syslog.appName(APPNAME);
  syslog.defaultPriority(LOG_INFO);
#endif

  if (location == "") location = getIPlocation(); // get postal code and/or timezone as needed
  else while (timezone == "") getIPlocation();

  display.clearDisplay();       // display hostname, ip address, timezone, version
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
  OUT.printf_P(PSTR("setup: %s, %s, %s, %s \r\n"),
    location.c_str(), countryCode.c_str(), timezone.c_str(), milTime ? "true" : "false");
#ifdef SYSLOG
  syslog.logf("setup: %s|%s|%s", location.c_str(), timezone.c_str(), milTime ? "true" : "false");
#endif
  setNTP(timezone); // configure NTP from timezone name for location
  delay(1000);
  startWebServer();
  displayDraw(brightness);  // initial time display before fetching weather and starting loop
  getWeather();
} // setup

void loop() {
#if defined(ESP8266)
  MDNS.update();    // ESP32 does this automatically
#endif
  ArduinoOTA.handle();    // check for OTA updates
  server.handleClient();  // handle WebServer requests
  struct tm * timeinfo;
  time_t now = time(nullptr); // get current time
  timeinfo = localtime(&now);
  if (now != pNow) { // skip ahead if still same time
    if (now > TWOAM) setNTP(timezone);  // recheck timezone every day at 2am
    int ss = timeinfo->tm_sec;
    int mm = timeinfo->tm_min;
    int hh = timeinfo->tm_hour;
    if ((!milTime) && (hh > 12)) hh -= 12;
    if (ss != pSS) {    // only update seconds if changed
      int s0 = ss % 10;
      int s1 = ss / 10;
      if (s0 != digit0.Value()) digit0.Morph(s0);
      if (s1 != digit1.Value()) digit1.Morph(s1);
      pSS = ss;
#ifdef LIGHT
      getLight();
#endif
    }
    if (mm != pMM) {    // update minutes, if changed
      int m0 = mm % 10;
      int m1 = mm / 10;
      if (m0 != digit2.Value()) digit2.Morph(m0);
      if (m1 != digit3.Value()) digit3.Morph(m1);
      pMM = mm;
      OUT.printf_P(PSTR("%02d:%02d %d "), hh, mm, ESP.getFreeHeap()); // output debug once per minute
#ifdef LIGHT
      OUT.print(light);
#endif
      OUT.print("\r");
    }
    if (hh != pHH) {    // update hours, if changed
      int h0 = hh % 10;
      int h1 = hh / 10;
      if (h0 != digit4.Value()) digit4.Morph(h0);
      if (h1 != digit5.Value()) digit5.Morph(h1);
      pHH = hh;
    }
#ifdef DS18
    sensors.requestTemperatures();
    int t;
    if (celsius) t = (int)round(sensors.getTempC(0));
    else t = (int)round(sensors.getTempF(0));
    if (t < -66 | t > 150) t = 0;
    if (Temp != t) {
      Temp = t;
      display.setCursor(0, row1);
      display.printf_P(PSTR("% 2d"), Temp);
    }
#endif
    pNow = now;
    if (now > wDelay) getWeather(); // fetch weather again if enough time has passed
  }
} // loop

void displayDraw(uint8_t b) { // clear display and draw all digits of current time
  display.clearDisplay();
  display.setBrightness(b);
  dim = b;
  time_t now = time(nullptr);
  int ss = now % 60;
  int mm = (now / 60) % 60;
  int hh = (now / (60 * 60)) % 24;
  if ((!milTime) && (hh > 12)) hh -= 12;
  OUT.printf_P(PSTR("%02d:%02d\r"), hh, mm);
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

#ifdef LIGHT
void getLight() {                   // if LDR present, auto-dim display below threshold
  int lt = analogRead(A0);
  if (lt > 20) {                    // ignore LDR if value invalid
    light = (light * 3 + lt) >> 2;  // average sensor into light slowly
    if (light >= threshold) dim = brightness;                 // light above threshold, full brightness
    else if (light < (threshold >> 3)) dim = brightness >> 4; // light below 1/8 threshold, brightness 1/16
    else if (light < (threshold >> 2)) dim = brightness >> 3; // light below 1/4 threshold, brightness 1/8
    else if (light < (threshold >> 1)) dim = brightness >> 2; // light below 1/2 threshold, brightness 1/4
    else if (light < threshold) dim = brightness >> 1;        // light below threshold, brightness 1/2
    display.setBrightness(dim);
  }
}
#endif
