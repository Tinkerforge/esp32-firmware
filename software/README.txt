Build instructions
------------------

- Install nodejs and npm (left as exercise for the reader)
- Install platformio: pip install -U platformio
- Update local packages: python3 update_packages.py
- The firmwares are organized as platformio environments: "platformio run -e warp" builds the WARP1 firmware, "platformio run -e warp2" the one for WARP2.
- The built firmwares can be found in build/ folder.
- You can also run "platformio run -e warp -t upload -t monitor" to build and upload the firmware to a connected ESP32 and start the serial monitor
