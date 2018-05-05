#MeteoStation-Node-Term
This is a Project to create a weather station, it's based in code from sparkfun by Nathan Seidle, but expanded to make it work to provide a station with its lcd display to show the sensors values. This station
also send via serial the sensors data in JSON to a NodeMCU - *the Meteonode* -and it provides a web server to read the data in json (at http://meteoNodeIP:meteoNodePort)/getMeteoData), a second nodemcu  - *the Meteoterm* - will connect to the server and will obtain de the sensors values and with these it will calculate some extra information, forecast, pressure history, wind in a polar graphic... And it will be displayed in a tft small screen.

the wifi credentials must be provided in meteonode and meteoterm (an example is provided on each project as wifi-credentials.h.example) move to copy to wifi-credentials.h on each project before builing and uploading the projects.


# Meteostation
BOM:
- Arduino Mega.
- BME280 Pressure sensor.
- SPARKFUn Weather Shield (for wind and rain as pressure temps was a pain...).
- LCD 20x4.

# Meteonode
BOM:
- NodeMCU (linked via serial to MEGA).
- Bidirectional level shifter(3V3 <--> 5V). (to make 5V arduino mega serial pins be reduced to 3V3).

#Meteoterm
BOM:
- NodeMCU 
- ILI9341 Touch screen.

Explained in respective project directory.
