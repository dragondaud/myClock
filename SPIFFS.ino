// SPIFFS config file
// auto format if unable to write

void readSPIFFS() {
  if (SPIFFS.begin()) {
    Serial.println(F("readSPIFFS: mounted"));
    if (SPIFFS.exists("/config.json")) {
      File configFile = SPIFFS.open(F("/config.json"), "r");
      if (!configFile) return;
      size_t size = configFile.size();
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(buf.get());
      if (json.success()) {
        String sp = json["softAPpass"];
        if (sp != "") softAPpass = sp;
        String lo = json["location"];
        if (lo != "") location = lo;
        String tz = json["timezone"];
        if (tz != "") timezone = tz;
        String tk = json["tzKey"];
        if (tk != "") tzKey = tk;
        String ow = json["owKey"];
        if (ow != "") owKey = ow;
        brightness = json["brightness"];
        milTime = json["milTime"];
        myColor = json["myColor"];
        threshold = json["threshold"];
        celsius = json["celsius"];
#ifdef SYSLOG
        String sl = json["syslogSrv"];
        if (sl != "") syslogSrv = sl;
        syslogPort = json["syslogPort"];
#endif
      }
    }
  } else {
    Serial.println(F("readSPIFFS: failed to mount SPIFFS"));
  }
}

void writeSPIFFS() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["softAPpass"] = softAPpass;
  json["location"] = location;
  json["timezone"] = timezone;
  json["tzKey"] = tzKey;
  json["owKey"] = owKey;
  json["brightness"] = brightness;
  json["milTime"] = milTime;
  json["myColor"] = myColor;
  json["threshold"] = threshold;
  json["celsius"] = celsius;
#ifdef SYSLOG
  json["syslogSrv"] = syslogSrv;
  json["syslogPort"] = syslogPort;
#endif
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    display.setCursor(2, row2);
    display.print(F("config failed"));
    Serial.println(F("failed to open config.json for writing"));
    if (SPIFFS.format()) Serial.println(F("SPIFFS formated"));
    delay(5000);
    ESP.restart();
  } else {
#ifdef SYSLOG
    syslog.log(F("save config"));
#endif
    json.prettyPrintTo(Serial);
    Serial.println();
    json.printTo(configFile);
    configFile.close();
    saveConfig = false;
    delay(1000);
  }
}

String getSPIFFS() {
  String payload;
  if (SPIFFS.exists("/config.json")) {
    File configFile = SPIFFS.open("/config.json", "r");
    if (configFile) {
      size_t size = configFile.size();
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(buf.get());
      if (json.success()) json.prettyPrintTo(payload);
      configFile.close();
    }
  }
  return payload;
}
