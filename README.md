
# Formula 1 Info tracker  
  
![main1](https://github.com/user-attachments/assets/bd2d88d6-c71a-4726-ab1c-6443115e69ff) 

## About this project  
Formula 1 info screen, is based on esp32 and **2.9"  3 color E-Paper module from weact studio** and ergast API, It allows to track Formula 1 events, dates, points, stats and many more on e-paper display and weserver dashboard.  
  
**Features:**  
  
- Data is fetched from **ergast** API and displayed on the screen every 30 minutes,  
- Automatic winter/summer time change  
- wifi AP mode for fast wifi setup  
    
**Default data on E-paper:**  
  
- Top 5 Teams including constructors points for current season   
- Top 10 drivers including season points   
- Last race location including podium places   
- Next race location, date and time  
- Pole position for next race  

**Buy the original creator a coffee**
<p align="center">
<a href="https://www.buymeacoffee.com/mazur888" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/default-orange.png" alt="Buy Me A Coffee" height="35" width="auto"></a>
</p>
  
## **Hardware:**  
E-Paper module: [2.9 Inch 3-color weact studio e-paper module](https://www.aliexpress.com/item/1005005183232092.html?spm=a2g0o.productlist.main.2.d29e43e0C0sZmD&algo_pvid=ab08f407-4a25-4efe-bcc8-adf1ec59051f&algo_exp_id=ab08f407-4a25-4efe-bcc8-adf1ec59051f-1&pdp_ext_f=%7B%22order%22%3A%22740%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis%21GBP%2126.76%217.09%21%21%21251.74%2166.72%21%40211b813b17533132046734689e8b5e%2112000032024103612%21sea%21UK%210%21ABX&curPageLogUid=xo1fotgOXa89&utparam-url=scene%3Asearch%7Cquery_from%3A)  
ESP32: [ESP32 dev board](https://www.aliexpress.com/item/1005010047681414.html?spm=a2g0o.productlist.main.1.30ec1b2cBcUzxq&algo_pvid=14b9ba4f-94ec-4696-9887-1cc359ec1b20&algo_exp_id=14b9ba4f-94ec-4696-9887-1cc359ec1b20-0&pdp_ext_f=%7B%22order%22%3A%225%22%2C%22eval%22%3A%221%22%2C%22fromPage%22%3A%22search%22%7D&pdp_npi=6%40dis%21DKK%2178.46%2128.25%21%21%2185.80%2130.89%21%40211b61ae17593335981955231ef35b%2112000050947514964%21sea%21DK%21938313092%21X%211%210%21n_tag%3A-29919%3Bd%3A570e4a5%3Bm03_new_user%3A-29895&curPageLogUid=9CoVqIbFTJsd&utparam-url=scene%3Asearch%7Cquery_from%3A%7Cx_object_id%3A1005010047681414%7C_p_origin_prod%3A)

**Soldering**
1. If ESP32 came with soldered on headers they should be desoldered, i recommend following [this tutorial](https://www.instructables.com/How-to-Desolder-Headers/) 
2. Cut 8 stranded wires to around 10cm
3. Solder on the underside of both devices like below, the connnections can be seen below
[IMAGE]

**ESP32 to Screen Connections:**  
GPIO 4 = BUSY  
GPIO 16 / RX2 = RST  
GPIO 17 / TX2 = DC  
GPIO 5 = CS  
GPIO 18  = SCK/SCL  
GPIO 23 = MOSI/SDA  

## **Software:**
**WiFI access point login details:**  
- SSID "F1 display"  
- PASS "formula1"  
  
**Time zones:**  
- Change time zone to match your location. default time zone is for Europe  
  
MET-1METDST,M3.5.0/01,M10.5.0/02   //Europe  
CET-1CEST,M3.5.0,M10.5.0/3         // Central Europe  
EST-2METDST,M3.5.0/01,M10.5.0/02   // Europe  
EST5EDT,M3.2.0,M11.1.0           // EST USA  
CST6CDT,M3.2.0,M11.1.0           // CST USA  
MST7MDT,M4.1.0,M10.5.0           // MST USA  
NZST-12NZDT,M9.5.0,M4.1.0/3      // Auckland  
EET-2EEST,M3.5.5/0,M10.5.5/0     // Asia  
ACST-9:30ACDT,M10.1.0,M4.1.0/3   // Australia  
  
**API Updates:**  
- Screen and dashboard updates are pulled from server every 30 minutes

## **Third‑Party Libraries**  
- **GxEPD2** (v1.6.4) by ZinggJM — GPL 3.0  
- **U8g2_for_Adafruit_GFX** (v1.8.0) by olikraus — BSD 2‑Clause  
- **WiFiManager** (v2.0.17) by tzapu — MIT  
- **ArduinoJson** (v7.4.2) by Benoit Blanchon — MIT

## Enclosure
- STL Files are included in this project  
  
**Printer:**  
Any printer will be ok, For this project I used Creality K1C and hyper pla  
all settings default in creality print  
printed with no supports  
  
## License
This project is released under the [GNU GPL v3.0](LICENSE).  
© 2025 Mazur888. No warranty; use at your own risk.

**Buy the original creator a coffee**
<p align="center">
<a href="https://www.buymeacoffee.com/mazur888" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/default-orange.png" alt="Buy Me A Coffee" height="35" width="auto"></a>
</p>
