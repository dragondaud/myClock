#!/bin/bash
# Linux: install Arduino IDE, ESP8266 core 2.5.0 and required libraries
# MacOS: install ESP8266 core and required libs only

if [[ "$OSTYPE" =~ linux ]]; then
	sudo -v || exit
	sudo apt update && sudo apt full-upgrade -y && sudo apt autoremove -y
	sudo apt install build-essential openssh-server git ubuntu-make -y
	sudo usermod -a -G dialout $USER
	umake ide arduino
	cd $HOME
	mkdir -p Arduino && cd Arduino
	arduino="`which arduino`"
elif [[ "$OSTYPE" =~ darwin ]]; then
	if [[ ! -d /Applications/Arduino.app ]]; then
		echo "Install Arduino IDE from https://www.arduino.cc/en/Main/Software"
		exit 0
	fi
	xcode-select --install
	mkdir -p $HOME/Documents/Arduino && cd $HOME/Documents/Arduino
	arduino="/Applications/Arduino.app/Contents/MacOS/Arduino"
else
	echo "Unsupported platform"
	exit 0
fi

"${arduino}" --pref boardsmanager.additional.urls=http://arduino.esp8266.com/stable/package_esp8266com_index.json --save-prefs 2>/dev/null
"${arduino}" --install-boards "esp8266:esp8266:2.5.0" 2>/dev/null
"${arduino}" --board "esp8266:esp8266:d1_mini:xtal=160,vt=flash,eesz=4M1M,ip=lm2f,dbg=Disabled,lvl=NoAssert-NDEBUG,wipe=none,baud=921600" --save-prefs 2>/dev/null

esp="`\"${arduino}\" --get-pref runtime.platform.path 2>/dev/null`"
( cd $esp && python ./tools/boards.txt.py --nofloat --boardsgen )

[ ! -d "myClock" ] && git clone https://github.com/dragondaud/myClock.git

mkdir -p libraries && cd libraries
[ ! -d "ArduinoJson" ] && ( git clone https://github.com/bblanchon/ArduinoJson.git && cd ArduinoJson && git checkout 5.x -q )
[ ! -d "Syslog" ] && git clone https://github.com/arcao/Syslog.git
[ ! -d "Adafruit-GFX-Library" ] && git clone https://github.com/adafruit/Adafruit-GFX-Library.git
[ ! -d "PxMatrix" ] && git clone https://github.com/2dom/PxMatrix.git
[ ! -d "WiFiManager" ] && ( git clone https://github.com/tzapu/WiFiManager.git && cd WiFiManager && git checkout development -q )
[ ! -d "DallasTemperature" ] && git clone https://github.com/milesburton/Arduino-Temperature-Control-Library DallasTemperature
[ ! -d "OneWire" ] && git clone https://github.com/PaulStoffregen/OneWire
