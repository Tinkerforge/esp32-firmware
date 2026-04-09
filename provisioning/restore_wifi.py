#!/usr/bin/env -S uv run --active --script

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'provisioning')

import contextlib
from contextlib import contextmanager
import datetime
import getpass
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
import csv
from tinkerforge_util.colored import red, green

from provisioning.tinkerforge.ip_connection import IPConnection, base58encode, base58decode, BASE58

def fatal_error(*args):
    for line in args:
        print(red(str(line)))
    sys.exit(0)

def run(args):
    return subprocess.check_output(args, env=dict(os.environ, LC_ALL="en_US.UTF-8")).decode("utf-8").split("\n")

@contextmanager
def wifi(ssid, passphrase):
    output = "\n".join(run(["sudo", "nmcli", "dev", "wifi", "connect", ssid, "password", passphrase]))
    if "successfully activated with" not in output:
        run(["sudo", "nmcli", "con", "del", ssid])
        fatal_error("Failed to connect to wifi.", "nmcli output was:", output)

    try:
        yield
    finally:
        output = "\n".join(run(["sudo", "nmcli", "con", "del", ssid]))
        if "successfully deleted." not in output:
            print("Failed to clean up wifi connection {}".format(ssid))

def my_input(s, color_fn=green):
    return input(color_fn(s) + " ")

def wait_for_wifi(ssid, timeout_s):
    start = time.time()
    last_scan = 0
    while time.time() - start < timeout_s:
        if time.time() - last_scan > 15:
            try:
                run(["sudo", "nmcli", "dev", "wifi", "rescan"])
            except:
                pass
            last_scan = time.time()
        output = '\n'.join(run(["sudo", "nmcli", "dev", "wifi", "list"]))

        if ssid in output:
            return True
        time.sleep(1)
    return False

def main():
    pattern = r"^WIFI:S:(esp32|warp)-([{BASE58}]{{3,6}});T:WPA;P:([{BASE58}]{{4}}-[{BASE58}]{{4}}-[{BASE58}]{{4}}-[{BASE58}]{{4}});;$".format(BASE58=BASE58)
    qr_code = getpass.getpass(green("Scan the ESP Brick QR code"))
    match = re.match(pattern, qr_code)
    while not match:
        qr_code = getpass.getpass(red("Scan the ESP Brick QR code"))
        match = re.match(pattern, qr_code)

    esp_uid_qr = match.group(2)
    passphrase_qr = match.group(3)
    
    ssid = "warp-" + esp_uid_qr

    print("Waiting for ESP wifi. Takes about one minute.")
    if not wait_for_wifi(ssid, 120):
        fatal_error("ESP wifi not found after 120 seconds")

    with wifi(ssid, passphrase_qr):
        my_input("Press any key when done")
  
if __name__ == "__main__":
    main()
