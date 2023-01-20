warp-more-hardware ESP32 firmware build instructions
====================================================

This is the README.md for the warp-more-hardware project that 
adds support for more hardware than just the awsome stuff what 
[TinkerForge](https://www.tinkerforge.com/en/shop/) has to offer.

The [README.txt](README.txt) is well worth a read as well!


Build instructions
------------------

- Install Node.js >= 14 and NPM >= 8 (left as exercise for the reader)
- Install PlatformIO >= 6: pip install -U platformio
- The firmwares are organized as PlatformIO environments:
  - `platformio run -e warpAC011K` builds the WARP Charger firmware for the EN+ AC011K hardware
  - ...
- The built firmwares can be found in build/ folder
- You can also run "platformio run -e warp -t upload -t monitor" to build and
  upload the firmware to a connected ESP32 and start the serial monitor
