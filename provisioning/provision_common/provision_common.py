#!/usr/bin/python3 -u

from collections import namedtuple
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
import csv

from tinkerforge.bricklet_rgb_led_v2 import BrickletRGBLEDV2
from tinkerforge.ip_connection import IPConnection, base58encode, base58decode, BASE58

rnd = secrets.SystemRandom()

PORT = None

def common_init(port):
    global PORT
    PORT = port

# use "with ChangedDirectory('/path/to/abc')" instead of "os.chdir('/path/to/abc')"
class ChangedDirectory:
    def __init__(self, path):
        self.path = path
        self.previous_path = None

    def __enter__(self):
        self.previous_path = os.getcwd()
        os.chdir(self.path)

    def __exit__(self, type_, value, traceback):
        os.chdir(self.previous_path)

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
    return subprocess.check_output(args, env=dict(os.environ, LC_ALL="en_US.UTF-8", LANG="C", LANGUAGE="en")).decode("utf-8").split("\n")

def esptool(args, override_port=None):
    return run(["python3", "./esptool/esptool.py", "--port", PORT if override_port is None else override_port, *args])

def espefuse(args, override_port=None):
    return run(["python3", "./esptool/espefuse.py", "--port", PORT if override_port is None else override_port, *args])

colors = {"off":"\x1b[00m",
          "blue":   "\x1b[34m",
          "cyan":   "\x1b[36m",
          "green":  "\x1b[32m",
          "red":    "\x1b[31m",
          "gray": "\x1b[90m",
          "blink": "\x1b[5m"}

def red(s):
    return colors["red"]+s+colors["off"]

def green(s):
    return colors["green"]+s+colors["off"]

def gray(s):
    return colors['gray']+s+colors["off"]

def blink(s):
    return colors['blink']+s+colors["off"]

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

# inherit from BaseException instead of Exception to avoid being handled by
# try/except blocks that handle Exception instances. sys.exit() raises a
# SystemExit exception that inherits from BaseException for the same reason
class FatalError(BaseException):
    pass

def fatal_error(*args, force_os_exit=None):
    for line in args:
        print(red(str(line)))

    if force_os_exit != None:
        os._exit(force_os_exit)

    raise FatalError

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


def get_new_uid():
    return int(urllib.request.urlopen('https://stagingwww.tinkerforge.com/uid', timeout=15).read())

def check_if_esp_is_sane_and_get_mac(ignore_flash_errors=False, allowed_revision=3, override_port=None):
    output = esptool(['flash_id'], override_port=override_port) # flash_id to get the flash size
    chip_type = None
    chip_revision = None
    flash_size = None
    crystal = None
    mac = None

    chip_type_re = re.compile(r'Chip is (ESP32-[^\s]*) \(revision v?([^\)]*)\)')
    flash_size_re = re.compile(r'Detected flash size: (\d*[KM]B)')
    crystal_re = re.compile(r'Crystal is (\d*MHz)')
    mac_re = re.compile(r'MAC: ((?:[0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2})')

    for line in output:
        chip_type_match = chip_type_re.match(line)
        if chip_type_match:
            chip_type, chip_revision = chip_type_match.group(1, 2)
            chip_revision = float(chip_revision)

        flash_size_match = flash_size_re.match(line)
        if flash_size_match:
            flash_size = flash_size_match.group(1)

        crystal_match = crystal_re.match(line)
        if crystal_match:
            crystal = crystal_match.group(1)

        mac_match = mac_re.match(line)
        if mac_match:
            mac = mac_match.group(1)

    for name, val, expected in [("chip type", chip_type, "ESP32-D0WD-V3"),
                                ("chip revision", chip_revision, allowed_revision),
                                ("crystal", crystal, "40MHz"),
                                ("flash_size", flash_size, "16MB" if not ignore_flash_errors else None)]:
        if expected is not None and val != expected:
            fatal_error("{} was {}, not the expected {}".format(name, val, expected), "esptool output was:", '\n'.join(output))

    return mac

def get_esp_mac():
    esptool(['read_mac'])

def get_espefuse_tasks_with_two_int_format():
    have_to_set_voltage_fuses = False
    have_to_set_block_3 = False

    output = espefuse(['dump'])

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
            fatal_error("Failed to read eFuses", "could not parse line '{}'".format(line), "espefuse output was", '\n'.join(output))

    if any(b is None for b in blocks):
        fatal_error("Failed to read eFuses", "Not all blocks where found", "espefuse output was", '\n'.join(output))

    if any(i != 0 for i in blocks[1]):
        fatal_error("eFuse block 1 is not empty.", "espefuse output was", '\n'.join(output))

    if any(i != 0 for i in blocks[2]):
        fatal_error("eFuse block 2 is not empty.", "espefuse output was", '\n'.join(output))

    voltage_fuses = blocks[0][4] & 0x0001c000
    if voltage_fuses == 0x0001c000:
        have_to_set_voltage_fuses = False
    elif voltage_fuses == 0x00000000:
        have_to_set_voltage_fuses = True
    else:
        fatal_error("Flash voltage efuses have unexpected value {}".format(voltage_fuses), "espefuse output was", '\n'.join(output))

    block3_bytes = b''.join([r.to_bytes(4, "little") for r in blocks[3]])
    passphrase, uid = block3_to_payload(block3_bytes, True)

    if passphrase == '1-1-1-1' and uid == '000-0000':
        have_to_set_block_3 = True
    else:
        if uid == 0:
            fatal_error("Block 3 efuses have unexpected value {}".format(block3_bytes.hex()),
                        "parsed passphrase and uid are {}; {}".format(passphrase, uid),
                        "espefuse output was",
                        '\n'.join(output))

    return have_to_set_voltage_fuses, have_to_set_block_3, passphrase, uid


def get_espefuse_tasks(override_port=None):
    have_to_set_voltage_fuses = False
    have_to_set_block_3 = False

    output = espefuse(['dump'], override_port=override_port)

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
            fatal_error("Failed to read eFuses", "could not parse line '{}'".format(line), "espefuse output was", '\n'.join(output))

    if any(b is None for b in blocks):
        fatal_error("Failed to read eFuses", "Not all blocks where found", "espefuse output was", '\n'.join(output))

    if any(i != 0 for i in blocks[1]):
        fatal_error("eFuse block 1 is not empty.", "espefuse output was", '\n'.join(output))

    if any(i != 0 for i in blocks[2]):
        fatal_error("eFuse block 2 is not empty.", "espefuse output was", '\n'.join(output))

    voltage_fuses = blocks[0][4] & 0x0001c000
    if voltage_fuses == 0x0001c000:
        have_to_set_voltage_fuses = False
    elif voltage_fuses == 0x00000000:
        have_to_set_voltage_fuses = True
    else:
        fatal_error("Flash voltage efuses have unexpected value {}".format(voltage_fuses), "espefuse output was", '\n'.join(output))

    block3_bytes = b''.join([r.to_bytes(4, "little") for r in blocks[3]])
    passphrase, uid = block3_to_payload(block3_bytes)

    if passphrase == '1-1-1-1' and uid == '1':
        have_to_set_block_3 = True
    else:
        passphrase_invalid = re.match('[{0}]{{4}}-[{0}]{{4}}-[{0}]{{4}}-[{0}]{{4}}'.format(BASE58), passphrase) is None
        uid_invalid = re.match('[{0}]{{3,6}}'.format(BASE58), uid) is None
        if passphrase_invalid or uid_invalid:
            fatal_error("Block 3 efuses have unexpected value {}".format(block3_bytes.hex()),
                        "parsed passphrase and uid are {}; {}".format(passphrase, uid),
                        "espefuse output was",
                        '\n'.join(output))

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

def block3_to_payload(block3, use_two_int_format=False):
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
    if use_two_int_format:
        uid_num = int.from_bytes(uid_bytes, "little")
        uid1 = str(uid_num & 0xFF).zfill(3)
        uid2 = str((uid_num >> 8) & 0xFFFFFF).zfill(4)
        uid = uid1 + '-' + uid2
    else:
        uid = base58encode(int.from_bytes(uid_bytes, "little"))
    passphrase = '-'.join(passphrase)
    return passphrase, uid

def handle_voltage_fuses(set_voltage_fuses):
    if not set_voltage_fuses:
        print("Voltage fuses already burned.")
        return

    print("Burning flash voltage eFuse to 3.3V")
    espefuse(["set_flash_voltage", "3.3V", "--do-not-confirm"])

def handle_block3_fuses(set_block_3, uid, passphrase):
    if not set_block_3:
        print("Block 3 eFuses already set. UID: {}, Passphrase valid".format(uid))
        return uid, passphrase

    print("Reading staging password")
    try:
        with open('staging_password.txt', 'rb') as f:
            staging_password = f.read().decode('utf-8').split('\n')[0].strip()
    except:
        fatal_error('staging_password.txt missing or malformed')
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
    #print("Passphrase: {}-{}-{}-{}".format(*wifi_passphrase))

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
        espefuse(["burn_block_data", "BLOCK3", name, "--do-not-confirm"])

    return uid, '-'.join(wifi_passphrase)

def handle_block3_fuses_with_two_int_format(set_block_3, uid):
    if not set_block_3:
        print("Block 3 eFuses already set. UID: {}".format(uid))
        return uid

    print("UID: " + uid)

    uid_parts = uid.split('-')
    uid1 = int(uid_parts[1]) # max 8 bit
    uid2 = int(uid_parts[2]) # max 24 bit
    if (uid1 > 0xFF) or (uid2 > 0xFFFFFF):
        print('UID malformed')
        return None

    print("Generating efuse binary")
    uid_bytes = (uid1 | (uid2 << 8)).to_bytes(4, byteorder='little')

    #56-95: 5 byte
    #160-183: 3 byte
    #192-255: 8 byte
    # = 16 byte

    # wifi_passphrase unused
    # 4 byte (uid) + 3 byte * 4 (wifi_passphrase) = 16 byte
    binary = bytearray(32)
    binary[28:32] = uid_bytes

    with temp_file() as (fd, name):
        with os.fdopen(fd, 'wb') as f:
            f.write(binary)

        print("Burning UID eFuses")
        espefuse(["burn_block_data", "BLOCK3", name, "--do-not-confirm"])

    return uid

def erase_flash():
    output = '\n'.join(esptool(["erase_flash"]))

    if "Chip erase completed successfully" not in output:
        fatal_error("Failed to erase flash.",
                    "esptool output was",
                    output)

def flash_firmware(path, reset=True):
    output = "\n".join(esptool(["--baud", "921600",
                                "--before", "default_reset",
                                "--after", "hard_reset" if reset else "no_reset",
                                "write_flash",
                                "--flash_mode", "dio",
                                "--flash_freq", "40m",
                                "--flash_size", "16MB",
                                "0x1000", path]))

    if "Hash of data verified." not in output:
        fatal_error("Failed to flash firmware.",
                    "esptool output was",
                    output)

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

def blink_thread_fn(rgbs, stop_event):
    while not stop_event.is_set():
        for rgb in rgbs:
            rgb.set_rgb_value(0,63,0)
            time.sleep(0.5)
        for rgb in rgbs:
            rgb.set_rgb_value(0,0,0)
            time.sleep(0.5)

def now():
    return datetime.datetime.utcnow().replace(tzinfo=datetime.timezone.utc).isoformat()

def my_input(s, color_fn=green):
    return input(color_fn(s) + " ")

Enum = namedtuple('Enum', 'uid hardware_version firmware_version device_identifier position')

def cb_enumerate(uids, uid, connected_uid, position, hardware_version, firmware_version,
                 device_identifier, enumeration_type):
    if enumeration_type == IPConnection.ENUMERATION_TYPE_DISCONNECTED:
        print("")
        return

    uids.add(Enum(uid, hardware_version, firmware_version, device_identifier, position))

def enumerate_devices(ipcon):
    uids = set()
    # Register Enumerate Callback
    ipcon.register_callback(IPConnection.CALLBACK_ENUMERATE, lambda *args: cb_enumerate(uids, *args))

    start = time.time()
    while time.time() - start < 10:
        ipcon.enumerate()
        if len(uids) == 6:
            break
        time.sleep(1)

    return uids

def test_bricklet_ports(ipcon, esp_device_id, is_warp):
    enums = enumerate_devices(ipcon)

    if is_warp:
        if len(enums) != 6 or any(x.device_identifier not in [BrickletRGBLEDV2.DEVICE_IDENTIFIER, esp_device_id] for x in enums):
            fatal_error("Expected 6 RGB LED 2.0 bricklets but found {}".format("\n\t".join("Port {}: {}".format(x.position, x.device_identifier) for x in enums)))
    if not is_warp:
        if len(enums) != 7 or any(x.device_identifier not in [BrickletRGBLEDV2.DEVICE_IDENTIFIER, esp_device_id] for x in enums):
            fatal_error("Expected 6 RGB LED 2.0 bricklets and the Brick itself but found {}".format("\n\t".join("Port {}: {}".format(x.position, x.device_identifier) for x in enums)))

    enums = sorted(enums, key=lambda x: x.position)

    bricklets = [(enum.position, BrickletRGBLEDV2(enum.uid, ipcon)) for enum in enums if enum.device_identifier == BrickletRGBLEDV2.DEVICE_IDENTIFIER]
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

    stop_event = threading.Event()
    blink_thread = threading.Thread(target=blink_thread_fn, args=([x[1] for x in bricklets], stop_event))
    blink_thread.start()
    input("Bricklet ports seem to work. Press any key to continue")
    stop_event.set()
    blink_thread.join()
    ipcon.disconnect()
