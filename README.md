# MeteoStation-Node-Term
This is a Project to create a weather station, it's based in code from sparkfun by Nathan Seidle, but expanded to make it work to provide a station with its lcd display to show the sensors values. This station also send via serial the sensors data in JSON to a NodeMCU - *the Meteonode* -and it provides a web server to read the data in json (at http://meteoNodeIP:meteoNodePort/getMeteoData), a second nodemcu  - *the Meteoterm* - will connect to the server and will obtain de the sensors values and with these it will calculate some extra information, forecast, pressure history, wind in a polar graphic... And it will be displayed in a tft small screen.

The wifi credentials must be provided in meteonode and meteoterm (an example is provided on each project as wifi-credentials.h.example) move to copy to wifi-credentials.h on each project before builing and uploading the projects.

I build this projects using arduino-makefile from http://hardwarefun.com/tutorials/compiling-arduino-sketches-using-makefile instead of arduino IDE, with some tweaks it can be easy to be ran by IDE.


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

# Meteoterm
BOM:
- NodeMCU 
- ILI9341 Touch screen.
Explained in respective project directory.


# TODO
Add 3d freecad files for cases. 
Add fritzing connections.
Move functions to own classes(as is separated) (yes i have to fulfill the transition from ino to cpp, but this project once upon a time was a simple ino file).
Use E-Ink display in meteoterm to save energy.
Use battery on meteoterm.
Transition to a less energy drainer putting nodemcu in sleep mode, and relax the polling.

# Kudos
big kudos to authors of: 
- Great bme library, from which I have copy some code https://github.com/finitespace/BME280 .
- Another zambretti based forecaster https://github.com/hes19073/hesweewx/blob/0415b182d2318f85cefb9727da630fbe42fab9c9/bin/user/forecast.py 
- A great forecaster https://github.com/fandonov/weatherstation , including zambretti interpretation.
- Zambretti js code of http://www.beteljuice.co.uk/zambretti/forecast.html (the inspiration and the basis of the forecast algorithm in meteoterm.
- Zambretti first forecaster: https://web.archive.org/web/20110610213848/http://www.meteormetrics.com/zambretti.htm
- Ntp code from https://www.geekstips.com/arduino-time-sync-ntp-server-esp8266-udp/
- Great timelib libs: https://github.com/PaulStoffregen/Time 
- Great GFX code from adafruit https://learn.adafruit.com/adafruit-gfx-graphics-library 

