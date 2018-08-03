#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

#define double_buffer
#include <PxMatrix.h>
#include <gfxfont.h>
#include <Fonts/Picopixel.h>

#include <Ticker.h>
Ticker display_ticker;

#define P_LAT 16
#define P_A 5
#define P_B 4
#define P_C 15
#define P_D 12
#define P_E 0
#define P_OE 2

PxMATRIX display(64, 32, P_LAT, P_OE, P_A, P_B, P_C, P_D, P_E);

void display_updater() {
  display.display(70);
}

#include "Digit.h"
const byte row1 = 8;
const byte row2 = 15;
const byte row3 = 22;
const byte row4 = 29;
const uint16_t myRED = display.color565(255, 0, 0);
const uint16_t myGREEN = display.color565(0, 255, 0);
const uint16_t myBLUE = display.color565(0, 0, 255);
const uint16_t myWHITE = display.color565(255, 255, 255);
const uint16_t myYELLOW = display.color565(255, 255, 0);
const uint16_t myCYAN = display.color565(0, 255, 255);
const uint16_t myMAGENTA = display.color565(255, 0, 255);
const uint16_t myBLACK = display.color565(0, 0, 0);
const uint16_t myColor = myGREEN;
Digit digit0(&display, 0, 63 - 1 - 9 * 1, 8, myColor);
Digit digit1(&display, 0, 63 - 1 - 9 * 2, 8, myColor);
Digit digit2(&display, 0, 63 - 4 - 9 * 3, 8, myColor);
Digit digit3(&display, 0, 63 - 4 - 9 * 4, 8, myColor);
Digit digit4(&display, 0, 63 - 7 - 9 * 5, 8, myColor);
Digit digit5(&display, 0, 63 - 7 - 9 * 6, 8, myColor);

#endif
