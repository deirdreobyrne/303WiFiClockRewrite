#include <Arduino.h>
#include "buttons.h"

// Debounce period is 8 x BUTTON_SCAN_MILLIS
#define BUTTON_SCAN_MILLIS 20

uint8_t setButtonDebounce = 0;
uint8_t upButtonDebounce = 0;
uint8_t downButtonDebounce = 0;
uint8_t buttonsPressedMask = 0;
uint64_t lastButtonScan = 0UL;

/**
 * Low level "is a button pressed" check
 */
bool buttonPressed(uint8_t button) {
    if (button == DOWN_BUTTON_PIN) {
        return digitalRead(DOWN_BUTTON_PIN);
    }
    return !digitalRead(button);
}

/**
 * Check a button state
 * mask is the button's mask in buttonsPressedMask
 * debounceState is one of the ...ButtonDebounce variables
 * cb is the button's callback function
 */
void checkButton(uint8_t mask, uint8_t debounceState, void (*cb)(void)) {
    if (debounceState) {                    // Button has been pressed within at least the last 8 x BUTTON_SCAN_MILLIS
        if (!(buttonsPressedMask & mask)) { // If we have not already recorded that the button was pressed
            buttonsPressedMask |= mask;     // Record it
            (*cb)();                        // Call the callback
        }
    } else {                                // Button has been released for at least all of the last 8 x BUTTON_SCAN_MILLIS
        if (buttonsPressedMask & mask) {    // If we are saying that the button is pressed
            buttonsPressedMask &= ~mask;    // Record that it has actually been released
        }
    }
}

/**
 * Scan the buttons
 */
void buttonScan() {
    if ((millis() - lastButtonScan) < BUTTON_SCAN_MILLIS) return; // We only check once every BUTTON_SCAN_MILLIS
    lastButtonScan = millis();
    setButtonDebounce <<= 1;
    upButtonDebounce <<= 1;
    downButtonDebounce <<= 1;
    if (buttonPressed(SET_BUTTON_PIN)) setButtonDebounce |= 1;
    if (buttonPressed(UP_BUTTON_PIN)) upButtonDebounce |= 1;
    if (buttonPressed(DOWN_BUTTON_PIN)) downButtonDebounce |= 1;
    checkButton(1, setButtonDebounce, setPressedCB);
    checkButton(2, upButtonDebounce, upPressedCB);
    checkButton(4, downButtonDebounce, downPressedCB);
}

/**
 * Set up the system for checking the buttons
 */
void buttonSetup() {
    pinMode(SET_BUTTON_PIN, INPUT_PULLUP);
    pinMode(UP_BUTTON_PIN, INPUT_PULLUP);
    pinMode(DOWN_BUTTON_PIN, INPUT);
    lastButtonScan = millis();
}