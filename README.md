# F1 Info display  

Formula 1 info screen, based on esp32 and **2.9 3 color E-Paper module from weact studio**.  
Data is fetched from **ergast** API and displayed on the screen every 30 minutes.  

**WiFi AP: ** 
SSID "F1 display"  
PASS "formula1"  

**Displayed Data:**  
Top 5 Teams for constructors points,  
Top 10 drivers, including season points,    
Last race location including podium places,  
Next race location and date,   
Pole position for next race.  

**Libraries used:  **  
GxEPD2 (1.6.4) by ZinggJM -  https://github.com/ZinggJM/GxEPD2  
U8g2_for_Adafruit_GFX (1.8.0) by olikraus - https://github.com/olikraus/U8g2_for_Adafruit_GFX  
WiFiManager (2.0.17) by tzapu - https://github.com/tzapu/WiFiManager  
ArduinoJson (7.4.2) by Benoit Blanchon - https://arduinojson.org/?utm_source=meta&utm_medium=library.properties  

**Arduino IDE 2.3.6 with esp32 3.2.1 **   

**Screeen Connections:  **  
BUSY = 4  
CS = 5  
RST = 16  
DC = 17  
SCK = 18  
MISO = 19  
MOSI = 23  

**Time zone is set to BST time in UK.**    
