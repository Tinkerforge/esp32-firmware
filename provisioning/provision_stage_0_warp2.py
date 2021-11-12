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
    if len(sys.argv) != 3:
        fatal_error("Usage: {} test_firmware port")

    if not os.path.exists(sys.argv[1]):
        fatal_error("Test firmware {} not found.".format(sys.argv[1]))

    PORT = sys.argv[2]

    common_init(PORT, None, None)

    if not os.path.exists(PORT):
        fatal_error("Port {} not found.".format(PORT))

    result = {"start": now()}

    print("Checking ESP state")
    mac_address = check_if_esp_is_sane_and_get_mac(ignore_flash_errors=True)
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

    print("Flashing test firmware")
    flash_firmware(sys.argv[1])
    result["test_firmware"] = sys.argv[1]

    ssid = "warp2-" + uid

    result["end"] = now()

    with open("{}_{}_report_stage_0.json".format(ssid, now().replace(":", "-")), "w") as f:
        json.dump(result, f, indent=4)

    print('Done!')

if __name__ == "__main__":
    main()
