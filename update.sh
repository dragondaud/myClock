#!/bin/bash
# update myClock and required libraries
# works on Linux, MacOS and Windows with git bash

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
