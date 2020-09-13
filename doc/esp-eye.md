# ESP-Eye

## Installation
Zuerst müssen die Pakete für den ESP installiert werden. Dafür in der Arduino IDE unter Arduino > Einstellungen unter "Zusätzliche Boardverwalter-URLs" folgendes eintragen:

https://dl.espressif.com/dl/package_esp32_index.json

und "Ok".

Dann unter Werkzeuge > Board > Boardverwalter "esp" suchen und installieren.

Unter Werkzeuge > Board muss dann ESP32 Wrover Module eingestellt werden.

Für den Webserver benötigt man dann die ESPAsyncWebServer Library. Diese lädt man hier runter:

https://github.com/me-no-dev/ESPAsyncWebServer/archive/master.zip

und bindet diese dann über Sketch > Bibliothek einbinden > .ZIP-Bibliothek einbinden ein. 
