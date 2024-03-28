#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "wifi.h"
#include "config.h"
#include "led.h"
#include "display.h"
#include "debug.h"

static bool softAP = false;

/**
 * Set up the ESP as a wifi access point
 */
void initAP() {
    IPAddress local_IP(192,168,200,1);
    IPAddress gateway(192,168,200,1);
    IPAddress subnet(255,255,255,0);
    setRedLEDOn(); // indicates that we are a wifi access point
    WiFi.softAPConfig(local_IP, gateway, subnet);
    WiFi.softAP("303Clock");
    setLEDSegments(LED_CHAR_C, LED_CHAR_o, LED_CHAR_n, LED_CHAR_F);
    softAP = true;
}

/**
 * Initialise the wifi system
 */
void initWiFi() {
    if (hasConfig(CFG_SSID)) {
        WiFi.mode(WIFI_STA);
        if (hasConfig(CFG_HOSTNAME)) WiFi.setHostname(getStringConfig(CFG_HOSTNAME).c_str());
        if (hasConfig(CFG_PASSWORD)) {
            WiFi.begin(getStringConfig(CFG_SSID), getStringConfig(CFG_PASSWORD));
        } else {
            WiFi.begin(getStringConfig(CFG_SSID));
        }
        for (int i = 0 ; i < 20 ; i++) {
            delay(500);
            if (WiFi.isConnected()) {
                IPAddress addr = WiFi.localIP();
                // Flash our IP address on the LED display
                for (int j = 0 ; j < 4 ; j++) {
                    clearLEDSegments();
                    delay(200);
                    showUInt8(addr[j]);
                    delay(1000);
                }
                clearLEDSegments();
                delay(200);
                return;
            }
        }
        DEBUG("Setting WiFi AP and STA mode\n")
        WiFi.mode(WIFI_AP_STA);
    } else {
      DEBUG("Setting WiFi AP mode")
      WiFi.mode(WIFI_AP);
    }
    initAP();
}

/**
 * Do we have a wifi connection to an access point?
 * 
 * This is called by timekeepingPoll() whenever the timekeeping system doesn't yet have the time from an NTP server
 */
bool hasWiFiConnection() {
  if (softAP) {
    if (WiFi.isConnected()) {
      WiFi.enableAP(softAP = false);
      setRedLEDOff();
      DEBUG("Disabling WiFi soft AP\n");
      return true;
    }
    return false;
  }
  return WiFi.isConnected();
}
