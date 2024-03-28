#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "config.h"
#include "webserver.h"
#include "display.h"
#include "debug.h"

AsyncWebServer server(80);

/**
 * Callback called when the main web page is requested
 */
void onRoot(AsyncWebServerRequest *request);

/**
 * Processor for the tags in our web page. Tags are delimited by percent signs - see
 * the documentation for AsyncWebServer for more information
 */
String processor(const String& tag) {
  DEBUG("Web page - processing tag \"%s\"\n", tag.c_str())
  if (tag == "SSID") {
    return getStringConfig(CFG_SSID);
  } else if (tag == "Hostname") {
    return getStringConfig(CFG_HOSTNAME, String("303Clock"));
  } else if (tag == "NTP1") {
    return getStringConfig(CFG_NTP_SERVER_1, String("0.pool.ntp.org"));
  } else if (tag == "NTP2") {
    return getStringConfig(CFG_NTP_SERVER_2, String("1.pool.ntp.org"));
  } else if (tag == "NTP3") {
    return getStringConfig(CFG_NTP_SERVER_3, String("2.pool.ntp.org"));
  } else if (tag.startsWith("LED")) {
    int8_t ledLevel = tag.charAt(3)-'0';
    DEBUG("Led level %d\n", ledLevel)
    if (getInt8Config(CFG_DEFAULT_BRIGHTNESS, LED_DEFAULT_BRIGHTNESS) == ledLevel) {
      return String("checked");
    } else {
      return String();
    }
  } else if (tag == "CFG24H") {
    if (cfgBitIsSet(CFG_MASK_24H)) {
      return String("checked");
    } else {
      return String();
    }
  } else if (tag == "TZName") {
    return getStringConfig(CFG_TZ_NAME, String("GMT"));
  } else if (tag == "TZHrs") {
    return String(getInt8Config(CFG_TIMEZONE, 0));
  } else if (tag == "DSTName") {
    return getStringConfig(CFG_DST_NAME);
  } else if (tag.startsWith("DST")) {
    DST_Transition transition;
    int val;
    bool isStart = tag.charAt(3) == 'S';
    if (!hasConfig(isStart ? CFG_DST_START : CFG_DST_END)) return String();
    getDSTTransition(isStart, &transition);
    val = tag.substring(5).toInt();
    DEBUG("DST - isStart %d value %d selector character %c\n", isStart, val, tag.charAt(4))
    switch (tag.charAt(4)) {
      case 'W': return val == transition.dowNumber ? String("selected") : String();
      case 'D': return val == transition.dow ? String("selected") : String();
      case 'M': return val == transition.month ? String("selected") : String();
      case 'T': return val == transition.timeOfDay ? String("selected") : String();
      default: ;
    }
  }
  DEBUG("Could not find tag!\n")
  return String();
}

/**
 * Update a stored configuration string parameter
 */
void updateStringParam(AsyncWebServerRequest *request, const char *tag) {
  if (!request->hasParam(tag, true)) return;
  setStringConfig(tag, request->getParam(tag, true)->value());
}

/**
 * Update a stored configuration integer parameter
 */
void updateInt8Param(AsyncWebServerRequest *request, const char *tag) {
  if (!request->hasParam(tag, true)) return;
  setInt8Config(tag, request->getParam(tag, true)->value().toInt());
}

/**
 * Update a stored configuration DST change
 */
void updateDSTChangeParam(AsyncWebServerRequest *request, bool isStart) {
  char tag[6];
  DST_Transition transition;
  tag[0]='D'; tag[1]='S'; tag[2]='T'; tag[3]=isStart?'S':'E'; tag[5]=0;
  tag[4]='W';
  if (!request->hasParam(tag, true)) return;
  transition.dowNumber = request->getParam(tag, true)->value().toInt();
  tag[4] = 'D';
  if (!request->hasParam(tag, true)) return;
  transition.dow = request->getParam(tag, true)->value().toInt();
  tag[4] = 'M';
  if (!request->hasParam(tag, true)) return;
  transition.month = request->getParam(tag, true)->value().toInt();
  tag[4] = 'T';
  if (!request->hasParam(tag, true)) return;
  transition.timeOfDay = request->getParam(tag, true)->value().toInt();
  setDSTConfig(transition, isStart);
}

/**
 * Callback when the /brightness URL is called
 */
void onBrightness(AsyncWebServerRequest *request) {
    if (request->params() == 1) {
        int brightness = request->getParam(0)->value().toInt();
        setDisplayBrightness(brightness+1);
        setInt8Config(CFG_DEFAULT_BRIGHTNESS, getDisplayBrightness());
    }
    request->send(200, "text/plain", "OK");
}

/**
 * Initialise the webserver system
 */
void initWebserver() {
    server.on("/", HTTP_GET|HTTP_POST, onRoot);
    server.on("/brightness", HTTP_GET|HTTP_POST, onBrightness);
    server.begin();
}

/**
 * The web page with which the user can change the stored configuration
 */
const char homePageData[] PROGMEM = "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n\
<html>\n\
<head>\n\
<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n\
<title>Clock configure</title>\n\
</head>\n\
<script>\n\
function setLEDBrightness(level) {\n\
var req = new XMLHttpRequest();\n\
var body = \"brightness=\".concat(level.toString());\n\
req.open(\"POST\",\"/brightness\");\n\
req.setRequestHeader(\"Content-Type\",\"application/x-www-form-urlencoded\");\n\
req.send(body);\n\
}\n\
</script>\n\
<body vlink=\"#9999ff\" text=\"#ffffff\" link=\"#33ccff\" bgcolor=\"#666666\"\n\
alink=\"#ff6666\">\n\
<h1 align=\"center\">Clock configure</h1>\n\
<form method=\"post\">\n\
<table width=\"100%%\" cellspacing=\"2\" cellpadding=\"2\" border=\"0\">\n\
<tbody>\n\
<tr>\n\
<td valign=\"top\" align=\"right\" width=\"50%%\">WiFi SSID:<br>\n\
</td>\n\
<td valign=\"top\" width=\"50%%\"><input name=\"SSID\" value=\"%SSID%\"><br>\n\
</td>\n\
</tr>\n\
<tr>\n\
<td valign=\"top\" align=\"right\">WiFi Password:<br>\n\
</td>\n\
<td valign=\"top\"><input name=\"PW\" type=\"password\"><br>\n\
</td>\n\
</tr>\n\
<tr>\n\
<td valign=\"top\" align=\"right\">Hostname:<br>\n\
</td>\n\
<td valign=\"top\"><input name=\"HOST\" value=\"%Hostname%\"><br>\n\
</td>\n\
</tr>\n\
<tr>\n\
<td valign=\"top\" align=\"right\"><br>\n\
</td>\n\
<td valign=\"top\"><br>\n\
</td>\n\
</tr>\n\
<tr>\n\
<td width=\"50%%\" valign=\"top\" align=\"right\">NTP Source 1:<br>\n\
</td>\n\
<td valign=\"top\"><input name=\"NTP1\" value=\"%NTP1%\"><br>\n\
</td>\n\
</tr>\n\
<tr>\n\
<td width=\"50%%\" valign=\"top\" align=\"right\">NTP Source 2:<br>\n\
</td>\n\
<td valign=\"top\"><input name=\"NTP2\" value=\"%NTP2%\"><br>\n\
</td>\n\
</tr>\n\
<tr>\n\
<td valign=\"top\" align=\"right\">NTP Source 3:<br>\n\
</td>\n\
<td valign=\"top\"><input name=\"NTP3\" value=\"%NTP3%\"><br>\n\
</td>\n\
</tr>\n\
<tr>\n\
<td><br>\n\
</td>\n\
<td><br>\n\
</td>\n\
</tr>\n\
<tr>\n\
<td valign=\"top\" align=\"right\">Digit Brightness:<br>\n\
</td>\n\
<td valign=\"top\">Dark\n\
<input name=\"BRI\" value=\"0\" type=\"radio\" %LED0% onclick=\"setLEDBrightness(0)\"/>\n\
<input name=\"BRI\" value=\"1\" type=\"radio\" %LED1% onclick=\"setLEDBrightness(1)\"/>\n\
<input name=\"BRI\" value=\"2\" type=\"radio\" %LED2% onclick=\"setLEDBrightness(2)\"/>\n\
<input name=\"BRI\" value=\"3\" type=\"radio\" %LED3% onclick=\"setLEDBrightness(3)\"/>\n\
<input name=\"BRI\" value=\"4\" type=\"radio\" %LED4% onclick=\"setLEDBrightness(4)\"/>\n\
<input name=\"BRI\" value=\"5\" type=\"radio\" %LED5% onclick=\"setLEDBrightness(5)\"/>\n\
<input name=\"BRI\" value=\"6\" type=\"radio\" %LED6% onclick=\"setLEDBrightness(6)\"/>\n\
<input name=\"BRI\" value=\"7\" type=\"radio\" %LED7% onclick=\"setLEDBrightness(7)\"/>\n\
Bright </td>\n\
</tr>\n\
<tr>\n\
<td valign=\"top\" align=\"right\"><br>\n\
</td>\n\
<td valign=\"top\"><br>\n\
</td>\n\
</tr>\n\
<tr>\n\
<td valign=\"top\" align=\"right\">24 Hour clock\n\
</td>\n\
<td valign=\"top\"><input type=\"checkbox\" name=\"24h\" %CFG24H% />\n\
</td>\n\
</tr>\n\
<tr>\n\
<td valign=\"top\" align=\"right\"><br>\n\
</td>\n\
<td valign=\"top\"><br>\n\
</td>\n\
</tr>\n\
<tr>\n\
<td valign=\"top\" align=\"right\">Timezone name:<br>\n\
</td>\n\
<td valign=\"top\"><input name=\"TZNAM\" value=\"%TZName%\"><br>\n\
</td>\n\
</tr>\n\
<tr>\n\
<td valign=\"top\" align=\"right\">Hours ahead (east) of\n\
Greenwich:<br>\n\
</td>\n\
<td valign=\"top\"><input name=\"TZHrs\" value=\"%TZHrs%\"><br>\n\
</td>\n\
</tr>\n\
<tr>\n\
<td valign=\"top\" align=\"right\">Daylight saving timezone\n\
name:<br>\n\
</td>\n\
<td valign=\"top\"><input name=\"DSTNAM\" value=\"%DSTName%\"> <i>(leave blank for\n\
no daylight saving)</i><br>\n\
</td>\n\
</tr>\n\
<tr>\n\
<td valign=\"top\" align=\"right\">Daylight saving start:<br>\n\
</td>\n\
<td valign=\"top\">\n\
<select name=\"DSTSW\">\n\
<option value=\"0\" %DSTSW0%>First</option>\n\
<option value=\"1\" %DSTSW1%>Second</option>\n\
<option value=\"2\" %DSTSW2%>Third</option>\n\
<option value=\"3\" %DSTSW3%>Fourth</option>\n\
<option value=\"4\" %DSTSW4%>Last</option>\n\
</select>\n\
<select name=\"DSTSD\">\n\
<option value=\"0\" %DSTSD0%>Sunday</option>\n\
<option value=\"1\" %DSTSD1%>Monday</option>\n\
<option value=\"2\" %DSTSD2%>Tuesday</option>\n\
<option value=\"3\" %DSTSD3%>Wednesday</option>\n\
<option value=\"4\" %DSTSD4%>Thursday</option>\n\
<option value=\"5\" %DSTSD5%>Friday</option>\n\
<option value=\"6\" %DSTSD6%>Saturday</option>\n\
</select>\n\
&nbsp;of&nbsp;\n\
<select name=\"DSTSM\">\n\
<option value=\"0\" %DSTSM0%>January</option>\n\
<option value=\"1\" %DSTSM1%>February</option>\n\
<option value=\"2\" %DSTSM2%>March</option>\n\
<option value=\"3\" %DSTSM3%>April</option>\n\
<option value=\"4\" %DSTSM4%>May</option>\n\
<option value=\"5\" %DSTSM5%>June</option>\n\
<option value=\"6\" %DSTSM6%>July</option>\n\
<option value=\"7\" %DSTSM7%>August</option>\n\
<option value=\"8\" %DSTSM8%>September</option>\n\
<option value=\"9\" %DSTSM9%>October</option>\n\
<option value=\"10\" %DSTSM10%>November</option>\n\
<option value=\"11\" %DSTSM11%>December</option>\n\
</select>\n\
&nbsp;at&nbsp;\n\
<select name=\"DSTST\">\n\
<option value=\"0\" %DSTST0%>00:00</option>\n\
<option value=\"1\" %DSTST1%>01:00</option>\n\
<option value=\"2\" %DSTST2%>02:00</option>\n\
<option value=\"3\" %DSTST3%>03:00</option>\n\
<option value=\"4\" %DSTST4%>04:00</option>\n\
<option value=\"5\" %DSTST5%>05:00</option>\n\
<option value=\"6\" %DSTST6%>06:00</option>\n\
<option value=\"7\" %DSTST7%>07:00</option>\n\
<option value=\"8\" %DSTST8%>08:00</option>\n\
<option value=\"9\" %DSTST9%>09:00</option>\n\
<option value=\"10\" %DSTST10%>10:00</option>\n\
<option value=\"11\" %DSTST11%>11:00</option>\n\
<option value=\"12\" %DSTST12%>12:00</option>\n\
<option value=\"13\" %DSTST13%>13:00</option>\n\
<option value=\"14\" %DSTST14%>14:00</option>\n\
<option value=\"15\" %DSTST15%>15:00</option>\n\
<option value=\"16\" %DSTST16%>16:00</option>\n\
<option value=\"17\" %DSTST17%>17:00</option>\n\
<option value=\"18\" %DSTST18%>18:00</option>\n\
<option value=\"19\" %DSTST19%>19:00</option>\n\
<option value=\"20\" %DSTST20%>20:00</option>\n\
<option value=\"21\" %DSTST21%>21:00</option>\n\
<option value=\"22\" %DSTST22%>22:00</option>\n\
<option value=\"23\" %DSTST23%>23:00</option>\n\
</select>\n\
<br>\n\
</td>\n\
</tr>\n\
<tr>\n\
<td valign=\"top\" align=\"right\">Daylight saving end:<br>\n\
</td>\n\
<td valign=\"top\">\n\
<select name=\"DSTEW\">\n\
<option value=\"0\" %DSTEW0%>First</option>\n\
<option value=\"1\" %DSTEW1%>Second</option>\n\
<option value=\"2\" %DSTEW2%>Third</option>\n\
<option value=\"3\" %DSTEW3%>Fourth</option>\n\
<option value=\"4\" %DSTEW4%>Last</option>\n\
</select>\n\
<select name=\"DSTED\">\n\
<option value=\"0\" %DSTED0%>Sunday</option>\n\
<option value=\"1\" %DSTED1%>Monday</option>\n\
<option value=\"2\" %DSTED2%>Tuesday</option>\n\
<option value=\"3\" %DSTED3%>Wednesday</option>\n\
<option value=\"4\" %DSTED4%>Thursday</option>\n\
<option value=\"5\" %DSTED5%>Friday</option>\n\
<option value=\"6\" %DSTED6%>Saturday</option>\n\
</select>\n\
&nbsp;of&nbsp;\n\
<select name=\"DSTEM\">\n\
<option value=\"0\" %DSTEM0%>January</option>\n\
<option value=\"1\" %DSTEM1%>February</option>\n\
<option value=\"2\" %DSTEM2%>March</option>\n\
<option value=\"3\" %DSTEM3%>April</option>\n\
<option value=\"4\" %DSTEM4%>May</option>\n\
<option value=\"5\" %DSTEM5%>June</option>\n\
<option value=\"6\" %DSTEM6%>July</option>\n\
<option value=\"7\" %DSTEM7%>August</option>\n\
<option value=\"8\" %DSTEM8%>September</option>\n\
<option value=\"9\" %DSTEM9%>October</option>\n\
<option value=\"10\" %DSTEM10%>November</option>\n\
<option value=\"11\" %DSTEM11%>December</option>\n\
</select>\n\
&nbsp;at&nbsp;\n\
<select name=\"DSTET\">\n\
<option value=\"0\" %DSTET0%>00:00</option>\n\
<option value=\"1\" %DSTET1%>01:00</option>\n\
<option value=\"2\" %DSTET2%>02:00</option>\n\
<option value=\"3\" %DSTET3%>03:00</option>\n\
<option value=\"4\" %DSTET4%>04:00</option>\n\
<option value=\"5\" %DSTET5%>05:00</option>\n\
<option value=\"6\" %DSTET6%>06:00</option>\n\
<option value=\"7\" %DSTET7%>07:00</option>\n\
<option value=\"8\" %DSTET8%>08:00</option>\n\
<option value=\"9\" %DSTET9%>09:00</option>\n\
<option value=\"10\" %DSTET10%>10:00</option>\n\
<option value=\"11\" %DSTET11%>11:00</option>\n\
<option value=\"12\" %DSTET12%>12:00</option>\n\
<option value=\"13\" %DSTET13%>13:00</option>\n\
<option value=\"14\" %DSTET14%>14:00</option>\n\
<option value=\"15\" %DSTET15%>15:00</option>\n\
<option value=\"16\" %DSTET16%>16:00</option>\n\
<option value=\"17\" %DSTET17%>17:00</option>\n\
<option value=\"18\" %DSTET18%>18:00</option>\n\
<option value=\"19\" %DSTET19%>19:00</option>\n\
<option value=\"20\" %DSTET20%>20:00</option>\n\
<option value=\"21\" %DSTET21%>21:00</option>\n\
<option value=\"22\" %DSTET22%>22:00</option>\n\
<option value=\"23\" %DSTET23%>23:00</option>\n\
</select>\n\
<br>\n\
</td>\n\
</tr>\n\
</tbody>\n\
</table>\n\
<p align=\"center\"><input type=\"submit\" name=\"submit\" value=\" Save \" />&nbsp;\n\
<input type=\"reset\" name=\"Reset\" value=\" Reset \" /></p>\n\
</form>\n\
</body>\n\
</html>\n\
";

void onRoot(AsyncWebServerRequest *request) {
    if (request->params() > 1) { // It was called with a new configuration
      DEBUG("Form submission received\n")
      updateStringParam(request, CFG_SSID);
      // Only update the password field if a password has been provided
      if (request->hasParam(CFG_PASSWORD, true)) {
        String val = request->getParam(CFG_PASSWORD, true)->value();
        if (!val.isEmpty()) {
          setStringConfig(CFG_PASSWORD, val);
        }
      }
      updateStringParam(request, CFG_HOSTNAME);
      updateStringParam(request, CFG_NTP_SERVER_1);
      updateStringParam(request, CFG_NTP_SERVER_2);
      updateStringParam(request, CFG_NTP_SERVER_3);
      updateInt8Param(request, CFG_DEFAULT_BRIGHTNESS);
      updateInt8Param(request, CFG_TIMEZONE);
      updateStringParam(request, CFG_TZ_NAME);
      updateStringParam(request, CFG_DST_NAME);
      updateDSTChangeParam(request, true);
      updateDSTChangeParam(request, false);
      if (request->hasArg("24h")) {
        setCfgBit(CFG_MASK_24H);
      } else {
        clearCfgBit(CFG_MASK_24H);
      }
      request->send(200, "text/plain", "Need to restart the clock!");
    } else {
      DEBUG("Sending home page\n")
      request->send_P(200, "text/html", homePageData, processor);
    }
}
