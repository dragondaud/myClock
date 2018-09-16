// Web Server for configuration and updates

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

static const char* serverRoot PROGMEM =
  "<!DOCTYPE HTML><html><head><style>"
  "body {background-color: black; color: White;}"
  "</style></head><body><h1>myClock " VERSION "</h1>"
  "<h2>Update Firmware</h2><form method='POST' action='/update' enctype='multipart/form-data'>"
  "<input type='file' name='update'><input type='submit' value='Update'></form>"
  "<p><form method='GET' action='/reset'><input type='submit' value='REBOOT'></form>";

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

void startWebServer() {
  server.on(F("/"), HTTP_GET, []() {
    syslog.log(LOG_INFO, F("webServer: index"));
    server.sendHeader(F("Connection"), F("close"));
    time_t now = time(nullptr);
    String t = ctime(&now);
    String payload = String(serverRoot) + F("<h3>") + t + F("</h3><p>Light Level: ") + String(light) + String(serverTail);
    server.send(200, F("text/html"), payload);
  });
  server.on(F("/reset"), HTTP_GET, []() {
    syslog.log(LOG_INFO, F("webServer: reset"));
    String payload = String(serverReboot) + String(serverTail);
    server.send(200, F("text/html"), payload);
    server.close();
    delay(1000);
    ESP.restart();
  });
  server.on(F("/favicon.ico"), HTTP_GET, []() {
    syslog.log(LOG_INFO, F("webServer: favicon.ico"));
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
