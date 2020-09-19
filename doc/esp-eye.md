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

## TensorFlow in PlatformIO

Die untenstehende Anleitung hat nicht funktioniert :(

Leider kann TensorFlow Lite nicht ohne Weiteres in PlatformIO genutzt werden, es sind einige Schritte dafür notwendig:

1. Modifikation von platformio.ini:
```
[env:esp-wrover-kit]
platform = espressif32
board = esp-wrover-kit
framework = arduino
lib_deps = tfmicro
```

2. Man muss nun erstmal ein ganz normales TensorFlow Lite Projekt erstellen, um da noch einige Sachen rauszukopieren:
```
sudo make -f tensorflow/lite/micro/tools/make/Makefile TARGET=esp generate_hello_world_esp_project
```
In
```
tensorflow/lite/micro/tools/make/gen/esp_xtensa-esp32/prj/hello_world/esp-idf/components
```
gibt es dann den Ordner tfmicro. Dieser muss in PlatformIO in den lib Ordner kopiert werden.

Diese Anleitung hier hat aber funktioniert:

https://create.arduino.cc/projecthub/andri/ai-powered-magic-wand-ab1c90
