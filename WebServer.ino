// Web Server for configuration and updates

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

static const char* serverRoot PROGMEM =
  "<!DOCTYPE HTML><html><head><title>myClock</title><style>"
  "body {background-color: black; color: White;}"
  "</style></head><body><h1>myClock " VERSION "</h1>"
  "<h2>Update Firmware</h2><form method='POST' action='/update' enctype='multipart/form-data'>"
  "<input type='file' name='update'><input type='submit' value='Update'></form>"
  "<p><form method='GET' action='/reset'><input type='submit' value='REBOOT CLOCK'></form>&nbsp;"
  "<form method='GET' action='/format'><input type='submit' value='ERASE CONFIG'></form>";

static const char* serverConfig PROGMEM =
  "<p><form method='post' action='/save' id='configForm' name='configForm'>"
  "<input type='submit' value='SAVE'> <input type='reset'>"
  "<p><textarea id='json' rows='13' cols='50' name='json' form='configForm'>";

static const char* serverTail PROGMEM = "</body></html>";

static const char* serverReboot PROGMEM =
  "<!DOCTYPE HTML><html><head>"
  "<meta http-equiv=\"refresh\" content=\"10;url=/\" />"
  "<style>body {background-color: black; color: White;}"
  "</style></head><body><h1>myClock " VERSION "</h1>"
  "Rebooting...</body></html>";

void handleNotFound() {
  syslog.log(LOG_INFO, F("webServer: Not Found"));
  server.sendHeader(F("Location"), F("/"));
  server.send(301);
}

void handleError() {
  String message = "FAIL\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(500, "text/plain", message);
}

void handleSave() {
  if (!server.hasArg("json")) return handleError();
  syslog.log(LOG_INFO, F("webServer: save"));
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(server.arg("json"));
  if (json.success()) {
    json.prettyPrintTo(Serial);
    Serial.println();
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
    uint16_t c = json["myColor"];
    if (c > 0) myColor = c;
#ifdef SYSLOG
    String sl = json["syslogSrv"];
    if (sl != "") syslogSrv = sl;
    syslogPort = json["syslogPort"];
#endif
  }
  writeSPIFFS();
  String payload = String(serverReboot) + String(serverTail);
  server.send(200, F("text/html"), payload);
  server.close();
  delay(1000);
  ESP.restart();
}

void startWebServer() {
  server.on(F("/"), HTTP_GET, []() {
    syslog.log(LOG_INFO, F("webServer: index"));
    server.sendHeader(F("Connection"), F("close"));
    time_t now = time(nullptr);
    String t = ctime(&now);
    String payload = String(serverRoot) + F("<h3>") + t + F("</h3><p>Free Heap: ") + String(ESP.getFreeHeap());
    if (LIGHT) payload = payload + F("<p>Light Level: ") + String(light);
    payload += String(serverConfig) + getSPIFFS() + F("</textarea></form>");
    payload += String(serverTail);
    server.send(200, F("text/html"), payload);
  });
  server.on(F("/save"), handleSave);
  server.on(F("/reset"), HTTP_GET, []() {
    syslog.log(LOG_INFO, F("webServer: reset"));
    Serial.println(F("webServer: reset"));
    String payload = String(serverReboot) + String(serverTail);
    server.send(200, F("text/html"), payload);
    server.close();
    delay(1000);
    ESP.restart();
  });
  server.on(F("/format"), HTTP_GET, []() {
    syslog.log(LOG_INFO, F("webServer: format"));
    Serial.println(F("webServer: format"));
    SPIFFS.format();
    String payload = String(serverReboot) + String(serverTail);
    server.send(200, F("text/html"), payload);
    server.close();
    delay(1000);
    ESP.restart();
  });
  server.on(F("/favicon.ico"), HTTP_GET, []() {
    server.sendHeader(F("Location"), F("https://www.arduino.cc/favicon.ico"));
    server.send(301);
  });
  server.on(F("/update"), HTTP_POST, []() {
    syslog.log(LOG_INFO, F("webServer: update"));
    server.send(200, F("text/plain"), (Update.hasError()) ? "FAIL" : "OK");
    server.close();
    delay(1000);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      display_ticker.detach();
      WiFiUDP::stopAll();
      Serial.printf("Update: %s\n", upload.filename.c_str());
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      if (!Update.begin(maxSketchSpace)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
    yield();
  });
  server.onNotFound(handleNotFound);
  server.begin();
  MDNS.addService("http", "tcp", 80);
}
