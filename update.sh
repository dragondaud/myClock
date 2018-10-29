#!/bin/bash
# update myClock, ESP core, and required libraries
# under Windows use git bash in git for Windows https://gitforwindows.org/
# MacOS requires GNU 'coreutils' for this script to function

SCRIPT=`realpath $0`
SCRIPTPATH=`dirname $SCRIPT`
ARDUINO=`realpath $SCRIPTPATH/..`
cd $SCRIPTPATH && git pull --no-edit
cd $ARDUINO/hardware/esp8266com/esp8266 && git pull --no-edit
cd $ARDUINO/libraries/ArduinoJson && git pull --no-edit
cd $ARDUINO/libraries/Syslog && git pull --no-edit
cd $ARDUINO/libraries/Adafruit-GFX-Library && git pull --no-edit
cd $ARDUINO/libraries/PxMatrix && git pull --no-edit
cd $ARDUINO/libraries/WiFiManager && git pull --no-edit
cd $ARDUINO/libraries/DallasTemperature && git pull --no-edit
cd $ARDUINO/libraries/OneWire && git pull --no-edit

