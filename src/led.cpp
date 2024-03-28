// led.cpp - control the LED on the 303WIFILC01 board

#include <Arduino.h>
#include "led.h"

#define LED_PIN 2

/**
 * Initialise the LED on the back of the display
 */
void initRedLED() {
  pinMode(LED_PIN, OUTPUT);
  setRedLEDOff(); 
}

/**
 * Set the LED on the back of the display on
 */
void setRedLEDOn() {
  digitalWrite(LED_PIN, LOW); // active low
}

/**
 * Set the LED on the back of the display off
 */
void setRedLEDOff() {
  digitalWrite(LED_PIN, HIGH); // active low
}
