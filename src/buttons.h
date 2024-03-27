#ifndef _BUTTONS_H_
#define _BUTTONS_H_

#define SET_BUTTON_PIN 0
#define UP_BUTTON_PIN 4
#define DOWN_BUTTON_PIN 15

void upPressedCB();
void downPressedCB();
void setPressedCB();

void buttonScan();
void buttonSetup();
bool buttonPressed(uint8_t button);
#endif
