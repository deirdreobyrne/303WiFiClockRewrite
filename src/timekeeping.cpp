#include <Arduino.h>
#include "timekeeping.h"
#include "config.h"
#include "display.h"
#include "wifi.h"

static time_t lastDisplayUpdate = 0;
static unsigned long lastUpdateMillis = 0;
static bool colon = false;
static bool initialised = false;
static volatile bool hasTime = false;
const char *ntp1 = 0;
const char *ntp2 = 0;
const char *ntp3 = 0;

/**
 * Generate the string that describes our timezone and DST change information
 *
 * Ref: https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
 * 
 * Format is
 *
 * std offset dst [offset],start[/time],end[/time]
 * 
 * start and end are
 * Mm.w.d
 * 
 * This specifies day d of week w of month m. The day d must be between 0 (Sunday) and 6.
 * The week w must be between 1 and 5; week 1 is the first week in which day d occurs,
 * and week 5 specifies the last d day in the month. The month m should be between 1 and 12.
 */
String getTimezoneString() {
    String ans;
    int8_t i;
    if (hasConfig(CFG_TZ_NAME)) {
        ans.concat(getStringConfig(CFG_TZ_NAME));
    } else {
        ans.concat("UNK");
    }
    i = getInt8Config(CFG_TIMEZONE, 0);
    if (i < 0) {
        ans.concat('-');
        i = -i;
    } else if (i > 0) {
        ans.concat('+');
    }
    ans.concat(i);
    if (hasConfig(CFG_DST_NAME)) {
        String dstName = getStringConfig(CFG_DST_NAME);
        if (!dstName.isEmpty()) {
            DST_Transition transition;
            ans.concat(dstName);
            ans.concat(",M");
            getDSTTransition(true, &transition);
            ans.concat(transition.month+1);
            ans.concat('.');
            ans.concat(transition.dowNumber+1);
            ans.concat('.');
            ans.concat(transition.dow);
            ans.concat('/');
            ans.concat(transition.timeOfDay);
            ans.concat(",M");
            getDSTTransition(false, &transition);
            ans.concat(transition.month+1);
            ans.concat('.');
            ans.concat(transition.dowNumber+1);
            ans.concat('.');
            ans.concat(transition.dow);
            ans.concat('/');
            ans.concat(transition.timeOfDay);
        }
    }
    DEBUG("TZ is : \"%s\"\n", ans.c_str())
    return ans;
}

/**
 * Display the time on the LED
 */
void displayTime(time_t now) {
    struct tm *local = localtime(&now);
    showTime(local->tm_hour, local->tm_min);
    lastDisplayUpdate = now;
    lastUpdateMillis = millis();
    colon = true;
}

/**
 * Callback when settimeofday is called - by the NTP client - which tells us
 * when the NTP client has figured out the time
 */
void setTimeOfDayCB() {
    DEBUG("Set time of day being called\n")
    if (!hasTime) {
        lastDisplayUpdate = time(0); // Start showing the time at the start of the next second
        hasTime = true;
    }
}

// The pointers to the server names must remain valid at all times
void addNTPServer(const char *tag) {
    const char *server = getStringConfig(tag).c_str();
    char *storage;
    if (!(*server)) return;
    storage = (char*)malloc(strlen(server)+1);
    strcpy(storage, server);
    if (!ntp1) {
        ntp1=storage;
    } else if (!ntp2) {
        ntp2=storage;
    } else {
        ntp3=storage;
    }
}

/**
 * Initialise the timekeeping system
 */
void initTimekeeping() {
    DEBUG("Initialising the timekeeping system\n")
    setLEDSegments(LED_CHAR_S, LED_CHAR_y, LED_CHAR_n, LED_CHAR_C);
    settimeofday_cb(setTimeOfDayCB);
    addNTPServer(CFG_NTP_SERVER_1);
    addNTPServer(CFG_NTP_SERVER_2);
    addNTPServer(CFG_NTP_SERVER_3);
    configTime(getTimezoneString().c_str(), ntp1, ntp2, ntp3);
    initialised = true;
}

/**
 * Timekeeping polling loop
 */
void timekeepingPoll() {
    unsigned long timeSinceUpdateMillis;

    if (!hasTime) {
      if ((!initialised) && (hasWiFiConnection())) {
        initTimekeeping();
      }
      return;
    }
    timeSinceUpdateMillis = millis() - lastUpdateMillis;
    if (timeSinceUpdateMillis < 500) {
        return; // Nothing could have changed within 500ms of last update
    } else if (colon) {
        setColon(colon = false); // After 500 millis, turn the display colon off
    } else if (timeSinceUpdateMillis < 950) {
        return; // Nothing else can happen until just before the next second
    } else {
        time_t now = time(0);
        if (now != lastDisplayUpdate) displayTime(now);
    }
}
