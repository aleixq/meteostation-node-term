#Terminal d'estació meterològica.
La unitat té un historial de pressió (12hores/4preses) registrada a la memòria
Flash. Quan es reinicia hauria de llegir l'historial de la memòria, així que
no hi ha problema en reiniciar.
L'historial serveix per fer un càlcul de la predicció seguint l'algorisme de
Zambretti.
## Funcionament
A intervals regulars (defecte 5seg). Fa una consulta a l'adreça de l'estació
meteorològica (mdns,hostname=meteonode) que s'hagi descobert a la mateixa xarxa,
la consulta que fa és (http://meteoNodeIP:meteoNodePort)/getMeteoData?time=now.
Un cop feta la consulta el meteonode envia les dades que els sensors han llegit,
en json, ho rebem i renderitzem els valors a la pantalla. Quan hem acabat guardo
a la memòria flash la història, hagi canviat o no.

## Hora
l'agafa de NTP cada 5 minuts.

## Historial:
Inici -> [Llegeix de la flash els registres guardats] ->
Demana lectura al meteonode -> tenint la Lectura comprova si la que hi ha a la
flash(fitxer history.json)és anterior a un interval (per defecte 12h/4samples
això és 3/6/9/12), si ho és fes correr tot l'historial una posició:
Clau    Valor
3       Descarta
2       valor de 1
1       valor de 0
0       valor llegit

