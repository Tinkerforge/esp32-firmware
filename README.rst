ESP32 Firmware
==============

This repository contains the firmware source code for all Tinkerforge
products that are based on the ESP32 Brick or ESP32 Ethernet Brick.
This includes WARP/WARP2 Charger and the WARP Energy Manager.


Repository Content
------------------

provisioning/:
 * Scripts for mass provisioning of ESP32 modules

software/:
 * build/: Compiled files will be put here
 * src/: Source of the firmware, including modules
 * web/: Source of the web interface, including modules

Software
--------

See software/README.txt for build instructions.
More documentation of how the software and the build process work
will follow in the future. For now in brief:

* The software is built with https://platformio.org/
* For each variation of the firmware (warp, warp2, esp32, esp32_ethernet, ...)
  there is a corresponding *.ini file specifying the PlatformIO environment used
  to build that variation. The environments mostly differ in the backend (i.e.
  firmware) and frontend (i.e. web interface) modules selected to be compiled
  into the firmware.
* Custom hooks compile the web interface from TypeScript and Sass into JavaScript
  and CSS, place everything in one HTML file, zip it and create a C header that
  is then compiled in the firmware.
* After the firmware is built in the software/build folder, the custom hooks
  merge the firmware, bootloader, partition table, etc into one bin file that
  can be flashed on the ESP32 Brick at offset 0x1000 or can be uploaded to a
  running WARP Charger.
