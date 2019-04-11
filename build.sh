#!/bin/bash
# build.sh -- Build myClock from command line
# https://github.com/arduino/Arduino/blob/master/build/shared/manpage.adoc
# under Windows use git bash in git for Windows https://gitforwindows.org/

APP="myClock"
SRC="$APP.ino"
BIN="$SRC.bin"
RM=`which rm`

#default board d1_mini
board="esp8266:esp8266:d1_mini:xtal=160,vt=flash,eesz=4M1M,ip=lm2f,dbg=Disabled,lvl=NoAssert-NDEBUG,wipe=none,baud=921600"
port="8266"

while getopts ":lvf:s:hcw" opt; do
	case $opt in
		l)
			echo "Board lolin32 selected." >&2
			board="espressif:esp32:lolin32:PartitionScheme=min_spiffs"
			port="3232"
			;;
		v)
			verbose="--verbose"
			debug="--debug"
			;;
		f)
			FLASH=$OPTARG
			echo "Uploading to $FLASH after build." >&2
			;;
		s)
			SER=$OPTARG
			echo "Uploading to $SER after build." >&2
			;;
		h)
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
			exit 0;;
		c)
			clean=true;;
		w)
			echo "Wipe entire flash on upload." >&2
			board=${board/wipe=none/wipe=all};;
		\?)
			echo "Invalid option -$OPTARG" >&2
			exit 1;;
		:)
			echo "Option -$OPTARG requires an argument." >&2
			exit 1;;
		
	esac
done

if [[ "$OSTYPE" == "darwin"* ]]; then
	arduino="/Applications/Arduino.app/Contents/MacOS/Arduino"
	buildpath="$HOME/.build"
	espota="python `\"${arduino}\" --get-pref runtime.platform.path 2>/dev/null`/tools/espota.py"
	esptool="`\"${arduino}\" --get-pref runtime.tools.esptool.path 2>/dev/null`/esptool"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
	arduino="/c/Program Files (x86)/Arduino/arduino_debug.exe"
	buildpath="${USERPROFILE}\.build"
	espota="`\"${arduino}\" --get-pref runtime.platform.path 2>/dev/null`/tools/espota.py"
	esptool="`\"${arduino}\" --get-pref runtime.tools.esptool.path 2>/dev/null`/esptool"
else
	arduino="`which arduino`"
	if [ -z "$arduino" ]; then
		arduino="`find ~/.local /usr -type f -name arduino -print -quit`"
	fi
	buildpath="$HOME/.build"
	espota="`\"${arduino}\" --get-pref runtime.platform.path 2>/dev/null`/tools/espota.py"
	esptool="`\"${arduino}\" --get-pref runtime.tools.esptool.path 2>/dev/null`/esptool"
fi

if [ -z "$arduino" ]; then
	echo "Arduino IDE must be installed" >&2
	exit 1
else
	if [ "$clean" = true ]; then
		echo "Cleaning ${buildpath}..." >&2
		"${RM}" ${verbose} --one-file-system -rf "${buildpath}"
	fi
	echo "Building ${APP} in ${buildpath}..." >&2
	"${arduino}" ${verbose} --pref build.path="${buildpath}" --board "${board}" --save-prefs --verify ${SRC}
	ret=$?
	if [ $ret -eq 0 ]; then
		if [ -f "$espota" ] && [ ! -z "$FLASH" ]; then
			echo "Flashing ${BIN} to ${FLASH}..." >&2
			"${espota}" ${debug} --progress --file="${buildpath}/${BIN}" --ip=${FLASH} --port=${port}
		elif [ -f "$esptool" ] && [ ! -z "$SER" ]; then
			echo "Flashing ${BIN} to ${SER}..." >&2
			"${esptool}" --port ${SER} --baud 921600 write_flash 0x0 "${buildpath}/${BIN}"
		fi
		echo "build complete"
	else
		exit $ret
	fi
fi

