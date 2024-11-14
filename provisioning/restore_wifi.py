#!/usr/bin/python3 -u

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

from provisioning.tinkerforge.ip_connection import IPConnection, base58encode, base58decode, BASE58

colors = {"off":"\x1b[00m",
          "blue":   "\x1b[34m",
          "cyan":   "\x1b[36m",
          "green":  "\x1b[32m",
          "red":    "\x1b[31m",
          "gray": "\x1b[90m"}

def red(s):
    return colors["red"]+s+colors["off"]

def green(s):
    return colors["green"]+s+colors["off"]

def gray(s):
    return colors['gray']+s+colors["off"]

def remove_color_codes(s):
    for code in colors.values():
        s = s.replace(code, "")
    return s

def ansi_format(fmt, s):
    s = str(s)
    prefix = ""
    suffix = ""
    for code in colors.values():
        if s.startswith(code):
            s = s.replace(code, "")
            prefix += code
        if s.endswith(code):
            s = s.replace(code, "")
            suffix += code
    result = fmt.format(s)
    return prefix + result + suffix

def fatal_error(*args):
    for line in args:
        print(red(str(line)))
    sys.exit(0)

def run(args):
    return subprocess.check_output(args, env=dict(os.environ, LC_ALL="en_US.UTF-8")).decode("utf-8").split("\n")

@contextmanager
def wifi(ssid, passphrase):
    output = "\n".join(run(["nmcli", "dev", "wifi", "connect", ssid, "password", passphrase]))
    if "successfully activated with" not in output:
        run(["nmcli", "con", "del", ssid])
        fatal_error("Failed to connect to wifi.", "nmcli output was:", output)

    try:
        yield
    finally:
        output = "\n".join(run(["nmcli", "con", "del", ssid]))
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
                run(["nmcli", "dev", "wifi", "rescan"])
            except:
                pass
            last_scan = time.time()
        output = '\n'.join(run(["nmcli", "dev", "wifi", "list"]))

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
