// WiFi and WiFiManager

void configModeCallback(WiFiManager *myWiFiManager) {
  display.clearDisplay();
  display.setFont(&TomThumb);
  display.setCursor(2, row1);
  display.print(F("Config WiFi"));
  display.setCursor(2, row2);
  display.print(myWiFiManager->getConfigPortalSSID());
  display.setCursor(2, row3);
  display.print(F("Pass: "));
  display.print(softAPpass);
  display.setCursor(2, row4);
  display.print(F("IP: "));
  display.print(WiFi.softAPIP());
}

#if defined(ESP32)
#include "esp_system.h"
String getMacAddress() {
  uint8_t baseMac[6];
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  char baseMacChr[18] = {0};
  sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
  return String(baseMacChr);
}
#endif

void startWiFi() {   // if WiFi does not connect, establish AP for configuration
#if defined(ESP8266)
  String t = WiFi.macAddress();
  t = String(APPNAME) + "-" + t.substring(9, 11) + t.substring(12, 14) + t.substring(15, 17);
  t.toCharArray(HOST, sizeof(HOST));
  WiFi.hostname(HOST);
#else
  String t = getMacAddress();
  t = String(APPNAME) + "-" + t.substring(9, 11) + t.substring(12, 14) + t.substring(15, 17);
  t.toCharArray(HOST, sizeof(HOST));
  WiFi.setHostname(HOST);
#endif
  WiFiManager wifiManager(OUT);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setDebugOutput(false);            // set true for wifi debugging
  wifiManager.setConfigPortalTimeout(300);      // 5 minute timeout for config portal
  wifiManager.setClass("invert");               // dark theme
  if (!wifiManager.autoConnect(HOST, softAPpass.c_str())) {
    OUT.println(F("\nWiFi: failed"));
    delay(5000);
    ESP.restart();
  }
  WiFi.mode(WIFI_STA);
  OUT.print(F("WiFi: "));
  OUT.print(HOST);
  OUT.print(F(" "));
  OUT.println(WiFi.localIP());
  ArduinoOTA.setHostname(HOST);
  ArduinoOTA.onStart([]() {
#ifdef SYSLOG
    syslog.log(F("OTA Update"));
#endif
    display.clearDisplay();   // turn off display during update
    display_ticker.detach();
    OUT.println(F("\nOTA: Start"));
  } );
  ArduinoOTA.onEnd([]() {
    OUT.println(F("\nOTA: End"));
    delay(1000);
    ESP.restart();
  } );
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    int p = round(progress / (total/100));
    if (dim != p) {
      OUT.print(PSTR("OTA Progress: ") + String(p) + PSTR(" \r"));
      dim = p;
    }
  });
  ArduinoOTA.onError([](ota_error_t error) {
    OUT.print(PSTR("\nError[") + String(error) + PSTR("]: "));
    if (error == OTA_AUTH_ERROR) OUT.println(F("Auth Failed"));
    else if (error == OTA_BEGIN_ERROR) OUT.println(F("Begin Failed"));
    else if (error == OTA_CONNECT_ERROR) OUT.println(F("Connect Failed"));
    else if (error == OTA_RECEIVE_ERROR) OUT.println(F("Receive Failed"));
    else if (error == OTA_END_ERROR) OUT.println(F("End Failed"));
    else OUT.println(F("unknown error"));
    delay(1000);
    ESP.restart();
  });
  ArduinoOTA.begin();
} // startWiFi
