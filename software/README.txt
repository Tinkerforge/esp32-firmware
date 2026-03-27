Build instructions
------------------

- Install uv: https://docs.astral.sh/uv/
- On Windows additionally install Node.js >= 20 and NPM >= 9
- Run "uv sync" to create the venv
- Either source the generated venv in .venv or always prefix "uv run" before "pio run"
- If using Visual Studio Code select the venv by pressing ctrl+shift+p, typing "python select interpreter"
  and selecting .venv/bin/python (or .venv/Scripts/python.exe on Windows)
- To improve auto-sourcing the venv in Visual Studio Code configure its terminal by pressing ctrl+,
  - "python-envs.terminal.autoActivationType": "shellStartup"
- If using Visual Studio Code with PlatformIO IDE integration configure it to use the venv by pressing ctrl+,
  - "platformio-ide.customPATH": "<absolute path to this directory>/.venv/bin"
                             (or "<absolute path to this directory>/.venv/Scripts" on Windows)
  - "platformio-ide.useBuiltinPIOCore": false
  - "platformio-ide.useBuiltinPython": false
- If you have problems getting Visual Studio Code with PlatformIO IDE to work properly, then delete
  the .platformio directory in your home directory and restart Visual Studio Code
- The firmwares are organized as PlatformIO environments:
  - "pio run -e warp" builds the WARP Charger firmware
  - "pio run -e warp2" builds the WARP2 Charger firmware
  - "pio run -e warp3" builds the WARP3 Charger firmware
  - "pio run -e energy_manager" builds the WARP Energy Manager firmware
  - "pio run -e energy_manager_v2" builds the WARP Energy Manager 2.0 firmware
  - "pio run -e esp32" builds the ESP32 Brick firmware
  - "pio run -e esp32_ethernet" builds the ESP32 Ethernet Brick firmware
  - ...
- The built firmwares can be found in build/ folder
- You can also run "pio run -e warp3 -t upload -t monitor" to build and
  upload the firmware to a connected ESP32 and start the serial monitor
