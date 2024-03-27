#include <Arduino.h>
#include <Preferences.h>
#include "config.h"
#include "debug.h"

Preferences prefs;
int8_t cfgBits = 0;

/**
 * Initialise the stored configuration system
 */
void initConfig() {
    prefs.begin("303Clock");
    cfgBits = getInt8Config(CFG_BOOL_CONFIGS, 0);
}

/**
 * Is a certain stored configuration parameter present?
 */
bool hasConfig(const char *tag) {
#ifdef DEBUGGING
    bool ans = prefs.isKey(tag);
    DEBUG("Has config \"%s\"? %d\n", tag, ans)
    return ans;
#else
    return prefs.isKey(tag);
#endif
}

/**
 * Get an integer value from the stored configuration
 */
int8_t getInt8Config(const char *tag, int8_t defaultValue) {
#ifdef DEBUGGING
    int8_t ans = prefs.getChar(tag, defaultValue);
    DEBUG("\"%s\" = %d\n", tag, ans);
    return ans;
#else
    return prefs.getChar(tag, defaultValue);
#endif
}

/**
 * Get a string value from the stored configuration
 */
String getStringConfig(const char *tag, String defaultValue) {
#ifdef DEBUGGING
    String ans = prefs.getString(tag, defaultValue);
    DEBUG("\"%s\" = \"%s\"\n", tag, ans.c_str());
    return ans;
#else
    return prefs.getString(tag, defaultValue);
#endif
}

/**
 * Get a DST change ruleset from the stored configuration
 */
void getDSTTransition(bool start, DST_Transition *ans) {
    prefs.getBytes(start ? CFG_DST_START : CFG_DST_END, ans, sizeof(DST_Transition));
    DEBUG("DST %s transition is dow %u wk %u mon %u tm %u\n", start ? "start" : "end", ans->dow, ans->dowNumber, ans->month, ans->timeOfDay)
}

/**
 * Store an integer value in the stored configuration
 */
void setInt8Config(const char *tag, int8_t value) {
    if ((prefs.isKey(tag)) && (prefs.getChar(tag) == value)) return;
    DEBUG("Setting \"%s\" to %d\n", tag, value)
    prefs.putChar(tag, value);
}

/**
 * Store a string value in the stored configuration
 */
void setStringConfig(const char *tag, String value) {
    if ((prefs.isKey(tag)) && (prefs.getString(tag) == value)) return;
    DEBUG("Setting \"%s\" to \"%s\"\n", tag, value.c_str())
    prefs.putString(tag, value);
}

/**
 * Store a DST change ruleset in the stored configuration
 */
void setDSTConfig(DST_Transition value, bool start) {
    const char *tag = start ? CFG_DST_START : CFG_DST_END;
    if (hasConfig(tag)) {
        DST_Transition transition;
        prefs.getBytes(tag, &transition, sizeof(DST_Transition));
        if ((transition.dow == value.dow) && (transition.dowNumber == value.dowNumber) && (transition.month == value.month) && (transition.timeOfDay == value.timeOfDay)) return;
    }
    DEBUG("Setting %s transition to dow %u wk %u mon %u tm %u\n", tag, value.dow, value.dowNumber, value.month, value.timeOfDay)
    prefs.putBytes(tag, &value, sizeof(DST_Transition));
}

/**
 * Reset the entire configuration
 */
void resetConfig() {
    DEBUG("Resetting config and rebooting")
    prefs.clear();
    // TODO NEED TO GET ESP REBOOTING WORKING!
    ESP.restart();
}

/**
 * Is one of the boolean stored configuration values set?
 */
bool cfgBitIsSet(uint8_t mask) {
    return cfgBits & mask;
}

/**
 * Set one of the boolean stored configuration bits
 */
void setCfgBit(uint8_t mask) {
    cfgBits |= mask;
    setInt8Config(CFG_BOOL_CONFIGS, cfgBits);
}

/**
 * Clear one of the boolean stored configuration bits
 */
void clearCfgBit(uint8_t mask) {
    cfgBits &= ~(mask);
    setInt8Config(CFG_BOOL_CONFIGS, cfgBits);
}
