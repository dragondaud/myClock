#!/bin/bash
# build.sh -- Build myClock from command line
# https://github.com/arduino/Arduino/blob/master/build/shared/manpage.adoc
# under Windows use git bash in git for Windows https://gitforwindows.org/

APP="myClock"
SRC="$APP.ino"
BIN="$SRC.bin"
RM=`which rm`
PYTHON=`which python`
BOLD='\033[1;34m'
NC='\033[0m'

#default board d1_mini
boardsmanager="http://arduino.esp8266.com/stable/package_esp8266com_index.json"
boardver="esp8266:esp8266:2.5.0"
board="esp8266:esp8266:d1_mini:xtal=160,vt=flash,eesz=4M1M,ip=lm2f,dbg=Disabled,lvl=NoAssert-NDEBUG,wipe=none,baud=921600"
port="8266"

if [[ "$OSTYPE" == "darwin"* ]]; then
	arduino="/Applications/Arduino.app/Contents/MacOS/Arduino"
	buildpath="$HOME/.build"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
	arduino="/c/Program Files (x86)/Arduino/arduino_debug.exe"
	buildpath="${USERPROFILE}\.build"
else
	arduino="`which arduino`"
	if [ -z "$arduino" ]; then
		arduino="`find ~/.local /usr -type f -name arduino -print -quit`"
	fi
	buildpath="$HOME/.build"
fi

if [ -z "$arduino" ]; then
	echo "Arduino IDE must be installed" >&2
	exit 1
fi

SKETCHBOOK="`\"${arduino}\" --get-pref sketchbook.path 2>/dev/null`"

while getopts ":lvf:s:hcwuo" opt; do
	case $opt in
		l)
			echo -e "Board ${BOLD}lolin32${NC} selected." >&2
			boardsmanager="https://dl.espressif.com/dl/package_esp32_index.json"
			boardver="esp32:esp32:1.0.2"
			board="esp32:esp32:lolin32:PartitionScheme=min_spiffs"
			port="3232"
			;;
		v)
			verbose="--verbose"
			debug="--debug"
			;;
		f)
			FLASH=$OPTARG
			espota="`\"${arduino}\" --get-pref runtime.platform.path 2>/dev/null`/tools/espota.py"
			echo "Uploading to $FLASH with $espota" >&2
			;;
		s)
			SER=$OPTARG
			esptool="`\"${arduino}\" --get-pref runtime.tools.esptool.path 2>/dev/null`/esptool"
			echo "Uploading to $SER with $esptool." >&2
			;;
		c)
			echo -e "${BOLD}Cleaning${NC} ${buildpath}..." >&2
			"${RM}" ${verbose} --one-file-system -rf "${buildpath}";;
		w)
			echo "Wipe entire flash on upload." >&2
			board=${board/wipe=none/wipe=all};;
		u)
			echo -en "${BOLD}`basename $PWD`: ${NC}" && git pull --no-edit
			if [ ! -d "$SKETCHBOOK/libraries/ArduinoJson" ]; then
				( cd $SKETCHBOOK/libraries/ && git clone https://github.com/bblanchon/ArduinoJson.git -b 5.x )
			else
				( cd $SKETCHBOOK/libraries/ArduinoJson && echo -en "${BOLD}`basename $PWD`: ${NC}" && git fetch && git checkout 5.x -f )
			fi
			if [ ! -d "$SKETCHBOOK/libraries/Syslog" ]; then
				( cd $SKETCHBOOK/libraries/ && git clone https://github.com/arcao/Syslog.git )
			else
				( cd $SKETCHBOOK/libraries/Syslog && echo -en "${BOLD}`basename $PWD`: ${NC}" && git pull --no-edit )
			fi
			if [ ! -d "$SKETCHBOOK/libraries/Adafruit-GFX-Library" ]; then
				( cd $SKETCHBOOK/libraries/ && git clone https://github.com/adafruit/Adafruit-GFX-Library.git )
			else
				( cd $SKETCHBOOK/libraries/Adafruit-GFX-Library && echo -en "${BOLD}`basename $PWD`: ${NC}" && git pull --no-edit )
			fi
			if [ ! -d "$SKETCHBOOK/libraries/PxMatrix" ]; then
				( cd $SKETCHBOOK/libraries/ && git clone https://github.com/2dom/PxMatrix.git -b v1.3.0 )
			else
				( cd $SKETCHBOOK/libraries/PxMatrix && echo -en "${BOLD}`basename $PWD`: ${NC}" && git fetch && git checkout v1.3.0 -f )
			fi
			if [ ! -d "$SKETCHBOOK/libraries/WiFiManager" ]; then
				( cd $SKETCHBOOK/libraries/ && git clone https://github.com/tzapu/WiFiManager.git -b development )
			else
				( cd $SKETCHBOOK/libraries/WiFiManager && echo -en "${BOLD}`basename $PWD`: ${NC}" && git fetch && git checkout development -f )
			fi
			if [ ! -d "$SKETCHBOOK/libraries/DallasTemperature" ]; then
				( cd $SKETCHBOOK/libraries/ && git clone https://github.com/milesburton/Arduino-Temperature-Control-Library DallasTemperature )
			else
				( cd $SKETCHBOOK/libraries/DallasTemperature && echo -en "${BOLD}`basename $PWD`: ${NC}" && git pull --no-edit )
			fi
			if [ ! -d "$SKETCHBOOK/libraries/OneWire" ]; then
				(cd $SKETCHBOOK/libraries/ && git clone https://github.com/PaulStoffregen/OneWire )
			else
				( cd $SKETCHBOOK/libraries/OneWire && echo -en "${BOLD}`basename $PWD`: ${NC}" && git pull --no-edit )
			fi
			exit 0;;
		o)
			echo -e "${BOLD}Select ${boardver}${NC}" >&2
			pref="`\"${arduino}\" --get-pref runtime.hardware.path 2>/dev/null`/../../../../preferences.txt"
			"${arduino}" --board "${board}" --save-prefs
			ret=$?
			if [ $ret -ne 0 ]; then
				echo -e "${BOLD}Install ${boardver}${NC}" >&2
				rm -v $pref
				"${arduino}" --pref boardsmanager.additional.urls=${boardsmanager} --save-prefs &>/dev/null
				"${arduino}" --install-boards "${boardver}" &>/dev/null
				"${arduino}" --board "${board}" --save-prefs
				ret=$?
				if [ $ret -ne 0 ]; then
					echo -e "${BOLD}FAILED! Delete preferences.txt and try again.${NC}" >&2
					exit 1
				fi
			fi
			if [[ $board == esp8266* ]]; then
				echo -e "${BOLD}Generate boards.txt${NC}" >&2
				tools="`\"${arduino}\" --get-pref runtime.platform.path 2>/dev/null`"
				( cd $tools && ${PYTHON} ./tools/boards.txt.py --nofloat --boardsgen)
			fi
			exit 0;;
		:)
			echo "Option -$OPTARG requires an argument." >&2
			exit 1;;
		*)
			echo "Usage: build.sh [-l] [-v] [-f IP] [-s /dev/ttyX|COMx] [-h] [-c] [-w]"
			echo ""
			echo "Build ${APP} and optionally flash device."
			echo ""
			echo "Options:"
			echo -e "\t-l\tBuild for lolin32 instead of d1_mini"
			echo -e "\t-v\tUse verbose output"
			echo -e "\t-f\tIP address of ${APP} to update"
			echo -e "\t-s\tSerial device of ${APP} to update"
			echo -e "\t-h\tDisplay usage and exit"
			echo -e "\t-c\tClean build directory before building"
			echo -e "\t-w\tWipe entire flash, instead of sketch only"
			echo -e "\t-u\tUpdate or install required libraries and exit"
			echo -e "\t-o\tUpdate or install core and exit\n"
			echo "Arduino: $arduino"
			echo "Sketchbook: $SKETCHBOOK"
			echo "Board: $board"
			exit 0;;
	esac
done

echo -e "${BOLD}Building ${APP}${NC} in ${buildpath}..." >&2
"${arduino}" ${verbose} --pref build.path="${buildpath}" --board "${board}" --save-prefs --verify ${SRC}
ret=$?
if [ $ret -eq 0 ]; then
	if [ -f "$espota" ] && [ ! -z "$FLASH" ]; then
		echo "Flashing ${BIN} to ${FLASH}..." >&2
		"${PYTHON}" "${espota}" ${debug} --progress --file="${buildpath}/${BIN}" --ip=${FLASH} --port=${port}
	elif [ -f "$esptool" ] && [ ! -z "$SER" ]; then
		echo "Flashing ${BIN} to ${SER}..." >&2
		"${esptool}" -cp ${SER} -cb 921600 -ca 0x0 -cd nodemcu -cf "${buildpath}/${BIN}"
	fi
	echo "build complete" >&2
else
	exit $ret
fi


