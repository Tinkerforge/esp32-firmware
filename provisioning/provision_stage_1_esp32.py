#!/usr/bin/python3 -u

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'provisioning')

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
import traceback
import urllib.request

from provisioning.tinkerforge.ip_connection import IPConnection, base58encode, base58decode, BASE58

ESP_DEVICE_ID = 113

from .provision_common.provision_common import *

def main():
    common_init('/dev/ttyUSB0')

    if len(sys.argv) != 2:
        fatal_error("Usage: {} firmware_type".format(sys.argv[0]))

    firmware_type = sys.argv[1]
    if firmware_type not in ["esp32"]:
        fatal_error("Unknown firmware type {}".format(firmware_type))

    result = {"start": now()}

    print("Checking ESP state")
    mac_address = check_if_esp_is_sane_and_get_mac()
    print("MAC Address is {}".format(mac_address))
    result["mac"] = mac_address

    set_voltage_fuses, set_block_3, passphrase, uid = get_espefuse_tasks()
    if set_voltage_fuses or set_block_3:
        fatal_error("Fuses are not set. Re-run stage 0!")

    esptool(["--after", "hard_reset", "chip_id"])

    result["uid"] = uid

    ssid = "esp32-" + uid

    run(["systemctl", "restart", "NetworkManager.service"])
    run(["iw", "reg", "set", "DE"])

    print("Waiting for ESP wifi. Takes about one minute.")
    if not wait_for_wifi(ssid, 90):
        fatal_error("ESP wifi not found after 90 seconds")

    print("Testing ESP Wifi.")
    with wifi(ssid, passphrase):
        req = urllib.request.Request("http://10.0.0.1/info/version")
        try:
            with urllib.request.urlopen(req, timeout=10) as f:
                fw_version = json.loads(f.read().decode("utf-8"))["firmware"].split("-")[0].split("+")[0]
        except Exception as e:
            fatal_error("Failed to connect via wifi!")

        result["wifi_test_successful"] = True

        ipcon = IPConnection()
        ipcon.connect("10.0.0.1", 4223)
        print("Connected. Testing bricklet ports")

        test_bricklet_ports(ipcon, ESP_DEVICE_ID, False)
        result["bricklet_port_test_successful"] = True

    led0 = input("Does the status LED blink blue? [y/n]")
    while led0 not in ("y", "n"):
        led0 = input("Does the status LED blink blue? [y/n]")
    result["status_led_test_successful"] = led0 == "y"
    if led0 == "n":
        fatal_error("Status LED does not work")

    led1 = input("Press IO0 button (for max 3 seconds). Does LED 1 glow green? [y/n]")
    while led1 not in ("y", "n"):
        led1 = input("Press IO0 Button (for max 3 seconds). Does LED 1 glow green? [y/n]")
    result["io0_test_successful"] = led1 == "y"
    if led1 == "n":
        fatal_error("LED 1 or IO0 button does not work")

    led0_stop = input("Press EN button. Does the status LED stop blinking for some seconds? [y/n]")
    while led0_stop not in ("y", "n"):
        led0_stop = input("Press EN button. Does the status LED stop blinking for some seconds? [y/n]")
    result["enable_test_successful"] = led0_stop == "y"
    if led0_stop == "n":
        fatal_error("EN button does not work")

    result["tests_successful"] = True
    result["end"] = now()

    with open("{}_{}_report_stage_1.json".format(ssid, now().replace(":", "-")), "w") as f:
        json.dump(result, f, indent=4)

    label_success = "n"
    while label_success != "y":
        run(["python3", "print-esp32-label.py", ssid, passphrase, "-c", "3" if firmware_type == "warp1" else "1"])
        label_prompt = "Stick one label on the ESP, put ESP in the ESD bag. Press n to retry printing the label. [y/n]"

        label_success = input(label_prompt)
        while label_success not in ("y", "n"):
            label_success = input(label_prompt)

    if firmware_type == "esp32":
        bag_label_success = "n"
        while bag_label_success != "y":
            run(["python3", "../../flash-test/label/print-label.py", "-c", "1", "ESP32 Brick", str(ESP_DEVICE_ID), datetime.datetime.now().strftime('%Y-%m-%d'), uid, fw_version])
            bag_label_prompt = "Stick bag label on bag. Press n to retry printing the label. [y/n]"

            bag_label_success = input(bag_label_prompt)
            while bag_label_success not in ("y", "n"):
                bag_label_success = input(bag_label_prompt)

    print('Done!')

if __name__ == "__main__":
    try:
        main()
    except FatalError:
        input("Press return to exit. ")
        sys.exit(1)
    except Exception as e:
        traceback.print_exc()
        input("Press return to exit. ")
        sys.exit(1)
