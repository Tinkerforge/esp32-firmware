#!/usr/bin/python3 -u

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'provisioning')

from contextlib import contextmanager
import getpass
import json
import os
import re
import sys
import time
import urllib.request
import traceback
import subprocess
import datetime

from provisioning.tinkerforge.ip_connection import IPConnection, base58encode, base58decode, BASE58
from provisioning.tinkerforge.bricklet_warp_energy_manager_v2 import BrickletWARPEnergyManagerV2
from provisioning.tinkerforge.bricklet_warp_front_panel import BrickletWARPFrontPanel

from provisioning.provision_common.provision_common import *
from provisioning.provision_common.sdm_simulator import SDMSimulator

from provisioning.provision_stage_2_warp2 import ContentTypeRemover, factory_reset, connect_to_ethernet

WARP_CHARGER_GIT_PATH = os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', '..', 'warp-charger')
FIRMWARES_GIT_PATH = os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', '..', 'firmwares')

def get_next_serial_number(prefix):
    with open(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'staging-password.txt'), 'r') as f:
        staging_password = f.read().strip()

    context = ssl.create_default_context()

    https_handler = urllib.request.HTTPSHandler(context=context)

    auth_handler = urllib.request.HTTPBasicAuthHandler()
    auth_handler.add_password(realm='Staging',
                              uri='https://stagingwww.tinkerforge.com',
                              user='staging',
                              passwd=staging_password)

    opener = urllib.request.build_opener(https_handler, auth_handler)
    urllib.request.install_opener(opener)

    serial_number = int(urllib.request.urlopen('https://stagingwww.tinkerforge.com/warpsn', timeout=15).read())

    return '{0}{1:09}'.format(prefix, serial_number)

class EnergyManagerV2Tester:
    def __init__(self):
        if len(sys.argv) != 2:
            self.fatal_error("Usage: {} firmware_type".format(sys.argv[0]))

        self.firmware_type = sys.argv[1]
        if self.firmware_type not in ["energy_manager_v2", "smart_energy_broker"]:
            self.fatal_error("Unknown firmware type {}".format(self.firmware_type))

        if self.firmware_type == "energy_manager_v2":
            self.brick_firmware_basename = "warp_energy_manager_v2"
            self.sku = "WARP-EM2"
            self.hw_version = "2.0"
            self.sn_prefix = "7"
        elif self.firmware_type == "smart_energy_broker":
            self.brick_firmware_basename = "smart_energy_broker"
            self.sku = "SEB"
            self.hw_version = "1.0"
            self.sn_prefix = "3"

        self.result = {}
        self.ipcon = IPConnection()
        self.ipcon.connect('localhost', 4223)

        self.result["start"] = now()

        github_reachable = True
        try:
            with urllib.request.urlopen('https://github.com/Tinkerforge/firmwares', timeout=5.0) as req:
                req.read()
        except Exception as e:
            print(e)
            print("github.com not reachable: Will not pull firmwares git.")
            github_reachable = False

        if github_reachable:
            with ChangedDirectory(FIRMWARES_GIT_PATH):
                run(["git", "pull"])

        wem_brick_directory = os.path.join(FIRMWARES_GIT_PATH, "bricks", self.brick_firmware_basename)
        wem_brick_path = os.readlink(os.path.join(wem_brick_directory, "brick_{0}_firmware_latest.bin".format(self.brick_firmware_basename)))
        wem_brick_path = os.path.join(wem_brick_directory, wem_brick_path)

        pattern = r"^WIFI:S:(wem2|seb)-([{BASE58}]{{3,6}});T:WPA;P:([{BASE58}]{{4}}-[{BASE58}]{{4}}-[{BASE58}]{{4}}-[{BASE58}]{{4}});;$".format(BASE58=BASE58)
        qr_code = getpass.getpass(green("Scan the ESP Brick QR code"))
        match = re.match(pattern, qr_code)

        while not match:
            qr_code = getpass.getpass(red("Scan the ESP Brick QR code"))
            match = re.match(pattern, qr_code)

        hardware_type = match.group(1)
        esp_uid_qr = match.group(2)
        passphrase_qr = match.group(3)

        print("ESP Brick QR code data:")
        print("    Hardware type: {}".format(hardware_type))
        print("    UID: {}".format(esp_uid_qr))

        self.result["uid"] = esp_uid_qr

        self.ssid = hardware_type + "-" + esp_uid_qr

        event_log = connect_to_ethernet(self.ssid, "event_log")[0].decode('utf-8')
        print(event_log)

        macs = re.findall(re.compile(r'ethernet         \| Connected: 100 Mbps Full Duplex, MAC: ((?:[0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2})'), event_log)
        if len(macs) == 0:
            self.fatal_error("Failed to find MAC address in event log!")
        self.mac = macs[0]

        m = re.search(r"(?:WARP ENERGY MANAGER 2.0|WARP Energy Manager 2.0|SMART ENERGY BROKER) V(\d+).(\d+).(\d+)", event_log)
        if not m:
            self.fatal_error("Failed to find version number in event log!" + event_log)

        version = [int(x) for x in m.groups()]
        latest_version = [int(x) for x in re.search(r"{0}_firmware_(\d+)_(\d+)_(\d+).bin".format(self.firmware_type), wem_brick_path).groups()]


        if version > latest_version:
            self.fatal_error("Flashed firmware {}.{}.{} is not released yet! Latest released is {}.{}.{}".format(*version, *latest_version))
        elif version < latest_version:
            print("Flashed firmware {}.{}.{} is outdated! Flashing {}.{}.{}...".format(*version, *latest_version))

            with open(wem_brick_path, "rb") as f:
                fw = f.read()

            opener = urllib.request.build_opener(ContentTypeRemover())
            for i in range(5):
                try:
                    req = urllib.request.Request("http://{}/flash_firmware".format(self.ssid), fw)
                    print(opener.open(req).read().decode())
                    break
                except urllib.error.HTTPError as e:
                    print("HTTP error", e)
                    if e.code == 423:
                        self.fatal_error("ESP32 blocked firmware update. Is the Bricklet working correctly?")
                    else:
                        self.fatal_error(e.read().decode("utf-8"))
                except urllib.error.URLError as e:
                    print("URL error", e)
                    if i != 4:
                        print("Failed to flash firmware. Retrying...")
                        time.sleep(3)
                    else:
                        if isinstance(e.reason, ConnectionResetError):
                            self.fatal_error("ESP32 blocked firmware update. Is the Bricklet working correctly?")
                        self.fatal_error("Can't flash firmware!")

            time.sleep(3)
            connect_to_ethernet(self.ssid, "firmware_update/validate")
            factory_reset(self.ssid)
        else:
            print("Flashed firmware is up-to-date.")

        self.result["firmware"] = wem_brick_path.split("/")[-1]

        host = connect_to_ethernet(self.ssid, "hidden_proxy/enable")[1]

        time.sleep(1)
        wem_ipcon = IPConnection()
        try:
            wem_ipcon.connect(host, 4223)
        except Exception as e:
            self.fatal_error("Failed to connect to ESP proxy. Is the router's DHCP cache full?")

        time.sleep(1)
        enumerations = enumerate_devices(wem_ipcon)

        wem_bricklet_enum = next((e for e in enumerations if e.device_identifier == BrickletWARPEnergyManagerV2.DEVICE_IDENTIFIER), None)
        if wem_bricklet_enum == None:
            self.fatal_error("WARP Energy Manager Bricklet not found!")

        fp_bricklet_enum = next((e for e in enumerations if e.device_identifier == BrickletWARPFrontPanel.DEVICE_IDENTIFIER), None)
        if fp_bricklet_enum == None:
            self.fatal_error("WARP Front Panel Bricklet not found!")

        self.wem = BrickletWARPEnergyManagerV2(wem_bricklet_enum.uid, wem_ipcon)

    def fatal_error(self, string):
        fatal_error(string)

    def print_labels(self):
        serial_number = get_next_serial_number(self.sn_prefix)
        production_date = datetime.datetime.now().strftime('%Y-%m')

        print('Printing labels...')

        if self.sku == 'SEB':
            arguments = [
                os.path.join(WARP_CHARGER_GIT_PATH, 'label', 'print-seb-label.py'),
                '-c',
                '2',
            ]
        else:
            arguments = [
                os.path.join(WARP_CHARGER_GIT_PATH, 'label', 'print-wem-label.py'),
                '-c',
                '2',
                self.sku,
            ]

        arguments += [
            self.hw_version,
            serial_number,
            production_date,
            self.mac
        ]

        result = subprocess.check_output(arguments)

        if result == b'':
            print(' ... Labels OK')
        else:
            self.fatal_error(" ... Labels FAILED!")

        time.sleep(2)

        if self.sku == 'SEB':
            arguments = [
                os.path.join(WARP_CHARGER_GIT_PATH, 'label', 'print-package-seb-label.py'),
            ]
        else:
            arguments = [
                os.path.join(WARP_CHARGER_GIT_PATH, 'label', 'print-package2-label.py'),
                self.sku,
            ]

        arguments += [
            self.hw_version,
            serial_number,
            production_date
        ]

        result = subprocess.check_output(arguments)

        if result == b'':
            print(' ... Package label OK')
        else:
            self.fatal_error(" ... Package label FAILED!")

        print(' ... Done')

    def test_all(self):
        self.wem.set_sg_ready_output(0, 0)
        self.wem.set_sg_ready_output(1, 0)
        self.wem.set_relay_output(0, 0)
        self.wem.set_relay_output(1, 0)
        time.sleep(0.25)
        inp = self.wem.get_input()
        if inp == (False, False, False, False):
            print("Check 0000 OK")
        else:
            fatal_error("Check 0000 Failed " + str(inp))

        self.wem.set_sg_ready_output(0, 1)
        self.wem.set_sg_ready_output(1, 0)
        self.wem.set_relay_output(0, 0)
        self.wem.set_relay_output(1, 0)
        time.sleep(0.25)
        inp = self.wem.get_input()
        if inp == (True, False, False, False):
            print("Check 1000 OK")
        else:
            fatal_error("Check 1000 Failed " + str(inp))

        self.wem.set_sg_ready_output(0, 0)
        self.wem.set_sg_ready_output(1, 1)
        self.wem.set_relay_output(0, 0)
        self.wem.set_relay_output(1, 0)
        time.sleep(0.25)
        inp = self.wem.get_input()
        if inp == (False, True, False, False):
            print("Check 0100 OK")
        else:
            fatal_error("Check 0100 Failed" + str(inp))

        self.wem.set_sg_ready_output(0, 0)
        self.wem.set_sg_ready_output(1, 0)
        self.wem.set_relay_output(0, 1)
        self.wem.set_relay_output(1, 0)
        time.sleep(0.25)
        inp = self.wem.get_input()
        if inp == (False, False, True, False):
            print("Check 0010 OK")
        else:
            fatal_error("Check 0010 Failed" + str(inp))

        self.wem.set_sg_ready_output(0, 0)
        self.wem.set_sg_ready_output(1, 0)
        self.wem.set_relay_output(0, 0)
        self.wem.set_relay_output(1, 1)
        time.sleep(0.25)
        inp = self.wem.get_input()
        if inp == (False, False, False, True):
            print("Check 0001 OK")
        else:
            fatal_error("Check 0001 Failed" + str(inp))

        for i in range(10):
            time.sleep(0.25)
            sd_info = self.wem.get_sd_information()
            if (sd_info.sd_status) == 0 and (sd_info.lfs_status) == 0:
                print("SD OK")
                break
        else:
            fatal_error("SD failed")

        state = self.wem.get_energy_meter_state()
        if state.energy_meter_type == self.wem.ENERGY_METER_TYPE_DSZ15DZMOD:
            self.meter_ok = True
            print("Meter OK")
        else:
            fatal_error("Meter failed")

        voltage = self.wem.get_input_voltage()
        if 11500 < voltage < 12500:
            print("Voltage OK {0} mV".format(voltage))
        else:
            fatal_error("Voltage failed {0} mV".format(voltage))

        self.print_labels()

        self.result["end"] = now()
        with open("{}_{}_report_stage_2.json".format(self.ssid, now().replace(":", "-")), "w") as f:
            json.dump(self.result, f, indent=4)

        print('Done!')

if __name__ == "__main__":
    try:
        while True:
            emt = EnergyManagerV2Tester()
            emt.test_all()
    except FatalError:
        input("Press return to exit. ")
        sys.exit(1)
    except Exception as e:
        traceback.print_exc()
        input("Press return to exit. ")
        sys.exit(1)
