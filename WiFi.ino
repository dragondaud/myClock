void configModeCallback (WiFiManager *myWiFiManager) {
  display.clearDisplay();
  display.setFont(&Picopixel);
  display.setCursor(2, row1);
  display.print("AP: myClock");
  display.setCursor(2, row2);
  display.print("Pass: ConFigMe");
  display.setCursor(2, row3);
  display.print("IP: 192.168.4.1");
}

