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
        sys.exit(0)

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
            sys.exit(0)

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
            sys.exit(0)

    if any(b is None for b in blocks):
        print("Failed to read eFuses")
        print("Not all blocks where found")
        print("espefuse output was")
        print('\n'.join(output))
        sys.exit(0)

    if any(i != 0 for i in blocks[1]):
        print("eFuse block 1 is not empty.")
        print("espefuse output was")
        print('\n'.join(output))
        sys.exit(0)

    if any(i != 0 for i in blocks[2]):
        print("eFuse block 2 is not empty.")
        print("espefuse output was")
        print('\n'.join(output))
        sys.exit(0)

    voltage_fuses = blocks[0][4] & 0x0001c000
    if voltage_fuses == 0x0001c000:
        have_to_set_voltage_fuses = False
    elif voltage_fuses == 0x00000000:
        have_to_set_voltage_fuses = True
    else:
        print("Flash voltage efuses have unexpected value {}".format(voltage_fuses))
        print("espefuse output was")
        print('\n'.join(output))
        sys.exit(0)

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
            sys.exit(0)

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
    except:
        print('staging_password.txt missing or malformed')
        sys.exit(0)

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
        sys.exit(0)

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
        sys.exit(0)

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

    if len(sys.argv) != 3:
        print("Usage: {} firmware port")
        sys.exit(0)

    if not os.path.exists(sys.argv[1]):
        print("firmware {} not found.".format(sys.argv[1]))

    PORT = sys.argv[2]

    if not os.path.exists(PORT):
        print("Port {} not found.".format(PORT))

    result = {"start": now()}

    print("Checking ESP state")
    mac_address = check_if_esp_is_sane_and_get_mac()
    print("MAC Address is {}".format(mac_address))
    result["mac"] = mac_address

    set_voltage_fuses, set_block_3, passphrase, uid = get_espefuse_tasks()
    result["set_voltage_fuses"] = set_voltage_fuses
    result["set_block_3"] = set_block_3

    handle_voltage_fuses(set_voltage_fuses)

    uid, passphrase = handle_block3_fuses(set_block_3, uid, passphrase)

    if handle_voltage_fuses or handle_block3_fuses:
        print("Verifying eFuses")
        _set_voltage_fuses, _set_block_3, _passphrase, _uid = get_espefuse_tasks()
        if _set_voltage_fuses:
            print("Failed to verify voltage eFuses! Are they burned in yet?")
            sys.exit(0)

        if _set_block_3:
            print("Failed to verify block 3 eFuses! Are they burned in yet?")
            sys.exit(0)

        if _passphrase != passphrase:
            print("Failed to verify block 3 eFuses! Passphrase is not the expected value")
            sys.exit(0)

        if _uid != uid:
            print("Failed to verify block 3 eFuses! UID {} is not the expected value {}".format(_uid, uid))
            sys.exit(0)

    result["uid"] = uid

    print("Erasing flash")
    erase_flash()

    print("Flashing firmware")
    flash_firmware(sys.argv[1])
    result["firmware"] = sys.argv[1]

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
    run(['./cp210x-cfg/cp210x-cfg', '-d', busnum + '.' + devnum, '-C', 'Tinkerforge GmbH', '-N', 'ESP32 Brick', '-S', 'Tinkerforge_ESP32_Brick_{0}_{1}'.format(uid, uid_number), '-t', '0'])

    result["cp2102n_configured"] = True
    result["end"] = now()

    with open("warp-{}_{}_report_stage_0.json".format(uid, now().replace(":", "-")), "w") as f:
        json.dump(result, f, indent=4)

    print('Done!')

if __name__ == "__main__":
    main()
