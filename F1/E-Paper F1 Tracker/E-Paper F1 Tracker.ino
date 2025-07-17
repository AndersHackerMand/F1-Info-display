/*

Formula 1 API to fetch info about drivers points, constructor points, 
last race with podium and next race



Damian Mazur 2025
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>  // https://arduinojson.org/?utm_source=meta&utm_medium=library.properties
#include <time.h>
#include <SPI.h>
#define ENABLE_GxEPD2_display 0
#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>              // https://github.com/ZinggJM/GxEPD2
#include <U8g2_for_Adafruit_GFX.h>  // https://github.com/olikraus/U8g2_for_Adafruit_GFX
#include <WiFiManager.h>            // https://github.com/tzapu/WiFiManager

#define AP_SSID "F1 display"
#define AP_PASS "formula1"
bool wifiConnected = false;
IPAddress ip;

String API_BASE;
String API_RACES;
String API_RESULTS;
String API_DRIVER_STAND;
String API_CONSTR_STAND;

#define SCREEN_WIDTH 296
#define SCREEN_HEIGHT 128
#define listx 225
#define logoWidth 80
#define logoHeight 20

enum alignmentType { LEFT,
                     RIGHT,
                     CENTER };

static const uint8_t EPD_BUSY = 4;   // to EPD BUSY
static const uint8_t EPD_CS = 5;     // to EPD CS
static const uint8_t EPD_RST = 16;   // to EPD RST
static const uint8_t EPD_DC = 17;    // to EPD DC
static const uint8_t EPD_SCK = 18;   // to EPD CLK
static const uint8_t EPD_MISO = 19;  // Master-In Slave-Out not used, as no data from display
static const uint8_t EPD_MOSI = 23;  // to EPD DIN

GxEPD2_3C<GxEPD2_290_C90c, GxEPD2_290_C90c::HEIGHT> display(GxEPD2_290_C90c(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));  // GDEM029C90 128xlistx, SSD1680

U8G2_FOR_ADAFRUIT_GFX gfx;

// Buffer for calendar + results + standings
StaticJsonDocument<32 * 1024> doc;

//#########################################################################################

// 80x20-pixel logo, monochrome
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

// stats
unsigned lastRound = 0, nextRound = 0;
String lastDate, lastName, lastCircuit, lastLoc;
String nextDate, nextName, nextCircuit, nextLoc;
//#########################################################################################

#define MY_TZ "GMT0BST,M3.5.0/1,M10.5.0/2"
time_t now;
struct tm timeinfo;

String strDateUSA, strDateUK, timeStr;
String abbrev0, abbrev1, abbrev2;
String fam0, fam1, fam2;
//#########################################################################################

void configModeCallback(WiFiManager* myWiFiManager) {}
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

void setup() {
  Serial.begin(115200);

  // Initialise Screen
  SPI.begin();
  display.init();
  display.setRotation(3);
  gfx.begin(display);
  gfx.setFontMode(1);
  gfx.setFontDirection(0);              // left to right (default)
  gfx.setForegroundColor(GxEPD_BLACK);  // Adafruit GFX color
  gfx.setBackgroundColor(GxEPD_WHITE);  // Adafruit GFX color
  gfx.setFont(u8g2_font_helvB10_tf);    // more fonts here: https://github.com/olikraus/u8g2/wiki/fntlistall
  display.fillScreen(GxEPD_WHITE);
  display.setFullWindow();

  //  splash screen
  display.fillScreen(GxEPD_WHITE);
  display.drawBitmap(SCREEN_WIDTH / 2 - logoWidth / 2, 50, F1_Logo, logoWidth, logoHeight, GxEPD_RED);
  drawString(40, 74, "It's Lights out and away we go!", LEFT);

  // Initialize WiFiManager
  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setCustomHeadElement(
    "<style>"
    "body { background: linear-gradient(135deg, #bdb7b7, #000); color: #E0E0E0; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; display: flex; flex-direction: column; align-items: center; justify-content: center; min-height: 100vh; margin: 0; padding: 20px; box-sizing: border-box; }"
    "h1, h2, h3, h4, h5, h6 { color: #4CAF50; }"
    ".container { background: rgba(30, 30, 30, 0.9); padding: 30px; border-radius: 12px; box-shadow: 0 8px 16px rgba(0, 0, 0, 0.5); max-width: 400px; width: 100%; }"
    "button { border-radius: 25px; padding: 12px 25px; margin: 8px 5px; background: linear-gradient(45deg, #007BFF, #00C4FF); color: #FFFFFF; font-size: 1.1em; font-weight: bold; border: none; cursor: pointer; transition: all 0.3s ease; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.3); }"
    "button:hover { background: linear-gradient(45deg, #0056b3, #0099CC); transform: translateY(-2px); box-shadow: 0 6px 12px rgba(0, 0, 0, 0.4); }"
    "input[type=\"text\"], input[type=\"password\"] { width: calc(100% - 20px); padding: 10px; margin: 8px 0; border-radius: 8px; border: 1px solid #555; background: #333; color: #EEE; font-size: 1em; }"
    "select { background: #333; color: #EEE; border-radius: 8px; padding: 10px; font-size: 1em; border: 1px solid #555; }" /* Styles for the select box itself */
    "option { background-color: #444; color: #FFF; }"                                                                      /* Styles for individual options in the dropdown */
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

  wifiManager.setTitle("Formula 1 Screen");

  if (!wifiManager.autoConnect(AP_SSID, AP_PASS)) {
    ESP.restart();
  }

  wifiConnected = true;
  ip = WiFi.localIP();

  configTzTime(MY_TZ, "pool.ntp.org", "time.nist.gov");
  while (!getLocalTime(&timeinfo)) {
    delay(1000);
  }

  // current season year for API
  int currentYear = timeinfo.tm_year + 1900;
  API_BASE = "https://api.jolpi.ca/ergast/f1/" + String(currentYear);
  API_RACES = API_BASE + "/races/";
  API_RESULTS = API_BASE + "/results/";
  API_DRIVER_STAND = API_BASE + "/driverstandings/";
  API_CONSTR_STAND = API_BASE + "/constructorstandings/";

  Serial.println("\nWi‑Fi connected, IP=" + WiFi.localIP().toString());
  display.display(false);
  display.fillScreen(GxEPD_WHITE);
}
//#########################################################################################

void loop() {

  if (getLocalTime(&timeinfo)) {
    DrawTime();
    FetchCalendar();
    DrawDrivers();
    DrawConstructors();
    DrawLastRace();
    DrawNextRace();
    if (nextRound != 0) {
      DrawPolePosition(nextRound);
    }
    display.display();
    RefreshTime();  //30 minutes between updates
  }
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
  //  drawString(SCREEN_WIDTH / 2, 0, "Updated: " + timeStr + "  " + strDateUK, CENTER);
  display.drawLine(0, 11, SCREEN_WIDTH, 11, GxEPD_RED);
}
//#########################################################################################

void FetchCalendar() {

  lastRound = 0;
  nextRound = 0;

  if (httpGetJson(API_RACES.c_str())) {

    JsonArray races = doc["MRData"]["RaceTable"]["Races"].as<JsonArray>();

    String today = strDateUSA;  // must be YYYY-MM-DD format to fetch dtaa

    for (JsonObject race : races) {
      String date = race["date"].as<const char*>();
      unsigned rnd = race["round"].as<unsigned>();
      String circuit = race["Circuit"]["circuitName"].as<const char*>();
      String loc = String(race["Circuit"]["Location"]["locality"].as<const char*>())
                   + ", " + race["Circuit"]["Location"]["country"].as<const char*>();

      if (date < today) {
        if (rnd > lastRound) {
          lastRound = rnd;
          lastDate = date;
          lastName = race["raceName"].as<const char*>();
          lastCircuit = circuit;
          lastLoc = loc;
        }
      } else if (nextRound == 0) {
        nextRound = rnd;
        nextDate = date;
        nextName = race["raceName"].as<const char*>();
        nextCircuit = circuit;
        nextLoc = loc;
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
    Serial.printf("  Round %u — %s on %s at %s\n", lastRound, lastCircuit.c_str(), lastDate.c_str(), lastLoc.c_str());

    String iso = lastDate;
    int p1 = iso.indexOf('-');
    int p2 = iso.indexOf('-', p1 + 1);

    String year = iso.substring(0, p1);        // yyyy
    String month = iso.substring(p1 + 1, p2);  // mm
    String day = iso.substring(p2 + 1);        // dd

    String displayDate = day + "-" + month + "-" + year;
    String lastInfo = lastLoc;
    drawString(SCREEN_WIDTH / 2, 118, lastInfo, CENTER);  // Last race location



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

  String quali = "Pole Pos: " + family + team;
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
  String nextInfo = "Next Race: " + nextLoc + ", " + displayDate;

  gfx.setForegroundColor(GxEPD_BLACK);
  gfx.setFont(u8g2_font_helvB08_tf);
  drawString(0, 14, nextInfo.c_str(), LEFT);
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

      // --- extract raw fields ---
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

      // just to make list better and more symetrical
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

void RefreshTime() {
  Serial.println("\n(Updating 30  minutes…)\n");
  //delay(10UL * 60UL * 1000UL);  // 10 mins
  delay(30UL * 60UL * 1000UL);  // 30 mins
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

void drawString(int x, int y, String text, alignmentType alignment) {
  int16_t x1, y1;  //the bounds of x,y and w and h of the variable 'text' in pixels.
  uint16_t w, h;
  display.setTextWrap(false);
  display.getTextBounds(text, x, y, &x1, &y1, &w, &h);
  if (alignment == RIGHT) x = x - w;
  if (alignment == CENTER) x = x - w / 2;
  gfx.setCursor(x, y + h);
  display.setTextColor(GxEPD_RED);

  gfx.print(text);
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

void InitialiseScreen() {
}
