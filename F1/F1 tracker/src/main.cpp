/*
#####################################################################

      Copyright (C) 2025 Mazur888

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
  ####################################################################

Screeen ISP Connections:
BUSY = 4
CS = 5
RST = 16
DC = 17
SCK = 18
MISO = 19
MOSI = 23

Time zone is set to BST time in UK.
more timezones here:
MET-1METDST,M3.5.0/01,M10.5.0/02 //Europe
CET-1CEST,M3.5.0,M10.5.0/3 // Central Europe
EST-2METDST,M3.5.0/01,M10.5.0/02 // Europe
EST5EDT,M3.2.0,M11.1.0 // EST USA
CST6CDT,M3.2.0,M11.1.0 // CST USA
MST7MDT,M4.1.0,M10.5.0 // MST USA
NZST-12NZDT,M9.5.0,M4.1.0/3 // Auckland
EET-2EEST,M3.5.5/0,M10.5.5/0 // Asia
ACST-9:30ACDT,M10.1.0,M4.1.0/3 // Australia

mDNS implemented: go to f1tracker.local or device IP
*/

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> //v7 // https://arduinojson.org/?utm_source=meta&utm_medium=library.properties
#include <time.h>
#include <SPI.h>
#define ENABLE_GxEPD2_display 0
#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>              // https://github.com/ZinggJM/GxEPD2
#include <U8g2_for_Adafruit_GFX.h>  // https://github.com/olikraus/U8g2_for_Adafruit_GFX
#include <WiFiManager.h>            // https://github.com/tzapu/WiFiManager
#include <Update.h>
#include <HTTPClient.h>
#include "OTA_Page.h"
#include "Dashboard_Page.h"
#include "esp_wifi.h"   // for esp_wifi_get_config()

#define AP_SSID "F1 tracker"
#define AP_PASS "formula1"
#define HTTP_TIMEOUT_MS 6000
bool wifiConnected = false;
bool serverStarted = false;      // tracks if we started our own web server
bool allowDeepSleep = false;     // set true when we want low-power “one-shot then sleep”
IPAddress ip;

String API_BASE;
String API_RACES;
String API_RESULTS;
String API_DRIVER_STAND;
String API_CONSTR_STAND;
String lastTime = "";   // stores the last calendar time string shown

#define SCREEN_WIDTH 296
#define SCREEN_HEIGHT 128
#define listx 225
#define logoWidth 80
#define logoHeight 20


enum screenAlignment { LEFT, RIGHT, CENTER };

static const uint8_t EPDBUSY = 4;   // BUSY
static const uint8_t EPDCS = 5;     // CS
static const uint8_t EPDRST = 16;   // RST
static const uint8_t EPDDC = 17;    // DC
static const uint8_t EPDSCK = 18;   // CLK
static const uint8_t EPDMISO = 19;  // Not used with this display
static const uint8_t EPDMOSI = 23;  // DIN

GxEPD2_3C<GxEPD2_290_C90c, GxEPD2_290_C90c::HEIGHT> display(GxEPD2_290_C90c(EPDCS, EPDDC, EPDRST, EPDBUSY));  // GDEM029C90, SSD1680

U8G2_FOR_ADAFRUIT_GFX gfx;

JsonDocument  doc;

//#########################################################################################

// 80x20-pixel F1 logo
const uint8_t F1_Logo[] PROGMEM = {
  0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0x8f, 0xfe, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff,
  0xff, 0xff, 0x1f, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x3f, 0xf8, 0x00, 0x00,
  0x3f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xf0, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xf8,
  0xff, 0xe0, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf1, 0xff, 0xc0, 0x00, 0x03, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xe3, 0xff, 0x80, 0x00, 0x07, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x07, 0xff, 0x00,
  0x00, 0x0f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xfe, 0x00, 0x00, 0x1f, 0xfc, 0x3f, 0xff, 0xff,
  0xff, 0x1f, 0xfc, 0x00, 0x00, 0x3f, 0xf8, 0xff, 0xff, 0xff, 0xfe, 0x3f, 0xf8, 0x00, 0x00, 0x7f,
  0xf1, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xf0, 0x00, 0x00, 0xff, 0xe3, 0xff, 0xff, 0xff, 0xf8, 0xff,
  0xe0, 0x00, 0x01, 0xff, 0xc7, 0xff, 0xff, 0xff, 0xf1, 0xff, 0xc0, 0x00, 0x03, 0xff, 0x8f, 0xff,
  0xff, 0xff, 0xe3, 0xff, 0x80, 0x00, 0x07, 0xff, 0x3f, 0xf8, 0x00, 0x00, 0x07, 0xff, 0x00, 0x00,
  0x0f, 0xfe, 0x7f, 0xe0, 0x00, 0x00, 0x0f, 0xfe, 0x00, 0x00, 0x1f, 0xfc, 0xff, 0xc0, 0x00, 0x00,
  0x1f, 0xfc, 0x00, 0x00, 0x3f, 0xf9, 0xff, 0x80, 0x00, 0x00, 0x3f, 0xf8, 0x00, 0x00, 0x7f, 0xf1,
  0xff, 0x00, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00
};
//#########################################################################################

unsigned lastRound = 0, nextRound = 0;
String lastDate, lastName, lastCircuit, lastLoc;
String nextDate, nextName, nextCircuit, nextLoc;
String lastGP, nextGP;
String nextTime; //countdown string
// Local-time strings for next race & qualifying (derived from UTC in schedule)
String nextRaceLocal;
String nextQualiLocal;
//#########################################################################################

#define MY_TZ "CET-1CEST,M3.5.0,M10.5.0/3"  // Change time zone if you are outside CET
#define NTP1 "pool.ntp.org"
#define NTP2 "time.nist.gov"

WebServer server(80);
Preferences preferences;

const unsigned long UPDATE_INTERVAL = 60UL * 60UL * 1000UL;  // 60 minutes
unsigned long lastUpdate = 0;

time_t now;
struct tm timeinfo;

String strDateUSA, strDateUK, timeStr;
String abbrev0, abbrev1, abbrev2;
String fam0, fam1, fam2;
//#########################################################################################

void configModeCallback(WiFiManager* myWiFiManager) {
}
//#########################################################################################

// Only display first 3 letters of each last name (needs done this way because of Hulkenbergs umlaut)
String utf8_substr(const String& s, int codepoints) {
  const char* p = s.c_str();
  String out;
  int seen = 0;
  while (*p && seen < codepoints) {
    uint8_t c = *p;
    int bytes = 1;
    if ((c & 0xF8) == 0xF0) bytes = 4;       // 4-byte UTF‑8
    else if ((c & 0xF0) == 0xE0) bytes = 3;  // 3-byte
    else if ((c & 0xE0) == 0xC0) bytes = 2;  // 2-byte
    for (int i = 0; i < bytes; i++) {
      out += *p++;
    }
    seen++;
  }
  return out;
}
//#########################################################################################
bool httpGetJson(const char* url) {
  doc.clear();
  HTTPClient http;
  http.begin(url);
  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    Serial.printf("HTTP %d for %s\n", code, url);
    http.end();
    return false;
  }
  DeserializationError err = deserializeJson(doc, http.getStream());
  http.end();
  if (err) {
    Serial.printf("JSON parse failed: %s\n", err.f_str());
    return false;
  }
  return true;
}
//#########################################################################################

void handleOTAUpload() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    preferences.begin("ota", false);
    preferences.putString("lastFirmware", upload.filename);
    preferences.end();
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
      server.send(500, "text/plain", "Update failed to start.");
      return;
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
      server.send(500, "text/plain", "Update failed to write data.");
      return;
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      server.sendHeader("Connection", "close");
      server.sendHeader("Content-Length", "0");
      server.send(200, "text/plain", "Update successful. Rebooting...");
      delay(1000);
      ESP.restart();
    } else {
      Update.printError(Serial);
      server.send(500, "text/plain", "Update failed to finalize.");
    }
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    Update.abort();
    server.send(500, "text/plain", "Update aborted.");
  }
}
//#########################################################################################
void drawString(int x, int y, String str, screenAlignment alignment) {
  int16_t x1, y1;
  uint16_t w, h;
  display.setTextWrap(false);
  display.getTextBounds(str, x, y, &x1, &y1, &w, &h);
  if (alignment == RIGHT) x = x - w;
  if (alignment == CENTER) x = x - w / 2;
  gfx.setCursor(x, y + h);
  display.setTextColor(GxEPD_RED);
  gfx.print(str);
}
//#########################################################################################



void DrawTime() {

  char bufTime[16];
  char bufDateUSA[16];
  char bufDateUK[16];

  // HH:MM
  snprintf(bufTime, sizeof(bufTime), "%02d:%02d",
           timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

  // YYYY-MM-DD american format
  snprintf(bufDateUSA, sizeof(bufDateUSA), "%04d-%02d-%02d",
           timeinfo.tm_year + 1900,
           timeinfo.tm_mon + 1,
           timeinfo.tm_mday);

  // DD-MM-YYYY european format
  snprintf(bufDateUK, sizeof(bufDateUK), "%02d-%02d-%04d",
           timeinfo.tm_mday,
           timeinfo.tm_mon + 1,
           timeinfo.tm_year + 1900);

  timeStr = String(bufTime);
  strDateUSA = String(bufDateUSA);
  strDateUK = String(bufDateUK);


  Serial.println(timeStr);
  Serial.println(strDateUSA);

  gfx.setFont(u8g2_font_helvB08_tf);
  drawString(SCREEN_WIDTH / 2, 0, "Today: " + strDateUK, CENTER);
  //  drawString(SCREEN_WIDTH / 2, 0, "Updated: " + timeStr + "  " + strDateUK, CENTER);  // Top header with time
  display.drawLine(0, 11, SCREEN_WIDTH, 11, GxEPD_RED);
}
//#########################################################################################

static time_t timegm_utc(struct tm* tm) {
  int year  = tm->tm_year + 1900;
  int month = tm->tm_mon + 1;   // tm_mon is 0‑11

  // move Jan/Feb to end of previous year  
  if (month <= 2) {
    year  -= 1;
    month += 12;
  }

  // days since 1970‑01‑01:
  // 365*y + leap days + days by month + day of month - offset
  int64_t days = 365LL * year
               + year/4   - year/100 + year/400
               + (153*(month-3) + 2)/5
               + tm->tm_mday
               - 719469;   // 1970‑01‑01 is day number 719469 since year 0

  return days * 86400
       + tm->tm_hour * 3600
       + tm->tm_min  * 60
       + tm->tm_sec;
}

// Return true if an SSID is stored in NVS for STA
static bool hasSavedWifiCredentials() {
  WiFi.mode(WIFI_STA); // make sure Wi-Fi is initialized
  wifi_config_t conf{};
  esp_err_t err = esp_wifi_get_config(WIFI_IF_STA, &conf);
  if (err != ESP_OK) {
    Serial.printf("esp_wifi_get_config err=%d\n", (int)err);
    return false;
  }
  size_t ssid_len = strnlen((const char*)conf.sta.ssid, sizeof(conf.sta.ssid));
  size_t pass_len = strnlen((const char*)conf.sta.password, sizeof(conf.sta.password));
  if (ssid_len) {
    Serial.printf("Saved SSID found: '%s' (pass %s)\n",
                  (const char*)conf.sta.ssid,
                  pass_len ? "present" : "empty");
  } else {
    Serial.println("No saved SSID.");
  }
  return ssid_len > 0;
}




// Format "YYYY-MM-DD" + "HH:MM:SSZ" (UTC) into local time, e.g. "Sun 12 Oct 15:00"
static String formatLocalFromUtc(const char* dateStr, const char* timeStr) {
  if (!dateStr || !timeStr) return "";

  // Parse date
  int Y=0,M=0,D=0, h=0,m=0,s=0;
  if (sscanf(dateStr, "%4d-%2d-%2d", &Y,&M,&D) != 3) return "";
  // Parse time (ignore trailing 'Z')
  if (sscanf(timeStr, "%2d:%2d:%2d", &h,&m,&s) != 3) return "";

  struct tm tm_utc = {};
  tm_utc.tm_year = Y - 1900;
  tm_utc.tm_mon  = M - 1;
  tm_utc.tm_mday = D;
  tm_utc.tm_hour = h;
  tm_utc.tm_min  = m;
  tm_utc.tm_sec  = s;
  tm_utc.tm_isdst = 0;

  time_t epoch_utc = timegm_utc(&tm_utc);
  struct tm loc;
  localtime_r(&epoch_utc, &loc);               // apply MY_TZ (set in setup)

  char out[32];
  // Full: "Sun 12 Oct 15:00"
  strftime(out, sizeof(out), "%a %d %b %H:%M", &loc);
  return String(out);
}
// Format to "dd-mm HH:MM"
static String formatCompactLocal(const char* dateStr, const char* timeStr) {
  if (!dateStr || !timeStr) return "";

  int Y=0,M=0,D=0,h=0,m=0,s=0;
  if (sscanf(dateStr, "%4d-%2d-%2d", &Y,&M,&D) != 3) return "";
  if (sscanf(timeStr, "%2d:%2d:%2d", &h,&m,&s) != 3) return "";

  struct tm tm_utc = {};
  tm_utc.tm_year = Y - 1900;
  tm_utc.tm_mon  = M - 1;
  tm_utc.tm_mday = D;
  tm_utc.tm_hour = h;
  tm_utc.tm_min  = m;
  tm_utc.tm_sec  = s;

  time_t epoch_utc = timegm_utc(&tm_utc);
  struct tm loc;
  localtime_r(&epoch_utc, &loc);

  char out[32];
  strftime(out, sizeof(out), "%d-%m %H:%M", &loc);
  return String(out);
}

//#########################################################################################

void FetchCalendar() {
  lastRound = nextRound = 0;

  if (!httpGetJson(API_RACES.c_str())) return;

  JsonArray races = doc["MRData"]["RaceTable"]["Races"].as<JsonArray>();
  time_t now = time(nullptr);

  for (JsonObject race : races) {
    // 1) read API fields
    const char* dateStr = race["date"].as<const char*>();  // "2025-07-19"
    const char* timeStr = race["time"].as<const char*>();  // "14:00:00Z"
    unsigned   rnd     = race["round"].as<unsigned>();
    String     circuit = race["Circuit"]["circuitName"].as<const char*>();
    String     GPname  = race["raceName"].as<const char*>();
    String     loc     = String(race["Circuit"]["Location"]["locality"].as<const char*>())
                        + ", " + race["Circuit"]["Location"]["country"].as<const char*>();

    // 2) parse into tm (UTC)
    struct tm tmRace = {};
    // parse date
    if (sscanf(dateStr, "%4d-%2d-%2d",
               &tmRace.tm_year, &tmRace.tm_mon, &tmRace.tm_mday) != 3) continue;
    tmRace.tm_year -= 1900;
    tmRace.tm_mon  -= 1;
    // parse time (drop trailing 'Z')
    int h,m,s;
    if (sscanf(timeStr, "%2d:%2d:%2d", &h, &m, &s) != 3) continue;
    tmRace.tm_hour = h;
    tmRace.tm_min  = m;
    tmRace.tm_sec  = s;
    tmRace.tm_isdst = 0;

    // 3) convert to epoch (UTC)
    time_t raceEpoch = timegm_utc(&tmRace);

    // 4) decide past vs future
    if (raceEpoch < now) {
      // this race is over
      if (rnd > lastRound) {
        lastRound   = rnd;
        lastDate    = dateStr;
        lastTime    = timeStr;
        lastName    = GPname;
        lastCircuit = circuit;
        lastLoc     = loc;
        lastGP      = GPname;
        lastGP.replace("Grand Prix","GP");
      }
    }
    else if (nextRound == 0) {
      // first future race
      nextRound   = rnd;
      nextDate    = dateStr;
      nextTime    = timeStr;
      nextName    = GPname;
      nextCircuit = circuit;
      nextLoc     = loc;
      nextGP      = GPname;
      nextGP.replace("Grand Prix","GP");

      // Build local-time strings for display
      nextRaceLocal  = formatCompactLocal(nextDate.c_str(), nextTime.c_str());

      const char* qDate = race["Qualifying"]["date"].as<const char*>();
      const char* qTime = race["Qualifying"]["time"].as<const char*>();
      if (qDate && qTime) {
        nextQualiLocal = formatCompactLocal(qDate, qTime);
      } else {
        nextQualiLocal = "";
      }
    }
  }
}

//#########################################################################################

void DrawLastRace() {
  String resultsUrl = String(API_BASE)
                      + "/" + String(lastRound)
                      + "/results/";

  Serial.println("\n=== Last Race ===");
  if (lastRound == 0) {
    Serial.println("  (no past races yet)");
  } else {
    Serial.printf("  Round %u — %s on %s at %s\n", lastRound, lastCircuit.c_str(), lastDate.c_str(), lastLoc.c_str(), lastGP.c_str());

    String iso = lastDate;
    int p1 = iso.indexOf('-');
    int p2 = iso.indexOf('-', p1 + 1);

    String year = iso.substring(0, p1);        // yyyy
    String month = iso.substring(p1 + 1, p2);  // mm
    String day = iso.substring(p2 + 1);        // dd

    String displayDate = day + "-" + month + "-" + year;
    String lastInfo = lastGP;

    drawString(SCREEN_WIDTH / 2, 118, lastInfo.c_str(), CENTER);  // Last race location



    if (httpGetJson(resultsUrl.c_str())) {
      JsonArray podium = doc["MRData"]["RaceTable"]["Races"][0]
                            ["Results"]
                              .as<JsonArray>();

      if (podium.size() >= 3) {
        String fam0 = String(podium[0]["Driver"]["familyName"].as<const char*>());
        String fam1 = String(podium[1]["Driver"]["familyName"].as<const char*>());
        String fam2 = String(podium[2]["Driver"]["familyName"].as<const char*>());

        abbrev0 = utf8_substr(fam0, 3);  // 1st place
        abbrev1 = utf8_substr(fam1, 3);  // 2nd place
        abbrev2 = utf8_substr(fam2, 3);  // 3rd place

        Serial.printf("P1: %s, P2: %s, P3: %s\n",
                      abbrev0.c_str(),
                      abbrev1.c_str(),
                      abbrev2.c_str());
      }
    }
  }

  // F1 Logo on main screen
  display.drawBitmap(SCREEN_WIDTH / 2 - logoWidth / 2, 40, F1_Logo, logoWidth, logoHeight, GxEPD_RED);

  //podium box:
  display.drawRect(133, 86, 30, 30, GxEPD_BLACK);   // 1st place box
  display.drawRect(104, 95, 30, 21, GxEPD_BLACK);   // 2nd place box
  display.drawRect(162, 104, 30, 12, GxEPD_BLACK);  // 3rd place box

  gfx.setForegroundColor(GxEPD_RED);
  drawString(SCREEN_WIDTH / 2 - 1, 77, abbrev0.c_str(), CENTER);  // 1st place name
  drawString(SCREEN_WIDTH / 2 - 38, 86, abbrev1.c_str(), LEFT);   // 2nd place name
  drawString(SCREEN_WIDTH / 2 + 20, 95, abbrev2.c_str(), LEFT);   // 3rd place name
}
//#########################################################################################

void DrawPolePosition(unsigned round) {
  String url = API_BASE
               + "/" + String(round)
               + "/qualifying/";

  if (!httpGetJson(url.c_str())) {
    Serial.println("Failed to fetch qualifying data");
    return;
  }

  JsonObject race = doc["MRData"]["RaceTable"]["Races"][0];
  JsonArray quals = race["QualifyingResults"].as<JsonArray>();
  if (quals.size() == 0) {
    Serial.println("No qualifying data yet");
    drawString(0, 24, "Pole Pos: No data yet", LEFT);
    return;
  }

  JsonObject pole = quals[0];
  String given = pole["Driver"]["givenName"].as<const char*>();
  String family = pole["Driver"]["familyName"].as<const char*>();
  String team = pole["Constructor"]["name"].as<const char*>();

  Serial.printf("Pole (Round %u): %s %s — %s\n",
                round,
                given.c_str(), family.c_str(),
                team.c_str());

  String quali = "Pole Pos: "+ given + " " + family;
  drawString(0, 24, quali.c_str(), LEFT);
}

//#########################################################################################

void DrawNextRace() {
  Serial.println("\n=== Next Race ===");
  if (nextRound == 0) {
    Serial.println("  (season complete)");
  } else {
    Serial.printf("  Round %u — %s on %s at %s\n",
                  nextRound, nextCircuit.c_str(),
                  nextDate.c_str(), nextLoc.c_str());
  }

  //converted date from yyyy-mm-dd to dd-mm-yyyy for screen
  String iso = nextDate;
  int p1 = iso.indexOf('-');
  int p2 = iso.indexOf('-', p1 + 1);

  String year = iso.substring(0, p1);        // year
  String month = iso.substring(p1 + 1, p2);  // month
  String day = iso.substring(p2 + 1);        // day

String displayDate = day + "-" + month + "-" + year;  // dd-mm-yyyy

// If we have a local race start time, show it right after the date
String nextInfo = "Next: " + nextGP;
if (nextRaceLocal.length()) {
  nextInfo += ", " + nextRaceLocal;  // e.g. "05/10 14:00"
}

gfx.setForegroundColor(GxEPD_BLACK);
gfx.setFont(u8g2_font_helvB08_tf);
drawString(0, 14, nextInfo.c_str(), LEFT);

// Show quali one line below if available
if (nextQualiLocal.length()) {
  drawString(0, 34, String("Quali: " + nextQualiLocal), LEFT);
}
}
//#########################################################################################

void DrawDrivers() {
  httpGetJson(API_DRIVER_STAND.c_str());
  {
    Serial.println("\n=== Driver Standings (Top 10) ===");
    JsonArray ds = doc["MRData"]["StandingsTable"]["StandingsLists"][0]
                      ["DriverStandings"]
                        .as<JsonArray>();
    String driverLine[10];
    String pointsLine[10];


    for (int i = 0; i < 10 && i < ds.size(); i++) {
      JsonObject d = ds[i];

      String pos = String(d["position"].as<const char*>());
      String full = String(d["Driver"]["familyName"].as<const char*>());
      String pts = String(d["points"].as<const char*>());
      String wins = String(d["wins"].as<const char*>());

      String abbr = utf8_substr(full, 3);

      String drvline = "#" + pos
                       + " " + abbr;
      //            + " - " + pts
      //  + " - (" + wins + "w)";
      String ptsline = " - " + pts;

      driverLine[i] = drvline;
      pointsLine[i] = ptsline;

      Serial.println(drvline + " - " + ptsline);
      gfx.setForegroundColor(GxEPD_RED);
      drawString(223, 13, "Top 10 Drivers", LEFT);
      gfx.setForegroundColor(GxEPD_BLACK);

      drawString(listx, 24, driverLine[0].c_str(), LEFT);
      drawString(listx, 34, driverLine[1].c_str(), LEFT);
      drawString(listx, 44, driverLine[2].c_str(), LEFT);
      drawString(listx, 54, driverLine[3].c_str(), LEFT);
      drawString(listx, 64, driverLine[4].c_str(), LEFT);
      drawString(listx, 74, driverLine[5].c_str(), LEFT);
      drawString(listx, 84, driverLine[6].c_str(), LEFT);
      drawString(listx, 94, driverLine[7].c_str(), LEFT);
      drawString(listx, 104, driverLine[8].c_str(), LEFT);
      drawString(listx, 114, driverLine[9].c_str(), LEFT);

      drawString(296, 24, pointsLine[0].c_str(), RIGHT);
      drawString(296, 34, pointsLine[1].c_str(), RIGHT);
      drawString(296, 44, pointsLine[2].c_str(), RIGHT);
      drawString(296, 54, pointsLine[3].c_str(), RIGHT);
      drawString(296, 64, pointsLine[4].c_str(), RIGHT);
      drawString(296, 74, pointsLine[5].c_str(), RIGHT);
      drawString(296, 84, pointsLine[6].c_str(), RIGHT);
      drawString(296, 94, pointsLine[7].c_str(), RIGHT);
      drawString(296, 104, pointsLine[8].c_str(), RIGHT);
      drawString(296, 114, pointsLine[9].c_str(), RIGHT);
    }
  }
}
//#########################################################################################

void DrawConstructors() {

  if (httpGetJson(API_CONSTR_STAND.c_str())) {
    Serial.println("\n=== Constructor Standings (Top 10) ===");
    JsonArray cs = doc["MRData"]["StandingsTable"]["StandingsLists"][0]
                      ["ConstructorStandings"]
                        .as<JsonArray>();
    String constrLines[10];

    for (int i = 0; i < 10 && i < cs.size(); i++) {
      JsonObject c = cs[i];

      String pos = String(c["position"].as<const char*>());
      String constr = String(c["Constructor"]["name"].as<const char*>());
      String pts = String(c["points"].as<const char*>());
      String wins = String(c["wins"].as<const char*>());

      String constrLine = "#" + pos
                          + " " + constr
                          + " - " + pts;

      constrLines[i] = constrLine;
      gfx.setForegroundColor(GxEPD_RED);
      drawString(0, 64, "Top 5 Teams:", LEFT);
      gfx.setForegroundColor(GxEPD_BLACK);

      drawString(0, 74, constrLines[0].c_str(), LEFT);
      drawString(0, 84, constrLines[1].c_str(), LEFT);
      drawString(0, 94, constrLines[2].c_str(), LEFT);
      drawString(0, 104, constrLines[3].c_str(), LEFT);
      drawString(0, 114, constrLines[4].c_str(), LEFT);

      Serial.printf("  #%s %s — %sp (%s wins)\n",
                    c["Constructor"]["name"].as<const char*>(),
                    c["points"].as<const char*>(),
                    c["wins"].as<const char*>());
    }
  }
}
//#########################################################################################

static void runUpdateOnce() {
  // Clear the RAM buffer before drawing the new frame
  display.fillScreen(GxEPD_WHITE);
  // display.setFullWindow(); // optional; set once in setup is fine

  if (getLocalTime(&timeinfo)) {
    DrawTime();
    FetchCalendar();
    DrawDrivers();
    DrawConstructors();
    DrawLastRace();
    DrawNextRace();
    if (nextRound != 0) DrawPolePosition(nextRound);
    display.display(); // push clean frame to panel
  }
}
//#########################################################################################
void setup() {
  Serial.begin(115200);

  // Screen Init
  SPI.begin();
  display.init();
  display.setRotation(3);
  gfx.begin(display);
  gfx.setFontMode(0); //fix for text ghosting
  gfx.setFontDirection(0);
  gfx.setForegroundColor(GxEPD_BLACK);
  gfx.setBackgroundColor(GxEPD_WHITE);
  gfx.setFont(u8g2_font_helvB10_tf);
  display.fillScreen(GxEPD_WHITE);
  display.setFullWindow();


  // ---------- Wi-Fi / server: only start AP+server if no saved Wi-Fi ----------
  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setCustomHeadElement(
    "<style>"
    "body { background: repeating-linear-gradient(45deg, #ffffff 0px, #ffffff 8px, #ffffff 8px, #ffffff 16px), linear-gradient(135deg, #DDDDDD, #E1EEBC); color: #F1F1F1; font-family: 'Orbitron', 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; display: flex; flex-direction: column; align-items: center; justify-content: center; min-height: 100vh; margin: 0; padding: 20px; box-sizing: border-box; }"
    "h1, h2, h3, h4, h5, h6 { color: #7a100eff; text-transform: uppercase; letter-spacing: 1px; }"
    ".container { background: rgba(0, 0, 0, 0.85); padding: 30px; border: 3px solid #E10600; border-radius: 8px; box-shadow: 0 10px 20px rgba(0,0,0,0.7); max-width: 420px; width: 100%; }"
    "input[type=\"text\"], input[type=\"password\"], select { width: calc(100% - 20px); padding: 12px; margin: 10px 0; border-radius: 4px; border: 2px solid #444; background: #111; color: #EEE; font-size: 1em; font-family: inherit; transition: border-color 0.2s; }"
    "input[type=\"text\"]:focus, input[type=\"password\"]:focus, select:focus { border-color: #E10600; outline: none; }"
    "option { background-color: #222; color: #FFF; }"
    "button { display: inline-block; border-radius: 25px; padding: 14px 28px; margin: 10px 5px; background: linear-gradient(90deg, #E10600, #FF4F4F); color: #FFF; font-size: 1.1em; font-weight: bold; font-family: inherit; border: none; cursor: pointer; transition: transform 0.2s ease, box-shadow 0.2s ease; box-shadow: 0 6px 12px rgba(0, 0, 0, 0.5); text-transform: uppercase; letter-spacing: 1px; }"
    "button:hover { transform: translateY(-3px); box-shadow: 0 8px 16px rgba(0, 0, 0, 0.6); }"
    "button:active { transform: translateY(-1px); box-shadow: 0 4px 8px rgba(0, 0, 0, 0.6); }"
    "div#footer, .msg { display: none !important; }"
    "</style>"
    "<script>"
    "window.addEventListener('load', function() {"
    "let buttons = document.querySelectorAll('button');"
    "buttons.forEach(btn => {"
    "if (btn.textContent.includes('Info') || btn.textContent.includes('Update') || btn.textContent.includes('Exit')) {"
    "btn.style.display = 'none';"
    "}"
    "});"
    "let custom = document.createElement('div');"
    "custom.innerHTML = '<p style=\"margin-top:20px; color:#4CAF50; font-size:1em;\">Please connect to your WiFi</p>';"
    "document.body.appendChild(custom);"
    "});"
    "</script>");
  wifiManager.setTitle("Formula 1 Tracker");

  bool startServer = false;

  // Try silent STA connect if creds exist
  WiFi.mode(WIFI_STA);           // ensure Wi-Fi is initialized in STA mode
  WiFi.persistent(true);         // keep creds in NVS
  if (hasSavedWifiCredentials()) {
    Serial.println("Saved Wi-Fi found — connecting silently (no AP, no server)...");
    WiFi.begin(); // uses stored credentials
    unsigned long t0 = millis();
    const unsigned long TIMEOUT = 10000; // 10s
    while (WiFi.status() != WL_CONNECTED && (millis() - t0) < TIMEOUT) {
      delay(200);
    }
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      ip = WiFi.localIP();
      Serial.println("Connected. IP=" + ip.toString());
      // keep startServer=false to reduce power
    } else {
      Serial.println("Silent connect failed — starting AP for configuration.");
      if (!wifiManager.autoConnect(AP_SSID, AP_PASS)) {
        ESP.restart();
      }
      wifiConnected = true;
      ip = WiFi.localIP();
      startServer = false; // keep our server OFF to save power
    }
  } else {
    Serial.println("No saved Wi-Fi — starting AP for configuration.");
    if (!wifiManager.autoConnect(AP_SSID, AP_PASS)) {
      ESP.restart();
    }
    wifiConnected = true;
    ip = WiFi.localIP();
    startServer = false;   // still OFF; WiFiManager’s portal was enough
  }

  // If we decided to run the server (only in config/AP or fallback), start mDNS + routes
  if (startServer) {
    if (!MDNS.begin("f1tracker")) {
      Serial.println("Error setting up mDNS responder!");
    } else {
      Serial.println("mDNS responder started: http://f1tracker.local");
      MDNS.addService("http", "tcp", 80);
    }

    server.on("/", HTTP_GET, handleOTAUpdatePage);
    server.on("/api", HTTP_GET, handleF1Page);
    server.on("/update", HTTP_POST,
      [](){
        server.sendHeader("Connection", "close");
        server.send(200, "text/plain", "Update successful, rebooting");
        delay(1000);
        ESP.restart();
      },
      handleOTAUpload
    );

    server.begin();
    serverStarted = true;          // <— track it
    allowDeepSleep = false;        // web mode => keep running (no sleep)
    Serial.println("Web server started.");
  } else {
    serverStarted = false;
    allowDeepSleep = true;         // normal mode => do one-shot + sleep
    Serial.println("Web server not started (power-save mode).");
  }
// Show splash only if we're staying awake (web/config mode)
if (!allowDeepSleep) {
  display.fillScreen(GxEPD_WHITE);
  display.drawBitmap(SCREEN_WIDTH / 2 - logoWidth / 2, 50, F1_Logo, logoWidth, logoHeight, GxEPD_RED);
  drawString(40, 74, "It's Lights out and away we go!", LEFT);
  display.display(false);  // full refresh for splash
}

  // Common post-Wi-Fi init (runs either way)
  configTzTime(MY_TZ, NTP1, NTP2);
  while (!getLocalTime(&timeinfo)) { delay(1000); }

  int currentYear = timeinfo.tm_year + 1900; // current season year for API
  API_BASE = "https://api.jolpi.ca/ergast/f1/" + String(currentYear);
  API_RACES = API_BASE + "/races/";
  API_RESULTS = API_BASE + "/results/";
  API_DRIVER_STAND = API_BASE + "/driverstandings/";
  API_CONSTR_STAND = API_BASE + "/constructorstandings/";

  Serial.println("\nWi-Fi connected, IP=" + WiFi.localIP().toString());
  // ---------- Low-power behavior ----------
if (allowDeepSleep) {
  Serial.println("Running one-shot update before deep sleep...");
  runUpdateOnce();

  // Put the e-paper into low-power mode (GxEPD2)
  display.hibernate();      // or display.powerOff() on some panels

  // Shut down Wi-Fi cleanly
  WiFi.disconnect(true);    // true => erase connection state (keeps creds in NVS)
  WiFi.mode(WIFI_OFF);

  // Sleep for 60 minutes
  const uint64_t uS_PER_MIN = 60ULL * 1000000ULL;
  esp_sleep_enable_timer_wakeup(60ULL * uS_PER_MIN);
  Serial.println("Entering deep sleep for 60 minutes...");
  delay(100); // let serial flush
  esp_deep_sleep_start();

  // (never returns)
}

// ---------- end Wi-Fi / server handling ----------

  lastUpdate = millis() - UPDATE_INTERVAL;

}
//#########################################################################################

void loop() {
  if (serverStarted) {
    server.handleClient();   // only when web server is active
    // (You can keep your periodic refresh here if you want live updates in web mode)
    unsigned long now = millis();
    if (now - lastUpdate >= UPDATE_INTERVAL) {
      lastUpdate = now;
      runUpdateOnce();
    }
  } else {
    // In deep-sleep mode we never reach here (we slept in setup()).
    // If we do, just idle briefly.
    delay(1000);
  }
}
//#########################################################################################
