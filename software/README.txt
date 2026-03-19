Build instructions
------------------

- Install Python >= 3.12, Node.js >= 20, NPM >= 9 and uv https://docs.astral.sh/uv/ (left as exercise for the reader)
- Either run uv sync to create the venv or build once with "uv run pio..."
- Either source the generated venv in .venv or always prefix "uv run" before "pio run"
- If using vscode select the venv by pressing Ctrl+P, typing "python select interpreter" and selecting .venv/bin/python (or .venv/Scripts/python.exe on Windows)
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
