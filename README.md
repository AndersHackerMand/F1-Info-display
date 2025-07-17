##  Like this project? [Buy me a coffee](coff.ee/mazur888)  
# My Project

Welcome to my project! Check out my website: [Mazur’s Electronics Blog](https://example.com)

## License

This project is released under the [GNU GPL v3.0](LICENSE).  
© 2025 Mazur. No warranty; use at your own risk.

# F1 Info display on 2.9" 3 color E-Paper
  
  
## Formula 1 info screen, based on esp32 and **2.9"  3 color E-Paper module from weact studio**.  
Data is fetched from **ergast** API and displayed on the screen every 30 minutes.  
  
  
  
![main1](https://github.com/user-attachments/assets/bd2d88d6-c71a-4726-ab1c-6443115e69ff)  
  
**WiFi AP:**  
SSID "F1 display"  
PASS "formula1"  

**Displayed Data:**  
Top 5 Teams for constructors points,  
Top 10 drivers, including season points,    
Last race location including podium places,  
Next race location and date,   
Pole position for next race.  
  
## Third‑Party Libraries  
- **GxEPD2** (v1.6.4) by ZinggJM — GPL 3.0  
- **U8g2_for_Adafruit_GFX** (v1.8.0) by olikraus — BSD 2‑Clause  
- **WiFiManager** (v2.0.17) by tzapu — MIT  
- **ArduinoJson** (v7.4.2) by Benoit Blanchon — MIT  

**Arduino IDE 2.3.6 with esp32 3.2.1**  

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
