void configModeCallback (WiFiManager *myWiFiManager) {
  display.clearDisplay();
  display.setFont(&Picopixel);
  display.setCursor(2, row1);
  display.print(F("Config WiFi"));
  display.setCursor(2, row2);
  display.print(myWiFiManager->getConfigPortalSSID());
  display.setCursor(2, row3);
  display.print(F("Pass: "));
  display.print(SOFTAP_PASS);
  display.setCursor(2, row4);
  display.print(F("IP: "));
  display.print(WiFi.softAPIP());
}

void startWiFi() {    // if WiFi does not connect, establish AP for configuration
  String t = WiFi.macAddress();
  t = String(APPNAME) + "-" + t.substring(9, 11) + t.substring(12, 14) + t.substring(15, 17);
  t.toCharArray(HOST, 20);
  WiFi.hostname(HOST);
  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setDebugOutput(false);
  wifiManager.setMinimumSignalQuality(10);
  if (!wifiManager.autoConnect(HOST, SOFTAP_PASS)) ESP.reset();
  MDNS.begin(HOST);
} // startWiFi

