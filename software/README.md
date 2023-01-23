warp-more-hardware ESP32 firmware build instructions
====================================================

This is the README.md for the warp-more-hardware project that 
adds support for more hardware than just the awsome stuff that 
[TinkerForge](https://www.tinkerforge.com/en/shop/) has to offer.

*The original [README.txt](README.txt) is really worth a read as well!*

> **Please make sure not to bother the TinkerForge people when you**  
> **encounter issues with this fork of the software running on non**  
> **TinkerForge hardware.**

Please use our [issue tracker](https://github.com/warp-more-hardware/esp32-firmware/issues) instead.

Or even better, join the project, fix it yourself and open a 
[pull request](https://github.com/warp-more-hardware/esp32-firmware/pulls)

Build instructions
------------------

- Install Node.js >= 14 and NPM >= 8 (left as exercise for the reader)
- Install PlatformIO >= 6: pip install -U platformio
- The firmwares for different targets (hardware versions) are organized as PlatformIO environments:
  - `platformio run -environment warpAC011K` builds the WARP Charger firmware for the EN+ AC011K hardware (with 8MB flash)
  - `pio run -e warpAC011K4mb` builds a shrinked down version for development of the AC011K firmware on 4MB hardware
  - ...  
    *note: `platformio run --environment` is equivalent to `pio run -e`*

Install instructions
------------------

- The built firmwares can be found in the `build/` folder (not in git)  
  `FIRMWARE=$(ls -1rt build/*merged* | tail -1)`
- If you are building for the AC011K, you can flash the firmware either
    - via a serial connection using  
    `esptool.py --chip esp32 --baud 921600 --port $PORT --before default_reset --after hard_reset write_flash 0x1000 $FIRMWARE`.  
    The board has to be in flash mode (connect GPIO0 and GND, see the red cable on the pictures in our [Wiki](https://github.com/warp-more-hardware/esp32-firmware/wiki#flashen-der-alternativen-firmware)).
    - or over the air (if you already have a previous version on the box with the default IP) using  
    `curl -vv --show-error --fail --data-binary "@$FIRMWARE" http://10.0.0.1/flash_firmware`
- You can also run `platformio run -e $ENVIRONMENT -t upload -t monitor` to build and
  upload the firmware to a ESP32 connected via serial and start the serial monitor

