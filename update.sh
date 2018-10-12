#!/bin/bash
# update myClock, ESP core, and required libraries
# under Windows use git bash in git for Windows https://gitforwindows.org/

SCRIPT=`realpath $0`
SCRIPTPATH=`dirname $SCRIPT`
cd $SCRIPTPATH
git pull --no-edit
cd ../hardware/esp8266com/esp8266
git pull --no-edit
cd ../../../libraries/ArduinoJson
git pull --no-edit
cd ../Syslog
git pull --no-edit
cd ../Adafruit-GFX-Library
git pull --no-edit
cd ../PxMatrix
git pull --no-edit
cd ../WiFiManager
git pull --no-edit
cd "$OLDPWD"
