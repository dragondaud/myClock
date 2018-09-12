void readSPIFFS() {
  if (SPIFFS.begin()) {
    Serial.println(F("readSPIFFS: mounted SPIFFS"));
    if (SPIFFS.exists("/config.json")) {
      File configFile = SPIFFS.open("/config.json", "r");
      if (!configFile) return;
      Serial.println(F("opened config file"));
      size_t size = configFile.size();
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(buf.get());
      if (json.success()) {
        String sp = json["softAPpass"];
        softAPpass = sp;
        String tz = json["timezone"];
        timezone = tz;
        String tk = json["tzKey"];
        tzKey = tk;
        String ow = json["owKey"];
        owKey = ow;
        brightness = json["brightness"];
#ifdef SYSLOG
        String sl = json["syslogSrv"];
        syslogSrv = sl;
        syslogPort = json["syslogPort"];
#endif
      }
    }
  } else {
    Serial.println(F("readSPIFFS: failed to mount SPIFFS"));
  }
}

void writeSPIFFS() {
  display.clearDisplay();
  display.setFont(&Picopixel);
  display.setCursor(2, row1);
  display.print(F("save config"));
  Serial.println(F("WiFiManager: save config"));
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["softAPpass"] = softAPpass;
  json["timezone"] = timezone;
  json["tzKey"] = tzKey;
  json["owKey"] = owKey;
  json["brightness"] = brightness;
#ifdef SYSLOG
  json["syslogSrv"] = syslogSrv;
  json["syslogPort"] = syslogPort;
#endif
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    display.setCursor(2, row2);
    display.print(F("config failed"));
    Serial.println(F("failed to open config.json for writing"));
    delay(5000);
    SPIFFS.format();
    delay(5000);
    ESP.restart();
  } else {
#ifdef SYSLOG
    syslog.log(LOG_INFO, F("save config"));
#endif
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    saveConfig = false;
  }
}
