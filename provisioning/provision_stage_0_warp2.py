#!/usr/bin/python3 -u

import contextlib
from contextlib import contextmanager
import datetime
import io
import json
import os
import re
import secrets
import shutil
import socket
import ssl
import subprocess
import sys
import tempfile
import threading
import time
import urllib.request

from tinkerforge.ip_connection import IPConnection, base58encode, base58decode, BASE58
from tinkerforge.bricklet_rgb_led_v2 import BrickletRGBLEDV2

from provision_common.provision_common import *

def main():
    if len(sys.argv) != 4:
        fatal_error("Usage: {} firmware port firmware_type".format(sys.argv[0]))

    if not os.path.exists(sys.argv[1]):
        fatal_error("Firmware {} not found.".format(sys.argv[1]))

    firmware_type = sys.argv[3]
    if firmware_type not in ["esp32", "esp32_ethernet", "warp2", "energy_manager", "warp3"]:
        fatal_error("Unknown firmware type {}".format(firmware_type))

    PORT = sys.argv[2]

    common_init(PORT)

    if not os.path.exists(PORT):
        fatal_error("Port {} not found.".format(PORT))

    result = {"start": now()}

    print("Checking ESP state")
    #allowed_revision=3.1 if firmware_type == "warp3" else 3
    mac_address = check_if_esp_is_sane_and_get_mac(ignore_flash_errors=True, allowed_revision=3)
    print("MAC Address is {}".format(mac_address))
    result["mac"] = mac_address

    set_voltage_fuses, set_block_3, passphrase, uid = get_espefuse_tasks()
    result["set_voltage_fuses"] = set_voltage_fuses
    result["set_block_3"] = set_block_3

    handle_voltage_fuses(set_voltage_fuses)

    uid, passphrase = handle_block3_fuses(set_block_3, uid, passphrase)

    print("Verifying eFuses")
    _set_voltage_fuses, _set_block_3, _passphrase, _uid = get_espefuse_tasks()
    if _set_voltage_fuses:
        fatal_error("Failed to verify voltage eFuses! Are they burned in yet?")

    if _set_block_3:
        fatal_error("Failed to verify block 3 eFuses! Are they burned in yet?")

    if _passphrase != passphrase:
        fatal_error("Failed to verify block 3 eFuses! Passphrase is not the expected value")

    if _uid != uid:
        fatal_error("Failed to verify block 3 eFuses! UID {} is not the expected value {}".format(_uid, uid))

    result["uid"] = uid

    print("Erasing flash")
    erase_flash()

    print("Flashing firmware")
    flash_firmware(sys.argv[1])
    result["firmware"] = sys.argv[1]

    if firmware_type in ["esp32", "esp32_ethernet", "warp2", "energy_manager"]:
        print('Configuring CP2102N chip')
        port_name = os.path.split(PORT)[-1]
        device_dir = os.path.realpath(os.path.join('/sys/bus/usb-serial/devices', port_name, '..', '..'))

        with open(os.path.join(device_dir, 'busnum'), 'r') as f:
            busnum = f.read().strip()

        with open(os.path.join(device_dir, 'devnum'), 'r') as f:
            devnum = f.read().strip()

        uid_number = base58decode(uid)

        # include UID as base58 and base10 because Windows reports serial numbers as all
        # uppercase which mangles base58. only [A-Za-z0-9_] is allowed, other chars might stop
        # pySerial from reporting the serial number. a dash (-) is tested to have that effect.
        # brickv expects this format: tinkerforge_<product-name>_<uid:base58>_<uid:base10>
        # the product name is allowed to include underscores. brickv will split by underscores
        # then expects the first part to be "tinkerforge" and the last two parts as the UID in
        # base58 (mangled on Windows) and base10. every other field in between is part of the
        # product name and will be capitalized and joined by a space to form the display name.
        # special care if given to "esp32" which is shown in uppercase
        if firmware_type == "esp32":
            run(['./cp210x-cfg/cp210x-cfg', '-d', busnum + '.' + devnum, '-C', 'Tinkerforge GmbH', '-N', 'ESP32 Brick', '-S', 'Tinkerforge_ESP32_Brick_{0}_{1}'.format(uid, uid_number), '-t', '0'])
        else:
            run(['./cp210x-cfg/cp210x-cfg', '-d', busnum + '.' + devnum, '-C', 'Tinkerforge GmbH', '-N', 'ESP32 Ethernet Brick', '-S', 'Tinkerforge_ESP32_Ethernet_Brick_{0}_{1}'.format(uid, uid_number), '-t', '0'])

        result["cp2102n_configured"] = True

    result["end"] = now()

    with open("{}-{}_{}_report_stage_0.json".format(firmware_type, uid, now().replace(":", "-")), "w") as f:
        json.dump(result, f, indent=4)

    print('Done!')

if __name__ == "__main__":
    main()
