#!/bin/bash
# install latest Arduino IDE and required libraries on Linux

if [[ "$OSTYPE" =~ linux ]]; then
	sudo -v || exit
	sudo apt update && sudo apt full-upgrade -y && sudo apt autoremove -y
	sudo apt install build-essential openssh-server git ubuntu-make -y
	sudo usermod -a -G dialout $USER
	umake ide arduino
	cd $HOME
	mkdir -p Arduino && cd Arduino
elif [[ "$OSTYPE" =~ darwin ]]; then
	if [[ ! -d /Applications/Arduino.app ]]; then
		echo "Install Arduino IDE from https://www.arduino.cc/en/Main/Software"
	fi
	mkdir -p $HOME/Documents/Arduino && cd $HOME/Documents/Arduino
fi

[ ! -d "myClock" ] && git clone https://github.com/dragondaud/myClock.git
mkdir -p hardware/esp8266com && cd hardware/esp8266com
[ ! -d "esp8266" ] && git clone https://github.com/esp8266/Arduino.git esp8266
cd esp8266
./tools/boards.txt.py --nofloat --allgen
cd tools
python get.py
cd ../../../..
mkdir -p libraries && cd libraries
[ ! -d "ArduinoJson" ] && git clone https://github.com/bblanchon/ArduinoJson.git
[ ! -d "Syslog" ] && git clone https://github.com/arcao/Syslog.git
[ ! -d "Adafruit-GFX-Library" ] && git clone https://github.com/adafruit/Adafruit-GFX-Library.git
[ ! -d "PxMatrix" ] && git clone https://github.com/2dom/PxMatrix.git
[ ! -d "WiFiManager" ] && git clone https://github.com/tzapu/WiFiManager.git

