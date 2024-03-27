// disp.cpp - driver for the 303WIFILC01 display/TM1650

#include <Arduino.h>
#include <Wire.h>
#include "display.h"
#include "config.h"

// The bitmap for lighting bits in the LED display goes as (big endian) b f a e  d c g dp
//
// The DP for the 2nd LED is actually wired to the colon
//
//   aaa
//  f   b
//  f   b
//   ggg
//  e   c
//  e   c
//   ddd  dp
//
// This translates to bit numbers -
//
//   555
//  6   7
//  6   7
//   111
//  4   2
//  4   2
//   333   0

static const uint8_t ledCharBitmaps[] = {
  0b11111100, // 0
  0b10000100, // 1
  0b10111010, // 2
  0b10101110, // 3
  0b11000110, // 4
  0b01101110, // 5
  0b01111110, // 6
  0b10100100, // 7
  0b11111110, // 8
  0b11101110, // 9
  0b00000000, // <space>
  0b01111000, // C
  0b00011110, // o
  0b00010110, // n
  0b01110010, // F
  0b01011110, // b - for "boot"
  0b01011010, // t - for "boot"
  0b11110010, // P - for "ntP" - replacing with "SynC"
  0b01101110, // S - for "SynC"
  0b11001110  // y
};

// The I2C connections on the 303WIFILC01 board
#define SCL_PIN 12
#define SDA_PIN 13

// Need to save the character of the 2nd LED so we can flash the colon easily
uint8_t digit2Char = LED_CHAR_SPACE;
uint8_t displayBrightness = LED_DEFAULT_BRIGHTNESS;

/**
 * Set the display brightness level (0 - 7)
 */
void setDisplayBrightness(int brightness) {
  int val;
  displayBrightness = brightness & 7;
  val = (((brightness+1) & 7) << 4) | 1;
  Wire.beginTransmission(0x24); // register 0x48 DIG1CTRL
  Wire.write(val); 
  Wire.endTransmission();
}

/**
 * Get the current display brightness level
 */
uint8_t getDisplayBrightness() {
  return displayBrightness;
}

/**
 * Initialise the display system
 */
void initDisplay(int brightness) {
  Wire.begin(SDA_PIN,SCL_PIN);
  setDisplayBrightness(brightness);
}

/**
 * Set a specific digit on the display
 */
void setDigit(uint8_t digitNum, uint8_t digitChar, bool digitDP) {
  if (digitNum == 1) digit2Char = digitChar;
  Wire.beginTransmission(0x34+digitNum);
  Wire.write(ledCharBitmaps[digitChar] | (digitDP ? 1 : 0));
  Wire.endTransmission();
}

/**
 * Set the colon on or off
 */
void setColon(bool colon) {
  setDigit(1, digit2Char, colon);
}

/**
 * Show characters on the display
 */
void showLEDChars(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, bool colon) {
  setDigit(0, dig1, false);
  setDigit(1, dig2, colon);
  setDigit(2, dig3, false);
  setDigit(3, dig4, false);
}

/**
 * Clear the display
 */
void clearLEDChars() {
  showLEDChars(LED_CHAR_SPACE, LED_CHAR_SPACE, LED_CHAR_SPACE, LED_CHAR_SPACE, false);
}

/**
 * Show the hours and minutes on the display
 */
void showTime(uint8_t h, uint8_t m) {
  if (h < 10) {
    setDigit(0, LED_CHAR_SPACE, false);
  } else if (h < 13) {
    setDigit(0, 1, false);
  } else if (cfgBitIsSet(CFG_MASK_24H)) {
    setDigit(0, (h > 19) ? 2 : 1, false);
  } else if (h < 22) {
    setDigit(0, LED_CHAR_SPACE, false);
  } else {
    setDigit(0, 1, false);
  }
  setDigit(1, h % 10, true);
  setDigit(2, m/10, false);
  setDigit(3, m % 10, false);
}

/**
 * Show an integer value on the display - used in showing our IP address
 */
void showUInt8(uint8_t v) {
  setDigit(0, LED_CHAR_SPACE, false);
  if (v < 100) {
    setDigit(1, LED_CHAR_SPACE, false);
  } else {
    setDigit(1, v/100, false);
  }
  if (v < 10) {
    setDigit(2, LED_CHAR_SPACE, false);
  } else {
    setDigit(2, (v/10)%10, false);
  }
  setDigit(3, v % 10, false);
}

