#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <Arduino.h>

#define LED_CHAR_SPACE 10
#define LED_CHAR_C 11
#define LED_CHAR_o 12
#define LED_CHAR_n 13
#define LED_CHAR_F 14
#define LED_CHAR_b 15
#define LED_CHAR_t 16
#define LED_CHAR_P 17
#define LED_CHAR_S 18
#define LED_CHAR_y 19
#define LED_DEFAULT_BRIGHTNESS 7

void setDisplayBrightness(int brightness);       // Sets brightness level 0 .. 7
uint8_t getDisplayBrightness();
void initDisplay(int brightness);
void setColon(bool colon);
void setLEDSegments(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, bool colon = false);
void clearLEDSegments();
void showTime(uint8_t h, uint8_t m);
void showUInt8(uint8_t v);

#endif
