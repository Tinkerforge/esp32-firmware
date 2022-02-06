Build instructions
------------------

- Install nodejs and npm (left as exercise for the reader)
- Install platformio: pip install -U platformio
- Change to the "other hardware" branch: git checkout warp_on_other_hardware
- Get the Time lib: cd esp32-firmware/software/modules/backend/enplus ; git clone https://github.com/PaulStoffregen/Time.git
- Maybe you have to install some npm modules: npm install cheerio ; npm install postcss
- Cd back to esp32-firmware/software
- Build the firmware: pio run --environment warp8enplus
- Find the *_merged.bin firmware file in the esp32-firmware/software/build directory.
