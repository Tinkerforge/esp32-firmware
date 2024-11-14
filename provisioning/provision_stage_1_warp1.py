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
from provisioning.tinkerforge.bricklet_rgb_led_v2 import BrickletRGBLEDV2

rnd = secrets.SystemRandom()

PORT = '/dev/ttyUSB0'

@contextmanager
def temp_file():
    fd, name = tempfile.mkstemp()
    try:
        yield fd, name
    finally:
        try:
            os.remove(name)
        except IOError:
            print('Failed to clean up temp file {}'.format(name))

def run(args):
    return subprocess.check_output(args, env=dict(os.environ, LC_ALL="en_US.UTF-8")).decode("utf-8").split("\n")

def esptool(args):
    return run(["python3", "./esptool/esptool.py", *args])

def espefuse(args):
    return run(["python3", "./esptool/espefuse.py", *args])


@contextmanager
def wifi(ssid, passphrase):
    output = "\n".join(run(["nmcli", "dev", "wifi", "connect", ssid, "password", passphrase]))
    if "successfully activated with" not in output:
        run(["nmcli", "con", "del", ssid])
        print("Failed to connect to wifi.")
        print("nmcli output was:")
        print(output)
        raise Exception("exit 1")

    try:
        yield
    finally:
        output = "\n".join(run(["nmcli", "con", "del", ssid]))
        if "successfully deleted." not in output:
            print("Failed to clean up wifi connection {}".format(ssid))


def get_new_uid():
    return int(urllib.request.urlopen('https://stagingwww.tinkerforge.com/uid', timeout=15).read())

def check_if_esp_is_sane_and_get_mac():
    output = esptool(['--port', PORT, 'flash_id']) # flash_id to get the flash size
    chip_type = None
    chip_revision = None
    flash_size = None
    crystal = None
    mac = None

    chip_type_re = re.compile(r'Chip is (ESP32-[^\s]*) \(revision (\d*)\)')
    flash_size_re = re.compile(r'Detected flash size: (\d*[KM]B)')
    crystal_re = re.compile(r'Crystal is (\d*MHz)')
    mac_re = re.compile(r'MAC: ((?:[0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2})')

    for line in output:
        chip_type_match = chip_type_re.match(line)
        if chip_type_match:
            chip_type, chip_revision = chip_type_match.group(1, 2)

        flash_size_match = flash_size_re.match(line)
        if flash_size_match:
            flash_size = flash_size_match.group(1)

        crystal_match = crystal_re.match(line)
        if crystal_match:
            crystal = crystal_match.group(1)

        mac_match = mac_re.match(line)
        if mac_match:
            mac = mac_match.group(1)

    for name, val, expected in [("chip type", chip_type, "ESP32-D0WD-V3"), ("chip revision", chip_revision, "3"), ("crystal", crystal, "40MHz"), ("flash_size", flash_size, "16MB")]:
        if val != expected:
            print("{} was {}, not the expected {}".format(name, val, expected))
            print("esptool output was:")
            print('\n'.join(output))
            raise Exception("exit 1")

    return mac

def get_esp_mac():
    esptool(['--port', PORT, 'read_mac'])

def get_espefuse_tasks():
    have_to_set_voltage_fuses = False
    have_to_set_block_3 = False

    output = espefuse(['--port', PORT, 'dump'])

    def parse_regs(line, regs):
        match = re.search(r'([0-9a-f]{8}\s?)' * regs, line)
        if not match:
            return False, []

        return True, [int(match.group(x + 1), base=16) for x in range(regs)]

    blocks = [None] * 4
    for line in output:
        if line.startswith('BLOCK0'):
            success, blocks[0] = parse_regs(line, 7)
        elif line.startswith('BLOCK1'):
            success, blocks[1] = parse_regs(line, 8)
        elif line.startswith('BLOCK2'):
            success, blocks[2] = parse_regs(line, 8)
        elif line.startswith('BLOCK3'):
            success, blocks[3] = parse_regs(line, 8)
        else:
            continue
        if not success:
            print("Failed to read eFuses")
            print("could not parse line '{}'".format(line))
            print("espefuse output was")
            print('\n'.join(output))
            raise Exception("exit 1")

    if any(b is None for b in blocks):
        print("Failed to read eFuses")
        print("Not all blocks where found")
        print("espefuse output was")
        print('\n'.join(output))
        raise Exception("exit 1")

    if any(i != 0 for i in blocks[1]):
        print("eFuse block 1 is not empty.")
        print("espefuse output was")
        print('\n'.join(output))
        raise Exception("exit 1")

    if any(i != 0 for i in blocks[2]):
        print("eFuse block 2 is not empty.")
        print("espefuse output was")
        print('\n'.join(output))
        raise Exception("exit 1")

    voltage_fuses = blocks[0][4] & 0x0001c000
    if voltage_fuses == 0x0001c000:
        have_to_set_voltage_fuses = False
    elif voltage_fuses == 0x00000000:
        have_to_set_voltage_fuses = True
    else:
        print("Flash voltage efuses have unexpected value {}".format(voltage_fuses))
        print("espefuse output was")
        print('\n'.join(output))
        raise Exception("exit 1")

    block3_bytes = b''.join([r.to_bytes(4, "little") for r in blocks[3]])
    passphrase, uid = block3_to_payload(block3_bytes)

    if passphrase == '1-1-1-1' and uid == '1':
        have_to_set_block_3 = True
    else:
        passphrase_invalid = re.match('[{0}]{{4}}-[{0}]{{4}}-[{0}]{{4}}-[{0}]{{4}}'.format(BASE58), passphrase) is None
        uid_invalid = re.match('[{0}]{{3,6}}'.format(BASE58), uid) is None
        if passphrase_invalid or uid_invalid:
            print("Block 3 efuses have unexpected value {}".format(block3_bytes.hex()))
            print("parsed passphrase and uid are {}; {}".format(passphrase, uid))
            print("espefuse output was")
            print('\n'.join(output))
            raise Exception("exit 1")

    return have_to_set_voltage_fuses, have_to_set_block_3, passphrase, uid

def payload_to_block3(passphrase, uid):
    passphrase_bytes_list = [base58decode(chunk).to_bytes(3, byteorder='little') for chunk in passphrase.split('-')]

    uid_bytes = base58decode(uid).to_bytes(4, byteorder='little')

    binary = bytearray(32)
    binary[7:10] = passphrase_bytes_list[0]
    binary[10:12] = passphrase_bytes_list[1][0:2]
    binary[20] = passphrase_bytes_list[1][2]
    binary[21:23] = passphrase_bytes_list[2][0:2]
    binary[24] = passphrase_bytes_list[2][2]
    binary[25:28] = passphrase_bytes_list[3]
    binary[28:32] = uid_bytes
    return binary

def block3_to_payload(block3):
    passphrase_bytes_list = [[0, 0, 0], [0, 0, 0], [0, 0, 0], [0, 0, 0]]
    passphrase_bytes_list[0] = block3[7:10]
    passphrase_bytes_list[1][0:2] = block3[10:12]
    passphrase_bytes_list[1][2] = block3[20]
    passphrase_bytes_list[2][0:2] = block3[21:23]
    passphrase_bytes_list[2][2] = block3[24]
    passphrase_bytes_list[3] = block3[25:28]
    uid_bytes = bytes(block3[28:32])
    passphrase_bytes_list = [bytes(chunk) for chunk in passphrase_bytes_list]
    passphrase = [base58encode(int.from_bytes(chunk, "little")) for chunk in passphrase_bytes_list]
    uid = base58encode(int.from_bytes(uid_bytes, "little"))
    passphrase = '-'.join(passphrase)
    return passphrase, uid

def handle_voltage_fuses(set_voltage_fuses):
    if not set_voltage_fuses:
        print("Voltage fuses already burned.")
        return

    print("Burning flash voltage eFuse to 3.3V")
    espefuse(["--port", PORT, "set_flash_voltage", "3.3V", "--do-not-confirm"])

def handle_block3_fuses(set_block_3, uid, passphrase):
    if not set_block_3:
        print("Block 3 eFuses already set. UID: {}, Passphrase valid".format(uid))
        return uid, passphrase

    print("Reading staging password")
    try:
        with open('staging_password.txt', 'rb') as f:
            staging_password = f.read().decode('utf-8').split('\n')[0].strip()
    except Exception as e:
        print('staging_password.txt missing or malformed')
        raise Exception("exit 1") from e

    print("Installing auth_handler")
    if sys.version_info < (3,5,3):
        context = ssl.SSLContext(protocol=ssl.PROTOCOL_SSLv23)
    else:
        context = ssl.SSLContext()

    #context.verify_mode = ssl.CERT_REQUIRED
    #context.load_verify_locations(certifi.where())
    https_handler = urllib.request.HTTPSHandler(context=context)

    auth_handler = urllib.request.HTTPBasicAuthHandler()
    auth_handler.add_password(realm='Staging',
                                uri='https://stagingwww.tinkerforge.com',
                                user='staging',
                                passwd=staging_password)

    opener = urllib.request.build_opener(https_handler, auth_handler)
    urllib.request.install_opener(opener)

    print("Generating passphrase")
    # smallest 4-char-base58 string is "2111" = 195112 ("ZZZ"(= 195111) + 1)
    # largest 4-char-base58 string is "ZZZZ" = 11316495
    # Directly selecting chars out of the BASE58 alphabet can result in numbers with leading 1s
    # (those map to 0, so de- and reencoding will produce the same number without the leading 1)
    wifi_passphrase = [base58encode(rnd.randint(base58decode("2111"), base58decode("ZZZZ"))) for i in range(4)]
    print("Generating UID")
    uid = base58encode(get_new_uid())

    print("UID: " + uid)

    print("Generating efuse binary")
    uid_bytes = base58decode(uid).to_bytes(4, byteorder='little')
    passphrase_bytes_list = [base58decode(chunk).to_bytes(3, byteorder='little') for chunk in wifi_passphrase]

    #56-95: 5 byte
    #160-183: 3 byte
    #192-255: 8 byte
    # = 16 byte

    # 4 byte (uid) + 3 byte * 4 (wifi_passphrase) = 16 byte
    binary = bytearray(32)
    binary[7:10] = passphrase_bytes_list[0]
    binary[10:12] = passphrase_bytes_list[1][0:2]
    binary[20] = passphrase_bytes_list[1][2]
    binary[21:23] = passphrase_bytes_list[2][0:2]
    binary[24] = passphrase_bytes_list[2][2]
    binary[25:28] = passphrase_bytes_list[3]
    binary[28:32] = uid_bytes

    with temp_file() as (fd, name):
        with os.fdopen(fd, 'wb') as f:
            f.write(binary)

        print("Burning UID and Wifi passphrase eFuses")
        espefuse(["--port", PORT, "burn_block_data", "BLOCK3", name, "--do-not-confirm"])

    return uid, '-'.join(wifi_passphrase)

def erase_flash():
    output = '\n'.join(esptool(["--port", PORT, "erase_flash"]))

    if "Chip erase completed successfully" not in output:
        print("Failed to erase flash.")
        print("esptool output was")
        print(output)
        raise Exception("exit 1")

def flash_firmware(path, reset=True):
    output = "\n".join(esptool(["--port", PORT,
                                    "--baud", "921600",
                                    "--before", "default_reset",
                                    "--after", "hard_reset" if reset else "no_reset",
                                    "write_flash",
                                    "--flash_mode", "dio",
                                    "--flash_freq", "40m",
                                    "--flash_size", "16MB",
                                    "0x1000", path]))

    if "Hash of data verified." not in output:
        print("Failed to flash firmware.")
        print("esptool output was")
        print(output)
        raise Exception("exit 1")

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

uids = set()

def cb_enumerate(uid, connected_uid, position, hardware_version, firmware_version,
                 device_identifier, enumeration_type):
    if enumeration_type == IPConnection.ENUMERATION_TYPE_DISCONNECTED:
        print("")
        return
    if device_identifier != 2127:
        return

    uids.add((position, uid))

def blink_thread_fn(rgbs, stop_event):
    while not stop_event.is_set():
        for rgb in rgbs:
            rgb.set_rgb_value(0,127,0)
            time.sleep(0.2)
        for rgb in rgbs:
            rgb.set_rgb_value(0,0,0)
            time.sleep(0.2)

def now():
    return datetime.datetime.utcnow().replace(tzinfo=datetime.timezone.utc).isoformat()

def main():
    global uids
    global PORT

    if len(sys.argv) != 1:
        print("Usage: {}")
        raise Exception("exit 1")

    result = {"start": now()}

    print("Checking ESP state")
    mac_address = check_if_esp_is_sane_and_get_mac()
    print("MAC Address is {}".format(mac_address))
    result["mac"] = mac_address

    set_voltage_fuses, set_block_3, passphrase, uid = get_espefuse_tasks()
    if set_voltage_fuses or set_block_3:
        print("Fuses are not set. Re-run stage 0!")

    esptool(["--after", "hard_reset", "chip_id"])

    result["uid"] = uid

    ssid = "warp-" + uid

    run(["systemctl", "restart", "NetworkManager.service"])

    print("Waiting for ESP wifi. Takes about one minute.")
    if not wait_for_wifi(ssid, 90):
        print("ESP wifi not found after 90 seconds")
        raise Exception("exit 1")

    print("Testing ESP Wifi.")
    with wifi(ssid, passphrase):
        with urllib.request.urlopen("http://10.0.0.1/hidden_proxy/enable") as f:
            f.read()
        ipcon = IPConnection()
        ipcon.connect("10.0.0.1", 4223)
        result["wifi_test_successful"] = True
        print("Connected. Testing bricklet ports")
        # Register Enumerate Callback
        ipcon.register_callback(IPConnection.CALLBACK_ENUMERATE, cb_enumerate)

        # Trigger Enumerate
        ipcon.enumerate()
        start = time.time()
        while time.time() - start < 5:
            if len(uids) == 6:
                break
            time.sleep(0.1)

        if len(uids) != 6:
            print("Expected 6 RGB LED 2.0 bricklets but found {}".format(len(uids)))
            raise Exception("exit 1")

        uids = sorted(uids, key=lambda x: x[0])

        bricklets = [(uid[0], BrickletRGBLEDV2(uid[1], ipcon)) for uid in uids]
        error_count = 0
        for bricklet_port, rgb in bricklets:
            rgb.set_rgb_value(127, 127, 0)
            time.sleep(0.5)
            if rgb.get_rgb_value() == (127, 127, 0):
                rgb.set_rgb_value(0, 127, 0)
            else:
                print("Setting color failed on port {}.".format(bricklet_port))
                error_count += 1

        if error_count != 0:
            raise Exception("exit 1")

        result["bricklet_port_test_successful"] = True

        stop_event = threading.Event()
        blink_thread = threading.Thread(target=blink_thread_fn, args=([x[1] for x in bricklets], stop_event))
        blink_thread.start()
        input("Bricklet ports seem to work. Press any key to continue")
        stop_event.set()
        blink_thread.join()
        ipcon.disconnect()

    led0 = input("Does LED 0 blink blue? [y/n]")
    while led0 not in ("y", "n"):
        led0 = input("Does LED 0 blink blue? [y/n]")
    result["led0_test_successful"] = led0 == "y"
    if led0 == "n":
        print("LED 0 does not work")
        raise Exception("exit 1")

    led1 = input("Press IO0 button (for max 3 seconds). Does LED 1 glow green? [y/n]")
    while led1 not in ("y", "n"):
        led1 = input("Press IO0 Button (for max 3 seconds). Does LED 1 glow green? [y/n]")
    result["led1_io0_test_successful"] = led1 == "y"
    if led1 == "n":
        print("LED 1 or IO0 button does not work")
        raise Exception("exit 1")

    led0_stop = input("Press EN button. Does LED 0 stop blinking for some seconds? [y/n]")
    while led0_stop not in ("y", "n"):
        led0_stop = input("Press EN button. Does LED 0 stop blinking for some seconds? [y/n]")
    result["enable_test_successful"] = led0_stop == "y"
    if led0_stop == "n":
        print("EN button does not work")
        raise Exception("exit 1")

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
    except Exception as e:
        traceback.print_exc()
        input("Press return to exit. ")
        sys.exit(1)
