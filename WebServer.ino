// Web Server for configuration and updates

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

static const char* serverHead PROGMEM =
  "<!DOCTYPE HTML><html><head>\n<title>myClock</title>\n<style>\n"
  "body {background-color: DarkSlateGray; color: White; font-family: sans-serif;}\n"
  "div {max-width: 500px; border: ridge; padding: 10px; background-color: SlateGray;}\n"
  "</style></head>\n"
  "<body><h1>myClock " VERSION "</h1>\n";

static const char* serverRoot PROGMEM =
  "<div><h2>Update Firmware</h2>\n"
  "<form method='POST' action='/update' enctype='multipart/form-data'>\n"
  "<input type='file' name='update'>\n"
  "<p><input type='submit' value='UPDATE'></form></div><p>\n";

static const char* serverColor PROGMEM =
  "<p><form method='POST' action='/color' id='colorForm' name='colorForm'>\n"
  "<input type='color' id='myColor' name='myColor' value='#######'> "
  "<input type='submit' value='Set Color'></form><p>\n";

static const char* serverConfig PROGMEM =
  "<div><h2>Edit Config</h2>\n"
  "<form method='post' action='/save' id='configForm' name='configForm'>\n"
  "<input type='submit' value='SAVE'> <input type='reset'>\n"
  "<p><textarea style='resize: none;' id='json' rows='13' cols='50' "
  "maxlength='400' name='json' form='configForm'>\n";

static const char* serverTail PROGMEM =
  "<p><form method='GET' action='/reset'><input type='submit' value='REBOOT CLOCK'></form>\n"
  "<p><form method='GET' action='/logout'><input type='submit' value='LOGOUT'></form>\n"
  "</body></html>";

static const char* serverReboot PROGMEM =
  "<!DOCTYPE HTML><html><head>\n"
  "<meta http-equiv=\"refresh\" content=\"10;url=/\" />"
  "<style>body {background-color: DarkSlateGray; color: White;}"
  "</style></head>\n"
  "<body><h1>myClock " VERSION "</h1>"
  "Rebooting...</body></html>";

static const char* textPlain PROGMEM = "text/plain";
static const char* textHtml PROGMEM = "text/html";

void handleNotFound() {
#ifdef SYSLOG
  syslog.log(LOG_INFO, F("webServer: Not Found"));
#endif
  server.sendHeader(F("Location"), F("/"));
  server.send(301);
}

void handleColor() {
  if (!server.authenticate("admin", softAPpass.c_str())) return server.requestAuthentication(DIGEST_AUTH);
  if (!server.hasArg("myColor")) return server.send(503, textPlain, F("FAILED"));
  String c = server.arg("myColor");
#ifdef SYSLOG
  syslog.logf(LOG_INFO, "webServer: color %s", c.c_str());
#endif
  myColor = htmlColor565(c);
  displayDraw(brightness);
  getWeather();
  writeSPIFFS();
  server.sendHeader(F("Location"), F("/"));
  server.send(301);
}

void handleSave() {
  if (!server.authenticate("admin", softAPpass.c_str())) return server.requestAuthentication(DIGEST_AUTH);
  if (!server.hasArg("json")) return server.send(503, textPlain, F("FAILED"));
#ifdef SYSLOG
  syslog.log(LOG_INFO, F("webServer: save"));
#endif
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
    myColor = json["myColor"];
    threshold = json["threshold"];
#ifdef SYSLOG
    String sl = json["syslogSrv"];
    if (sl != "") syslogSrv = sl;
    syslogPort = json["syslogPort"];
#endif
  }
  writeSPIFFS();
  server.send(200, textHtml, serverReboot);
  server.close();
  delay(1000);
  ESP.restart();
}

void handleRoot() {
  if (!server.authenticate("admin", softAPpass.c_str())) return server.requestAuthentication(DIGEST_AUTH);
#ifdef SYSLOG
  syslog.log(LOG_INFO, F("webServer: root"));
#endif
  server.sendHeader(F("Connection"), F("close"));
  time_t now = time(nullptr);
  String t = ctime(&now);
  t.trim();
  String payload = String(serverHead) + F("<h3>") + t + F("</h3>\n<p>");
  if (LIGHT) payload = payload + F("LDR: ") + String(light) + ", ";
  payload = payload + F("Heap: ") + String(ESP.getFreeHeap()) + "\n";
  payload += String(serverRoot);
  payload += String(serverColor);
  char c[8];
  sprintf(c, "#%06X", color565to888(myColor));
  payload.replace("#######", String(c));
  payload += String(serverConfig) + getSPIFFS() + F("</textarea></form></div>\n");
  payload += String(serverTail);
  server.send(200, textHtml, payload);
}

void handleReset() {
  if (!server.authenticate("admin", softAPpass.c_str())) return server.requestAuthentication(DIGEST_AUTH);
#ifdef SYSLOG
  syslog.log(LOG_INFO, F("webServer: reset"));
#endif
  Serial.println(F("webServer: reset"));
  server.send(200, textHtml, serverReboot);
  server.close();
  delay(1000);
  ESP.restart();
}

void handleLogout() {
  server.send(401, textPlain, "logged out");
}

void startWebServer() {
  server.on(F("/"), HTTP_GET, handleRoot);
  server.on(F("/save"), handleSave);
  server.on(F("/color"), handleColor);
  server.on(F("/reset"), HTTP_GET, handleReset);
  server.on(F("/logout"), HTTP_GET, handleLogout);
  server.on(F("/favicon.ico"), HTTP_GET, []() {
    server.sendHeader(F("Location"), F("https://www.arduino.cc/favicon.ico"));
    server.send(301);
  });
  server.on(F("/update"), HTTP_POST, []() {
    if (!server.authenticate("admin", softAPpass.c_str())) return server.requestAuthentication(DIGEST_AUTH);
#ifdef SYSLOG
    syslog.log(LOG_INFO, F("webServer: update"));
#endif
    server.send(200, textPlain, (Update.hasError()) ? "FAIL" : "OK");
    server.close();
    delay(1000);
    ESP.restart();
  }, []() {
    if (!server.authenticate("admin", softAPpass.c_str())) return server.requestAuthentication(DIGEST_AUTH);
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
