# FeatherLCCCAN

This is a Feather Wing for ESP32 Feathers that contain a CAN tranceiver and 
RJ45 connectors that allow connecting an ESP32 Feather to a LCC network.  The
CAN is set up to use pin 11 for CAN_TX and pin 12 for CAN_RX.

![FeatherLCCCAN](https://github.com/RobertPHeller/FeatherLCCCAN/blob/main/FeatherLCCCAN.png?raw=true)

Subdirectories:

- firmware Simple node, without the display, 4Meg partition table, stores node
  DB on Flash under SPIFFS
- firmware-display Complex node, with display, 8Meg partition table, stores 
  node DB on uSD card (assumes Adafruit 3.5" TFT w/Touch Screen V2.0, which
  includes a uSD card slot).
- DisplayCase FreeCAD code for a case for the version with display.

The firmware compiles with Espressif esp-idf (at least V 5.0) and uses Mike 
Dunston's OpenMRNIDF library.

I am using KiCAD 6.0.11 on a Raspberry Pi 5 running Debian 12.  Also I am using
FreeCAD .21.2 (aarch64 Appimage).
