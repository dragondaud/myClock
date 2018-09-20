#!/bin/bash
# build.sh -- Build myClock from command line
# https://github.com/arduino/Arduino/blob/master/build/shared/manpage.adoc

arduino="`which arduino`"
buildpath="$HOME/.build"

# d1_mini
board="esp8266com:esp8266:d1_mini:xtal=80,vt=flash,eesz=4M1M,ip=lm2,dbg=Disabled,lvl=NoAssert-NDEBUG,wipe=none,baud=921600"

# dt_mini_lite
#board="esp8266com:esp8266:d1_mini:xtal=80,vt=flash,eesz=1M64,ip=lm2,dbg=Disabled,lvl=NoAssert-NDEBUG,wipe=none,baud=921600"

if [[ "$OSTYPE" == "darwin"* ]]; then
arduino="/Applications/Arduino.app/Contents/MacOS/Arduino"
elif [[ "$OSTYPE" == "cygwin" ]]; then
arduino="/c/Program Files (x86)/Arduino/arduino_debug.exe"
buildpath="${USERPROFILE}\.build"
elif [[ "$OSTYPE" == "msys" ]]; then
arduino="/c/Program Files (x86)/Arduino/arduino_debug.exe"
buildpath="${USERPROFILE}\.build"
fi

"${arduino}" --verbose-build --pref build.path="${buildpath}" --board "${board}" --save-prefs --verify myClock.ino
