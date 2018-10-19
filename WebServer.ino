// Web Server for configuration and updates

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define ADMIN_USER "admin"

static const char* serverHead PROGMEM =
  "<!DOCTYPE HTML><html><head>\n<title>myClock</title>\n"
  "<link href='stylesheet.css' rel='stylesheet' type='text/css'></head>\n"
  "<body><a href='https://github.com/dragondaud/myClock' target='_blank'>\n"
  "<h1>myClock " VERSION "</h1></a>\n";

static const char* serverStyle PROGMEM =
  "body {background-color: DarkSlateGray; color: White; font-family: sans-serif;}\n"
  "a {text-decoration: none; color: LightSteelBlue;}\n"
  "a:hover {text-decoration: underline; color: SteelBlue;}\n"
  "div {max-width: 500px; border: ridge; padding: 10px; background-color: SlateGray;}\n"
  "th, td {padding: 5px; text-align: right;}\n"
  "input[type=range] {vertical-align: middle;}\n"
  "meter {width: 400px; vertical-align: middle;}\n"
  "meter::after {content: attr(value); position:relative; top:-17px; color: Black;}\n"
  "meter::-webkit-meter-bar {background: none; background-color: LightBlue; "
  "box-shadow: 5px 5px 5px SlateGray inset; border: 1px solid; }\n"
  ".switch {position: relative; display: inline-block; width: 60px; height: 34px;}\n"
  ".switch input {opacity: 0; width: 0; height: 0;}\n"
  ".slider {position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; -webkit-transition: .4s; transition: .4s;}\n"
  ".slider:before {position: absolute; content: ''; height: 26px; width: 26px; left: 4px; bottom: 4px; background-color: white; -webkit-transition: .4s; transition: .4s;}\n"
  "input:checked + .slider {background-color: DarkSlateGray;}\n"
  "input:focus + .slider {box-shadow: 0 0 1px SlateGray;}\n"
  "input:checked + .slider:before {-webkit-transform: translateX(26px); -ms-transform: translateX(26px); transform: translateX(26px);}\n"
  ".slider.round {border-radius: 34px;}\n"
  ".slider.round:before {border-radius: 50%;}\n";

static const char* serverUpdate PROGMEM =
  "<div><h2>Update Firmware</h2>\n"
  "<form method='POST' action='/update' enctype='multipart/form-data'>\n"
  "<input type='file' name='update'>\n"
  "<p><input type='submit' value='UPDATE'></form></div><p>\n";

static const char* serverOptions PROGMEM =
  "<div><h2>myClock Options</h2>\n"
  "<form method='POST' action='/options' id='optionsForm' name='optionsForm'>\n"
  "<table><tr><th><label for='myColor'>Color</label></th>\n"
  "<th><input type='color' id='myColor' name='myColor' value='%myColor%'></th></tr>\n"
  "<tr><th><label for='brightness'> Brightness</label></th>\n"
  "<th><input type='number' id='brightNum' name='brightNum' style='width: 3em;'"
  "min='1' max='255' value='%brightness%' oninput='brightness.value=brightNum.value'> \n"
  "<input type='range' id='brightness' name='brightness' "
  "min='1' max='255' value='%brightness%' oninput='brightNum.value=brightness.value'></th></tr>\n"
  "<tr><th><label for='milTime'>24hour Time</label></th>\n"
  "<th><label class='switch'><input type='checkbox' id='milTime' name='milTime' %milTime%>"
  "<span class='slider round'></span></label></th></tr>\n"
  "<tr><th><label for='celsius'>Celsius</label></th>\n"
  "<th><label class='switch'><input type='checkbox' id='celsius' name='celsius' %celsius%>"
  "<span class='slider round'></span></label></th></tr>\n"
  "<tr><th><label for='language'>Language</label></th>\n"
  "<th><select name='language' id='language'>\n"
  "<option value='%lang%'>Select (%lang%)</option>\n"
  "<option value='en'>English</option>\n"
  "<option value='hr'>Croatian</option>\n"
  "<option value='cz'>Czech</option>\n"
  "<option value='nl'>Dutch</option>\n"
  "<option value='fi'>Finnish</option>\n"
  "<option value='fr'>French</option>\n"
  "<option value='gl'>Galician</option>\n"
  "<option value='de'>German</option>\n"
  "<option value='hu'>Hungarian</option>\n"
  "<option value='it'>Italian</option>\n"
  "<option value='la'>Latvian</option>\n"
  "<option value='lt'>Lithuanian</option>\n"
  "<option value='pl'>Polish</option>\n"
  "<option value='pt'>Portuguese</option>\n"
  "<option value='sk'>Slovak</option>\n"
  "<option value='sl'>Slovenian</option>\n"
  "<option value='es'>Spanish</option></select></th></tr>\n"
  "<tr><th><label for='softAPpass'>Admin Password</label></th>\n"
  "<th><input type='password' id='softAPpass' name='softAPpass' placeholder='new password'></th></tr>\n"
  "<tr><th><label for='location'>Postal Code</label></th>\n"
  "<th><input type='number' id='location' name='location' style='width: 5em' value='%location%'></th></tr>\n"
  "<tr><th><input type='submit' value='SET OPTIONS'></th></tr>\n"
  "</table></form><p></div><p>\n";

static const char* serverConfig PROGMEM =
  "<div><h2>Edit Config</h2>\n"
  "<form method='post' action='/save' id='configForm' name='configForm'>\n"
  "<input type='submit' value='SAVE'> <input type='reset'>\n"
  "<p><textarea style='resize: none;' id='json' rows='15' cols='50' "
  "maxlength='500' name='json' form='configForm'>\n";

static const char* serverTail PROGMEM =
  "<p><form method='GET' action='/reset'><input type='submit' value='REBOOT CLOCK'></form>\n"
  "<p><form method='GET' action='/logout'><input type='submit' value='LOGOUT'></form>\n"
  "</body></html>";

static const char* serverReboot PROGMEM =
  "<!DOCTYPE HTML><html><head>\n"
  "<meta http-equiv=\"refresh\" content=\"15;url=/\" />"
  "<style>body {background-color: DarkSlateGray; color: White;}"
  "</style></head>\n"
  "<body><h1>myClock " VERSION "</h1>"
  "Rebooting...</body></html>";

static const char* textPlain PROGMEM = "text/plain";
static const char* textHtml PROGMEM = "text/html";
static const char* textCss PROGMEM = "text/css";
static const char* checked PROGMEM = "checked";

void handleNotFound() {
#ifdef SYSLOG
  syslog.log(F("webServer: Not Found"));
#endif
  server.sendHeader(F("Location"), F("/"));
  server.send(301);
}

bool handleAuth() {
  return server.authenticate(ADMIN_USER, softAPpass.c_str());
}

void reqAuth() {
  return server.requestAuthentication(DIGEST_AUTH, HOST);
}

void handleStyle() {
  if (!handleAuth()) return reqAuth();
  server.send(200, textCss, String(serverStyle));
}

void handleOptions() {
  if (!handleAuth()) return reqAuth();
  if (!server.hasArg(F("myColor"))) return server.send(503, textPlain, F("FAILED"));
  String c = server.arg(F("myColor"));
  if (c != "") myColor = htmlColor565(c);
  uint8_t b = server.arg(F("brightness")).toInt();
  if (b) brightness = b;
  else brightness = 255;
  c = server.arg(F("milTime"));
  if (c == "on") milTime = true;
  else milTime = false;
  c = server.arg(F("celsius"));
  if (c == "on") celsius = true;
  else celsius = false;
  c = server.arg(F("language"));
  if (c != "") language = c;
  c = server.arg(F("softAPpass"));
  if (c != "") softAPpass = c;
  c = server.arg(F("location"));
  if (c != "") location = c;
  displayDraw(brightness);
  getWeather();
  writeSPIFFS();
  server.sendHeader(F("Location"), F("/"));
  server.send(301);
}

void handleSave() {
  if (!handleAuth()) return reqAuth();
  if (!server.hasArg(F("json"))) return server.send(503, textPlain, F("FAILED"));
#ifdef SYSLOG
  syslog.log(F("webServer: save"));
#endif
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(server.arg(F("json")));
  parseJson(json);
  writeSPIFFS();
  server.send(200, textHtml, serverReboot);
  server.close();
  delay(1000);
  ESP.restart();
}

void handleRoot() {
  if (!handleAuth()) return reqAuth();
  size_t fh = ESP.getFreeHeap();
#ifdef SYSLOG
  syslog.log(F("webServer: root"));
#endif
  server.sendHeader(F("Connection"), F("close"));
  time_t now = time(nullptr);
  String t = ctime(&now);
  t.trim();
  char c[8];
  String payload = String(serverHead) + F("<h3>") + t + F("</h3>\n");
#ifdef DS18
  payload += "<p><meter value='" + String(Temp) + "' min='-50' max='150'></meter> Temperature\n";
#endif
  if (LIGHT) payload += "<p><meter value='" + String(light) + "' high='" + String(threshold << 1)
                          + "' min='0' max='1023' low='" + String(threshold)
                          + "' optimal='" + String(threshold) + "'></meter> Light Level\n";
  payload += "<p><meter value='" + String(fh) + "' min='0' max='32767'"
             + " low='10000' optimal='15000'></meter> Free Heap\n";
  payload += String(serverOptions);
  sprintf(c, "#%06X", color565to888(myColor));
  payload.replace("%light%", String(light));
  payload.replace("%myColor%", String(c));
  payload.replace("%brightness%", String(brightness));
  payload.replace("%milTime%", milTime ? checked : "");
  payload.replace("%celsius%", celsius ? checked : "");
  payload.replace("%lang%", String(language));
  payload.replace("%location%", String(location));
  payload += String(serverUpdate);
  payload += String(serverConfig) + getSPIFFS() + F("</textarea></form></div>\n");
  payload += String(serverTail);
  server.send(200, textHtml, payload);
}

void handleReset() {
  if (!handleAuth()) return reqAuth();
#ifdef SYSLOG
  syslog.log(F("webServer: reset"));
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
  server.on(F("/stylesheet.css"), HTTP_GET, handleStyle);
  server.on(F("/save"), handleSave);
  server.on(F("/options"), handleOptions);
  server.on(F("/reset"), HTTP_GET, handleReset);
  server.on(F("/logout"), HTTP_GET, handleLogout);
  server.on(F("/favicon.ico"), HTTP_GET, []() {
    server.sendHeader(F("Location"), F("https://www.arduino.cc/favicon.ico"));
    server.send(301);
  });
  server.on(F("/update"), HTTP_POST, []() {
    if (!handleAuth()) return reqAuth();
#ifdef SYSLOG
    syslog.log(F("webServer: update"));
#endif
    server.send(200, textPlain, (Update.hasError()) ? "FAIL" : "OK");
    server.close();
    delay(1000);
    ESP.restart();
  }, []() {
    if (!handleAuth()) return reqAuth();
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      display_ticker.detach();
      WiFiUDP::stopAll();
      Serial.printf_P(PSTR("Update: %s\n"), upload.filename.c_str());
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
        Serial.printf_P(PSTR("Update Success: %u\nRebooting...\n"), upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
    yield();
  });
  server.onNotFound(handleNotFound);
  server.begin();
  MDNS.addService(F("http"), F("tcp"), 80);
}
