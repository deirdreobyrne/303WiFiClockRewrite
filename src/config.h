#ifndef _EEPROM_H_
#define _EEPROM_H_
#include <Arduino.h>

#define CFG_MASK_24H 1
//#define CFG_MASK_MONTHNAMES 2
//#define CFG_MASK_OTA_UPDATE 4

#define CFG_SSID "SSID"
#define CFG_PASSWORD "PW"
#define CFG_HOSTNAME "HOST"
#define CFG_NTP_SERVER_1 "NTP1"
#define CFG_NTP_SERVER_2 "NTP2"
#define CFG_NTP_SERVER_3 "NTP3"
#define CFG_BOOL_CONFIGS "CFG"
#define CFG_DEFAULT_BRIGHTNESS "BRI"
#define CFG_TIMEZONE "TZ"
#define CFG_DST_START "DSTS"
#define CFG_DST_END "DSTE"
#define CFG_TZ_NAME "TZNAM"
#define CFG_DST_NAME "DSTNAM"

typedef struct DST_Transition_t {
    uint8_t dow;
    uint8_t dowNumber;
    uint8_t month;
    uint8_t timeOfDay;
} DST_Transition;

void initConfig();
bool hasConfig(const char *tag);
int8_t getInt8Config(const char *tag, int8_t defaultValue);
String getStringConfig(const char *tag, String defaultValue = String());
void getDSTTransition(bool start, DST_Transition *ans);
void setInt8Config(const char *tag, int8_t value);
void setStringConfig(const char *tag, String value);
void setDSTConfig(DST_Transition value, bool start);
void resetConfig();

bool cfgBitIsSet(uint8_t mask);
void setCfgBit(uint8_t mask);
void clearCfgBit(uint8_t mask);

#endif
