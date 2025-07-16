/*

API calls only for serial debugging
season 2025
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>  // v6 via Library Manager
#include <time.h>


const char* ssid = "SSID";
const char* password = "PASS";

// ——— Jolpi Ergast API endpoints ———
const char* API_BASE = "https://api.jolpi.ca/ergast/f1/2025";
const char* API_RACES = "https://api.jolpi.ca/ergast/f1/2025/races/";
const char* API_RESULTS = "https://api.jolpi.ca/ergast/f1/2025/results/";
const char* API_DRIVER_STAND = "https://api.jolpi.ca/ergast/f1/2025/driverstandings/";
const char* API_CONSTR_STAND = "https://api.jolpi.ca/ergast/f1/2025/constructorstandings/";

// Buffer for calendar + results + standings
StaticJsonDocument<32 * 1024> doc;

// stats
unsigned lastRound = 0, nextRound = 0;
String lastDate, lastName, lastCircuit, lastLoc;
String nextDate, nextName, nextCircuit, nextLoc;
//#########################################################################################

#define MY_TZ "GMT0BST,M3.5.0/1,M10.5.0/2"
time_t now;
struct tm timeinfo;

String dateStr, timeStr;
//#########################################################################################

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting Wi‑Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print('.');
  }
  // Initialize time
  configTzTime(MY_TZ, "pool.ntp.org", "time.nist.gov");
  while (!getLocalTime(&timeinfo)) {
    delay(1000);
  }
  Serial.println("\nWi‑Fi connected, IP=" + WiFi.localIP().toString());
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
    RefreshTime();  //10 minutes between updates
  }
}
//#########################################################################################

void DrawTime() {

  char bufTime[16];
  char bufDate[16];

  snprintf(bufTime, sizeof(bufTime), "%02d:%02d:%02d",
           timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  snprintf(bufDate, sizeof(bufDate), "%04d-%02d-%02d",
           timeinfo.tm_year + 1900,
           timeinfo.tm_mon + 1,
           timeinfo.tm_mday);

  //u8g2.drawStr(0, 12 + y, timeStr);

  timeStr = String(bufTime);
  dateStr = String(bufDate);

  Serial.println(timeStr);
  Serial.println(dateStr);
}
//#########################################################################################

void FetchCalendar() {

  lastRound = 0;
  nextRound = 0;
  // Race calendar
  if (!httpGetJson(API_RACES)) return;
  JsonArray races = doc["MRData"]["RaceTable"]["Races"].as<JsonArray>();

  //String today = "2025-07-15"; // YYYY-MM-DD format
  String today = dateStr;  // YYYY-MM-DD format dateStr


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
//#########################################################################################

void DrawLastRace() {
  String resultsUrl = String(API_BASE)
                      + "/" + String(lastRound)
                      + "/results/";

  Serial.println("\n=== Last Race ===");
  if (lastRound == 0) {
    Serial.println("  (no past races yet)");
  } else {
    Serial.printf("  Round %u — %s on %s at %s\n",
                  lastRound, lastCircuit.c_str(),
                  lastDate.c_str(), lastLoc.c_str());

    // Fetch *all* results, then pick out the one with round==lastRound
    if (httpGetJson(resultsUrl.c_str())) {
      // now doc["MRData"]["RaceTable"]["Races"][0]["Results"] is *that* race’s results
      JsonArray podium = doc["MRData"]["RaceTable"]["Races"][0]
                            ["Results"]
                              .as<JsonArray>();

      Serial.println("  Podium:");
      for (int i = 0; i < 3 && i < podium.size(); i++) {
        JsonObject P = podium[i];
        Serial.printf("    %d) %s %s — %sp\n",
                      i + 1,
                      P["Driver"]["givenName"].as<const char*>(),
                      P["Driver"]["familyName"].as<const char*>(),
                      P["points"].as<const char*>());
      }
    }
  }
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
}
//#########################################################################################

void DrawDrivers() {
  // 4) Top 10 Driver Standings
  if (httpGetJson(API_DRIVER_STAND)) {
    Serial.println("\n=== Driver Standings (Top 10) ===");
    JsonArray ds = doc["MRData"]["StandingsTable"]["StandingsLists"][0]
                      ["DriverStandings"]
                        .as<JsonArray>();
    for (int i = 0; i < 10 && i < ds.size(); i++) {
      JsonObject d = ds[i];
      Serial.printf("  #%s %s — %sp (%s wins)\n",
                    d["position"].as<const char*>(),
                    d["Driver"]["familyName"].as<const char*>(),
                    d["points"].as<const char*>(),
                    d["wins"].as<const char*>());
    }
  }
}
//#########################################################################################

void RefreshTime() {
  // Wait before refreshing
  Serial.println("\n(Updating 10 minutes…)\n");
  delay(10UL * 60UL * 1000UL);
}

void DrawConstructors() {
  //Top 10 Constructor Standings
  if (httpGetJson(API_CONSTR_STAND)) {
    Serial.println("\n=== Constructor Standings (Top 10) ===");
    JsonArray cs = doc["MRData"]["StandingsTable"]["StandingsLists"][0]
                      ["ConstructorStandings"]
                        .as<JsonArray>();
    for (int i = 0; i < 10 && i < cs.size(); i++) {
      JsonObject c = cs[i];
      Serial.printf("  #%s %s — %sp (%s wins)\n",
                    c["position"].as<const char*>(),
                    c["Constructor"]["name"].as<const char*>(),
                    c["points"].as<const char*>(),
                    c["wins"].as<const char*>());
    }
  }
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
