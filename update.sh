#!/bin/bash
# update myClock, ESP core, and required libraries
# under Windows use git bash in git for Windows https://gitforwindows.org/
# MacOS requires GNU 'coreutils' for this script to function

SCRIPT=`realpath $0`
SCRIPTPATH=`dirname $SCRIPT`
ARDUINO=`realpath $SCRIPTPATH/..`
BOLD='\033[1;34m'
NC='\033[0m'

while getopts "cth" opt; do
	case $opt in
		c)
			if [ ! -d "$ARDUINO/hardware/esp8266com/esp8266" ]; then
				echo -e "${BOLD}Installing ESP8266 core ${NC}"
				mkdir -p $ARDUINO/hardware/esp8266com
				cd $ARDUINO/hardware/esp8266com
				git clone https://github.com/esp8266/Arduino.git esp8266
				cd esp8266
			else
				echo -e "${BOLD}Updating ESP8266 core ${NC}"
				cd $ARDUINO/hardware/esp8266com/esp8266
				git reset --hard origin/master
				git pull --no-edit
			fi
			git submodule update --init
			./tools/boards.txt.py --nofloat --allgen
			cd tools
			./get.py
			exit 0;;
		t)
			if [ ! -d "$ARDUINO/hardware/espressif/esp32" ]; then
				echo -e "${BOLD}Installing ESP32 core ${NC}"
				mkdir -p $ARDUINO/hardware/espressif
				cd $ARDUINO/hardware/espressif
				git clone https://github.com/espressif/arduino-esp32.git esp32
				cd esp32
				git submodule update --init --recursive
				python -m pip install --upgrade pip
				pip -q install requests --user
			else
				echo -e "${BOLD}Updating ESP32 core ${NC}"
				cd $ARDUINO/hardware/espressif/esp32
				git reset --hard origin/master
				git pull --no-edit
				git submodule update
			fi
			cd tools
			python ./get.py || ./get.exe
			exit 0;;
		h)
			echo "Usage: update.sh [-c] [-t] [-h]"
			echo ""
			echo "default: update myClock and required libraries."
			echo ""
			echo "Options:"
			echo -e "\t-c\tUpdate or install latest github version ESP8266 core"
			echo -e "\t-t\tUpdate or install ESP32 core"
			echo -e "\t-h\tDisplay usage and exit"
			exit 0;;
		\?)
			echo "Invalid option -$OPTARG" >&2
			exit 1;;
	esac
done

cd $SCRIPTPATH  && echo -en "${BOLD}`basename $PWD`: ${NC}" && git pull --no-edit
cd $ARDUINO/libraries/ArduinoJson && echo -en "${BOLD}`basename $PWD`: ${NC}" && git pull --no-edit && git checkout 5.x -q
cd $ARDUINO/libraries/Syslog && echo -en "${BOLD}`basename $PWD`: ${NC}" && git pull --no-edit
cd $ARDUINO/libraries/Adafruit-GFX-Library && echo -en "${BOLD}`basename $PWD`: ${NC}" && git pull --no-edit
cd $ARDUINO/libraries/PxMatrix && echo -en "${BOLD}`basename $PWD`: ${NC}" && git pull --no-edit
cd $ARDUINO/libraries/WiFiManager && echo -en "${BOLD}`basename $PWD`: ${NC}" && git pull --no-edit && git checkout development -q
cd $ARDUINO/libraries/DallasTemperature && echo -en "${BOLD}`basename $PWD`: ${NC}" && git pull --no-edit
cd $ARDUINO/libraries/OneWire && echo -en "${BOLD}`basename $PWD`: ${NC}" && git pull --no-edit
