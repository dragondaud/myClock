#!/bin/bash
# build.sh -- Build myClock from command line
# https://github.com/arduino/Arduino/blob/master/build/shared/manpage.adoc
# under Windows use git bash in git for Windows https://gitforwindows.org/

APP="myClock"
SRC="$APP.ino"
BIN="$SRC.bin"
RM=`which rm`

#default board d1_mini
board="esp8266com:esp8266:d1_mini:xtal=160,vt=flash,eesz=4M1M,ip=lm2f,dbg=Disabled,lvl=NoAssert-NDEBUG,wipe=none,baud=921600"

while getopts ":lvf:hc" opt; do
	case $opt in
		l)
			board="esp8266com:esp8266:d1_mini_lite:xtal=160,vt=flash,eesz=1M64,ip=lm2f,dbg=Disabled,lvl=NoAssert-NDEBUG,wipe=none,baud=921600";;
		v)
			verbose="--verbose"
			debug="--debug";;
		f)
			FLASH=$OPTARG;;
		h)
			echo "Usage: build.sh [-b board] [-f IP] [-h]"
			echo ""
			echo "Build ${APP} and optionally flash device."
			echo ""
			echo "Options:"
			echo -e "\t-l\tBuild for d1_mini_lite instead of d1_mini"
			echo -e "\t-v\tUse verbose output"
			echo -e "\t-f\tIP address of ${APP} to flash update"
			echo -e "\t-h\tDisplay usage and exit"
			echo -e "\t-c\tClean build directory before building"
			exit 0;;
		c)
			clean=true;;
		\?)
			echo "invalid option -$OPTARG" >&2
			exit 1;;
		:)
			echo "Option -$OPTARG requires an argument." >&2
			exit 1;;
		
	esac
done

if [[ "$OSTYPE" == "darwin"* ]]; then
	arduino="/Applications/Arduino.app/Contents/MacOS/Arduino"
	buildpath="$HOME/.build"
	espota="$HOME/Documents/Arduino/hardware/esp8266com/esp8266/tools/espota.py"
elif [[ "$OSTYPE" == "cygwin" ]]; then
	arduino="/c/Program Files (x86)/Arduino/arduino_debug.exe"
	buildpath="${USERPROFILE}\.build"
	espota="$HOME/Documents/Arduino/hardware/esp8266com/esp8266/tools/espota.py"
elif [[ "$OSTYPE" == "msys" ]]; then
	arduino="/c/Program Files (x86)/Arduino/arduino_debug.exe"
	buildpath="${USERPROFILE}\.build"
	espota="$HOME/Documents/Arduino/hardware/esp8266com/esp8266/tools/espota.py"
else
	arduino="`which arduino`"
	if [ -z "$arduino" ]; then
		arduino=`find ~/.local /usr -type f -name arduino -print -quit`
	fi
	buildpath="$HOME/.build"
	espota="$HOME/Arduino/hardware/esp8266com/esp8266/tools/espota.py"
fi

if [ -z "$arduino" ]; then
	echo "Arduino IDE must be installed"
	exit 1
else
	if [ "$clean" = true ]; then
		echo "Cleaning ${buildpath}..."
		"${RM}" ${verbose} --one-file-system -rf "${buildpath}"
	fi
	echo "Building ${APP} in ${buildpath}..."
	"${arduino}" ${verbose} --pref build.path="${buildpath}" --board "${board}" --save-prefs --verify ${SRC}
	ret=$?
	if [ $ret -eq 0 ] && [ -f "$espota" ] && [ ! -z "$FLASH" ]; then
		echo "Flashing ${BIN} to ${FLASH}..."
		"${espota}" ${debug} --progress --file="${buildpath}/${BIN}" --ip=${FLASH}
	else
		exit $ret
	fi
fi

