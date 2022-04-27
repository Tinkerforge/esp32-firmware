Build instructions
------------------

- Install Node.js and NPM >= 8 (left as exercise for the reader)
- Install PlatformIO: pip install -U platformio
- Update local packages: platformio run -e prepare
- The firmwares are organized as PlatformIO environments:
  - "platformio run -e warp" builds the WARP Charger firmware
  - "platformio run -e warp2" builds the WARP2 Charger firmware
  - "platformio run -e esp32" builds the ESP32 Brick firmware
  - "platformio run -e esp32_ethernet" builds the ESP32 Ethernet Brick firmware
  - ...
- The built firmwares can be found in build/ folder
- You can also run "platformio run -e warp -t upload -t monitor" to build and
  upload the firmware to a connected ESP32 and start the serial monitor
