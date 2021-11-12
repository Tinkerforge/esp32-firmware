ESP32 Firmware
==============

This repository contains the firmware source code and the source code of the
web site, all released firmwares and more.

Repository Content
------------------

provisioning/:
 * Scripts for mass provisioning of ESP32 modules

software/:
 * modules/: Modules that can be emplaced in the firmware and web interface
 * build/: Compiled files will be put here
 * src/: The source of the selected backend modules will be put here
 * web/: Source of the web interface

Software
--------

See software/readme.txt for build instructions.
More documentation of how the software and the build process work
will follow in the future. For now in brief:

* The software is built with https://platformio.org/
* In software/platformio.ini backend (i.e. firmware) and frontend (i.e. web interface) modules
  can be selected to be compiled in the firmware.
* Custom hooks are run in the build process to move the selected modules in
  software/src/modules and software/web/src/modules to compile them in
* The hooks also compile the web interface from Typescript and Sass into Javascript and CSS,
  place everything in one HTML file, zip it and create a C header that is then compiled in the firmware
* After the firmware is built in the software/build folder, the hooks use merge_bins.py
  to merge the firmware, bootloader, partition table, etc into one bin file that can be flashed
  on the ESP32 Brick at offset 0x1000 or can be uploaded to a running WARP Charger.
