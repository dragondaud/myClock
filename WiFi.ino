void configModeCallback (WiFiManager *myWiFiManager) {
  display.clearDisplay();
  display.setFont(&Picopixel);
  display.setCursor(2, row1);
  display.print("AP: ");
  display.print(myWiFiManager->getConfigPortalSSID());
  display.setCursor(2, row2);
  display.print("Pass: ");
  display.print(SOFTAP_PASS);
  display.setCursor(2, row3);
  display.print("IP: ");
  display.print(WiFi.softAPIP());
}

