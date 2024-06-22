#include <Arduino.h>
#include <ArduinoOTA.h>
#include "ota.h"
#include "wifi.h"
#include "config.h"

static bool isSetup = false;

void otaPoll() {
    if (isSetup) {
        ArduinoOTA.handle();
    } else if (hasWiFiConnection()) {
        if (hasConfig(CFG_HOSTNAME)) {
            ArduinoOTA.setHostname(getStringConfig(CFG_HOSTNAME).c_str());
        }
        ArduinoOTA.begin();
        isSetup = true;
    }
}