#!/bin/bash
# update myClock, ESP core, and required libraries
# under Windows use git bash in git for Windows https://gitforwindows.org/
# MacOS requires GNU 'coreutils' for this script to function

SCRIPT=`realpath $0`
SCRIPTPATH=`dirname $SCRIPT`
ARDUINO=`realpath $SCRIPTPATH/..`
cd $SCRIPTPATH  && echo -n "`basename $PWD`: " && git pull --no-edit
cd $ARDUINO/hardware/esp8266com/esp8266 && echo -n "`basename $PWD`: " && git pull --no-edit
cd $ARDUINO/libraries/ArduinoJson && echo -n "`basename $PWD`: " && git pull --no-edit
cd $ARDUINO/libraries/Syslog && echo -n "`basename $PWD`: " && git pull --no-edit
cd $ARDUINO/libraries/Adafruit-GFX-Library && echo -n "`basename $PWD`: " && git pull --no-edit
cd $ARDUINO/libraries/PxMatrix && echo -n "`basename $PWD`: " && git pull --no-edit
cd $ARDUINO/libraries/WiFiManager && echo -n "`basename $PWD`: " && git checkout development -q && git pull --no-edit
cd $ARDUINO/libraries/DallasTemperature && echo -n "`basename $PWD`: " && git pull --no-edit
cd $ARDUINO/libraries/OneWire && echo -n "`basename $PWD`: " && git pull --no-edit

