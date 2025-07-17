## Printer:  
I used Creality K1C  
hyper pla, no supports default speed and layer settings in creality print.  

## What else is needed:  
-) One or two small screws to attach front frame to base (there are 2 pilot holes).  
-) One USB C connector like this (2 or 4 wire):  
-) Optional: 2x 5k1 resistors.  

## Electronics:  
#1  If you dont care about USB C to C you can use **2 wire connector** and solder positive to esp32 dev board's 5V and GND to GND,  
this will give you standard USB A to USB C option only, C to C wont work.

#2 If you want to use C- C cable, you have to buy **4 wire connector**. Solder positive to esp32 dev board's 5V, GND to GND,  
and solder resistors between CC1 and GND, and CC2 and GND to enable power when used C to C cables. 

## Connector I used:  
<img width="592" height="473" alt="Screenshot From 2025-07-17 09-56-34" src="https://github.com/user-attachments/assets/05c50ee7-b88c-44f0-b425-a9ab086ed698" />

 


