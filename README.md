##  Like this project? [Buy me a coffee](coff.ee/mazur888)  

## License

This project is released under the [GNU GPL v3.0](LICENSE).  
© 2025 Mazur888. No warranty; use at your own risk.

# Formula 1 Info tracker on 2.9" 3 color E-Paper
  
  
## Formula 1 info screen, based on esp32 and **2.9"  3 color E-Paper module from weact studio**.  

  
## Featuring:
#1 mDNS option (**f1tracker.local**), or use just device IP address,  
#2 Data is fetched from **ergast** API and displayed on the screen every 30 minutes,  
#3 OTA Upload,  
#3 Full online dashboard including season calendar, all season rounds, pole positions, driver points etc..   

**Displayed Data on E-paper:**  
Top 5 Teams for constructors points,  
Top 10 drivers, including season points,    
Last race location including podium places,  
Next race location and date,   
Pole position for next race.  

**3D printed enclosure and USB Port:**  
Any printer will be ok, I used Creality K1C and hyper pla, all settings default in creality print, no supports.  

![main1](https://github.com/user-attachments/assets/bd2d88d6-c71a-4726-ab1c-6443115e69ff)  
![Dash 1](https://github.com/user-attachments/assets/c9934ac0-f57a-41f1-b868-346afdeda1d2)

  
**WiFi AP:**  
SSID "F1 display"  
PASS "formula1"  
  
## Third‑Party Libraries  
- **GxEPD2** (v1.6.4) by ZinggJM — GPL 3.0  
- **U8g2_for_Adafruit_GFX** (v1.8.0) by olikraus — BSD 2‑Clause  
- **WiFiManager** (v2.0.17) by tzapu — MIT  
- **ArduinoJson** (v7.4.2) by Benoit Blanchon — MIT  

**Arduino IDE 2.3.6 with esp32 3.2.1**  
Arduino setup:  
Board- ESP32 DEV MODULE  
Partition- Minimal SPIFFS with OTA,  
The rest is left default.  

**Screeen Connections:**  
BUSY = 4  
CS = 5  
RST = 16  
DC = 17  
SCK = 18  
MISO = 19  
MOSI = 23  

**Time zone is set to BST time in UK.**   
more timezones here:  
MET-1METDST,M3.5.0/01,M10.5.0/02   //Europe  
CET-1CEST,M3.5.0,M10.5.0/3         // Central Europe  
EST-2METDST,M3.5.0/01,M10.5.0/02   // Europe  
EST5EDT,M3.2.0,M11.1.0           // EST USA  
CST6CDT,M3.2.0,M11.1.0           // CST USA  
MST7MDT,M4.1.0,M10.5.0           // MST USA  
NZST-12NZDT,M9.5.0,M4.1.0/3      // Auckland  
EET-2EEST,M3.5.5/0,M10.5.5/0     // Asia  
ACST-9:30ACDT,M10.1.0,M4.1.0/3   // Australia  
