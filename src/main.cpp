#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "buttons.h"
#include "config.h"
#include "display.h"
#include "led.h"
#include "timekeeping.h"
#include "webserver.h"
#include "wifi.h"
#include "debug.h"

// Callback for when the "UP" button is pressed
void upPressedCB() {
  setDisplayBrightness(getDisplayBrightness()+1);
  setInt8Config(CFG_DEFAULT_BRIGHTNESS, getDisplayBrightness());
}

// Callback for when the "DOWN" button is pressed
void downPressedCB() {
  setDisplayBrightness(getDisplayBrightness()-1);
  setInt8Config(CFG_DEFAULT_BRIGHTNESS, getDisplayBrightness());
}

// Callback for when the "SET" button is pressed
void setPressedCB() {}

void setup() {
#ifdef DEBUGGING
  Serial.begin(115200);
  do { delay(500); } while (!Serial);
#endif
  DEBUG("\n\nStarted - initialising LED\n")
  initRedLED();
  DEBUG("Init Config\n")
  initConfig();
  DEBUG("Init buttons\n")
  buttonSetup();
  if (buttonPressed(DOWN_BUTTON_PIN)) resetConfig();
  DEBUG("Init display\n")
  initDisplay(getInt8Config(CFG_DEFAULT_BRIGHTNESS, LED_DEFAULT_BRIGHTNESS));
  setLEDSegments(LED_CHAR_b, LED_CHAR_o, LED_CHAR_o, LED_CHAR_t);
  DEBUG("Init WiFi\n")
  initWiFi();
  DEBUG("Init Webserver\n")
  initWebserver();
}

void loop() {
  timekeepingPoll(); // This will initialise the timekeeping if/when we have a WiFi connection
  buttonScan();
  delay(20);
}
