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
import traceback
import urllib.request

from tinkerforge.ip_connection import IPConnection, base58encode, base58decode, BASE58
from tinkerforge.bricklet_rgb_led_v2 import BrickletRGBLEDV2

from provision_common.provision_common import *

def main():
    #common_init('/dev/ttyUSB0', '192.168.178.242', 9100)
    common_init('/dev/ttyUSB0', '192.168.178.242', 9100)

    if len(sys.argv) != 1:
        fatal_error("Usage: {}".format(sys.argv[0]))

    result = {"start": now()}

    check_label_printer()

    print("Checking ESP state")
    mac_address = check_if_esp_is_sane_and_get_mac()
    print("MAC Address is {}".format(mac_address))
    result["mac"] = mac_address

    set_voltage_fuses, set_block_3, passphrase, uid = get_espefuse_tasks()
    if set_voltage_fuses or set_block_3:
        fatal_error("Fuses are not set. Re-run stage 0!")

    esptool(["--after", "hard_reset", "chip_id"])

    result["uid"] = uid

    ssid = "warp2-" + uid

    run(["systemctl", "restart", "NetworkManager.service"])

    print("Waiting for ESP wifi. Takes about one minute.")
    if not wait_for_wifi(ssid, 90):
        fatal_error("ESP wifi not found after 90 seconds")

    print("Testing ESP Wifi.")
    with wifi(ssid, passphrase):
        req = urllib.request.Request("http://10.0.0.1/ethernet/config_update",
                                     data=json.dumps({"enable_ethernet":True,
                                                      "hostname":"warp2-{}".format(uid),
                                                      "ip":[192,168,123,123],
                                                      "gateway":[0,0,0,0],
                                                      "subnet":[0,0,0,0],
                                                      "dns":[0,0,0,0],
                                                      "dns2":[0,0,0,0]}).encode("utf-8"),
                                     method='PUT',
                                     headers={"Content-Type": "application/json"})
        try:
            with urllib.request.urlopen(req, timeout=10) as f:
                f.read()
        except Exception as e:
            fatal_error("Failed to set ethernet config!")

        req = urllib.request.Request("http://10.0.0.1/reboot", data='null'.format(uid).encode("utf-8"), method='PUT', headers={"Content-Type": "application/json"})
        try:
            with urllib.request.urlopen(req, timeout=10) as f:
                f.read()
        except Exception as e:
            fatal_error("Failed to initiate reboot!")

        result["wifi_test_successful"] = True

    print("Connecting via ethernet to 192.168.123.123", end="")
    for i in range(30):
        start = time.time()
        try:
            with urllib.request.urlopen("http://192.168.123.123/hidden_proxy/enable", timeout=1) as f:
                f.read()
                break
        except:
            pass
        t = max(0, 1 - (time.time() - start))
        time.sleep(t)
        print(".", end="")
    else:
        print("Failed to connect via ethernet!")
        raise Exception("exit 1")
    print(" Connected.")

    req = urllib.request.Request("http://192.168.123.123/ethernet/config_update",
                                 data=json.dumps({"enable_ethernet":True,
                                                  "hostname":"warp2-{}".format(uid),
                                                  "ip":[0,0,0,0],
                                                  "gateway":[0,0,0,0],
                                                  "subnet":[0,0,0,0],
                                                  "dns":[0,0,0,0],
                                                  "dns2":[0,0,0,0]}).encode("utf-8"),
                                 method='PUT',
                                 headers={"Content-Type": "application/json"})
    try:
        with urllib.request.urlopen(req, timeout=10) as f:
            f.read()
    except Exception as e:
        fatal_error("Failed to reset ethernet config!")


    ipcon = IPConnection()
    ipcon.connect("192.168.123.123", 4223)
    result["ethernet_test_successful"] = True
    print("Connected. Testing bricklet ports")

    enums = enumerate_devices(ipcon)

    if len(enums) != 6 or any(x.device_identifier != BrickletRGBLEDV2.DEVICE_IDENTIFIER for x in enums):
        fatal_error("Expected 6 RGB LED 2.0 bricklets but found {}".format("\n\t".join("Port {}: {}".format(x.position, x.device_identifier) for x in enums)))

    enums = sorted(enums, key=lambda x: x.position)

    bricklets = [(enum.position, BrickletRGBLEDV2(enum.uid, ipcon)) for enum in enums]
    error_count = 0
    for bricklet_port, rgb in bricklets:
        rgb.set_rgb_value(127, 127, 0)
        time.sleep(0.5)
        if rgb.get_rgb_value() == (127, 127, 0):
            rgb.set_rgb_value(0, 127, 0)
        else:
            print(red("Setting color failed on port {}.".format(bricklet_port)))
            error_count += 1

    if error_count != 0:
        fatal_error("")

    result["bricklet_port_test_successful"] = True

    stop_event = threading.Event()
    blink_thread = threading.Thread(target=blink_thread_fn, args=([x[1] for x in bricklets], stop_event))
    blink_thread.start()
    input("Bricklet ports seem to work. Press any key to continue")
    stop_event.set()
    blink_thread.join()
    ipcon.disconnect()

    led0 = input("Does the status LED blink blue? [y/n]")
    while led0 not in ("y", "n"):
        led0 = input("Does the status LED blink blue? [y/n]")
    result["status_led_test_successful"] = led0 == "y"
    if led0 == "n":
        fatal_error("Status LED does not work")

    # We don't test the IO0 button anymore
    result["io0_test_successful"] = None

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
        run(["python3", "print-esp32-label.py", ssid, passphrase, "-c", "3"])
        label_success = input("Stick one label on the ESP, put ESP and the other two labels in the ESD bag. Press n to retry printing the labels. [y/n]")
        while label_success not in ("y", "n"):
            label_success = input("Stick one label on the ESP, put ESP and the other two labels in the ESD bag. Press n to retry printing the labels. [y/n]")

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
