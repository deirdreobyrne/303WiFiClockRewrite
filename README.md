# Clock firmware for the 303 WIFI LC 01 clock

This is a rewrite of [Maarten Pennings firmware](https://github.com/maarten-pennings/303WIFILC01) for the 303 WiFi LC 01 clock.

## Changes

- Easier web-based configuration
- Over-the-air firmware upgrade
- The brightness level of the LED display survives reboot
- Got rid of everything that isn't just a basic clock
- Ported to PlatformIO on VS-Code
- General re-write to suit my own coding style

## Using the firmware

First, follow the instructions Maarten has for [flashing the firmware](https://github.com/maarten-pennings/303WIFILC01/blob/main/3-flash/readme.md).

Upon reboot, the clock will say "boot", followed by "ConF". When "ConF" appears, the clock is broadcasting an access point on SSID "303Clock", with no password.

Connect to the access point, and load http://192.168.200.1/ to configure the clock.

