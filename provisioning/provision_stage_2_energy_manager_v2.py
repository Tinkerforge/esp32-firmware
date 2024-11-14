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
from provisioning.tinkerforge.bricklet_rgb_led_v2 import BrickletRGBLEDV2
from provisioning.tinkerforge.bricklet_industrial_quad_relay_v2 import BrickletIndustrialQuadRelayV2
from provisioning.tinkerforge.bricklet_industrial_dual_analog_in_v2 import BrickletIndustrialDualAnalogInV2
from provisioning.tinkerforge.bricklet_industrial_dual_ac_in import BrickletIndustrialDualACIn
from provisioning.tinkerforge.bricklet_rs485 import BrickletRS485
from provisioning.tinkerforge.bricklet_io4_v2 import BrickletIO4V2
from provisioning.tinkerforge.bricklet_warp_energy_manager import BrickletWARPEnergyManager

from provisioning.provision_common.provision_common import *
from provisioning.provision_common.sdm_simulator import SDMSimulator

from provisioning.provision_stage_2_warp2 import ContentTypeRemover, factory_reset, connect_to_ethernet

WARP_CHARGER_GIT_PATH = os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', '..', 'warp-charger')
FIRMWARES_GIT_PATH = os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', '..', 'firmwares')

def get_next_serial_number(prefix):
    with open(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'staging-password.txt'), 'r') as f:
        staging_password = f.read().strip()

    if sys.version_info < (3, 5, 3):
        context = ssl.SSLContext(protocol=ssl.PROTOCOL_SSLv23)
    else:
        context = ssl.SSLContext()

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

class EnergyManagerTester:
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
        """
        self.ipcon = IPConnection()
        self.idai = BrickletIndustrialDualAnalogInV2('24yU', self.ipcon)
        self.rgb_led = BrickletRGBLEDV2('VRF', self.ipcon)
        self.iqr = BrickletIndustrialQuadRelayV2('21wZ', self.ipcon)
        self.idaci = BrickletIndustrialDualACIn('acin', self.ipcon)
        self.io4 = BrickletIO4V2('Q4U', self.ipcon)
        self.sdm_sim = SDMSimulator('25in')
        self.sdm_sim.noise = (1.0, 1.0)

        self.ipcon.connect('localhost', 4223)"""

        self.result["start"] = now()

        #self.rgb_led.set_rgb_value(0, 0, 255)
        #self.iqr.set_selected_value(0, 1)

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

        """
        wem_bricklet_directory = os.path.join(FIRMWARES_GIT_PATH, "bricklets", "warp_energy_manager_v2")
        wem_bricklet_path = os.readlink(os.path.join(wem_bricklet_directory, "bricklet_warp_energy_manager_v2_firmware_latest.zbin"))
        wem_bricklet_path = os.path.join(wem_bricklet_directory, wem_bricklet_path)

        wem_brick_directory = os.path.join(FIRMWARES_GIT_PATH, "bricks", self.brick_firmware_basename)
        wem_brick_path = os.readlink(os.path.join(wem_brick_directory, "brick_{0}_firmware_latest.bin".format(self.brick_firmware_basename)))
        wem_brick_path = os.path.join(wem_brick_directory, wem_brick_path)"""

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

        event_log = connect_to_ethernet(self.ssid, "event_log").decode('utf-8')
        print(event_log)

        macs = re.findall(re.compile(r'ethernet         \| Connected: 100 Mbps Full Duplex, MAC: ((?:[0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2})'), event_log)
        if len(macs) == 0:
            self.fatal_error("Failed to find MAC address in event log!")
        self.mac = macs[0]

        """
        m = re.search(r"(?:WARP ENERGY MANAGER|WARP Energy Manager|SMART ENERGY BROKER) V(\d+).(\d+).(\d+)", event_log)
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
            factory_reset(self.ssid)
        else:
            print("Flashed firmware is up-to-date.")

        self.result["firmware"] = wem_brick_path.split("/")[-1]

        connect_to_ethernet(self.ssid, "hidden_proxy/enable")

        time.sleep(1)
        self.wem_ipcon = IPConnection()
        try:
            self.wem_ipcon.connect(self.ssid, 4223)
        except Exception as e:
            self.fatal_error("Failed to connect to ESP proxy. Is the router's DHCP cache full?")

        time.sleep(1)
        enumerations = enumerate_devices(self.wem_ipcon)

        wem_bricklet_enum = next((e for e in enumerations if e.device_identifier == 2169), None)
        if wem_bricklet_enum == None:
            self.fatal_error("WARP Energy Manager Bricklet not found!")

        self.wem = BrickletWARPEnergyManager(wem_bricklet_enum.uid, self.wem_ipcon)
        self.wem.set_rgb_value(0, 0, 255)"""

    def fatal_error(self, string):
        """self.rgb_led.set_rgb_value(255, 0, 0)

        # Depending on the error, the WEM Bricklet might not be initialize yet
        try:
            self.wem.set_rgb_value(255, 0, 0)
        except:
            pass"""
        fatal_error(string)

    def test_voltage_supply(self):
        print('Testing voltage supply...')
        voltage = self.idai.get_all_voltages()[0]/1000.0
        if 11.75 < voltage < 12.25:
            print(' ... Voltage OK ({}V)'.format(voltage))
        else:
            self.fatal_error(" ... Voltage FAILED! ({}V)".format(voltage))

    def test_contactor(self):
        print('Testing contactor...')
        self.wem.set_contactor(True)
        time.sleep(0.5)
        value = self.idaci.get_value()[0]
        if value:
            print(' ... Contactor active OK')
        else:
            self.fatal_error(" ... Contactor active FAILED!")
        self.wem.set_contactor(False)
        time.sleep(0.5)
        value = self.idaci.get_value()[0]
        if not value:
            print(' ... Contactor not active OK')
        else:
            self.fatal_error(" ... Contactor not active FAILED!")
        print(' ... Done')

    def test_gp_output(self):
        print('Testing GP output...')
        self.wem.set_output(True)
        time.sleep(0.5)
        value = self.io4.get_value()[1]
        if not value:
            print(' ... GP output active OK')
        else:
            self.fatal_error(" ... GP output active FAILED!")
        self.wem.set_output(False)
        time.sleep(0.5)
        value = self.io4.get_value()[1]
        if value:
            print(' ... GP output not active OK')
        else:
            self.fatal_error(" ... GP output not active FAILED!")
        print(' ... Done')

    def test_input(self):
        print('Testing input...')

        self.iqr.set_selected_value(1, 0)
        self.iqr.set_selected_value(2, 0)
        time.sleep(0.5)
        value = self.wem.get_input()
        if value == (0, 0):
            print(' ... Input (0, 0) OK')
        else:
            self.fatal_error(" ... Input (0, 0) FAILED!")
        print(' ... Done')

        self.iqr.set_selected_value(1, 1)
        self.iqr.set_selected_value(2, 0)
        time.sleep(0.5)
        value = self.wem.get_input()
        if value == (1, 0):
            print(' ... Input (1, 0) OK')
        else:
            self.fatal_error(" ... Input (1, 0) FAILED!")
        print(' ... Done')

        self.iqr.set_selected_value(1, 0)
        self.iqr.set_selected_value(2, 1)
        time.sleep(0.5)
        value = self.wem.get_input()
        if value == (0, 1):
            print(' ... Input (0, 1) OK')
        else:
            self.fatal_error(" ... Input (0, 1) FAILED!")
        print(' ... Done')

        self.iqr.set_selected_value(1, 1)
        self.iqr.set_selected_value(2, 1)
        time.sleep(0.5)
        value = self.wem.get_input()
        if value == (1, 1):
            print(' ... Input (1, 1) OK')
        else:
            self.fatal_error(" ... Input (1, 1) FAILED!")
        print(' ... Done')

    def test_rs485(self):
        print('Testing RS485...')
        value = self.wem.get_energy_meter_values()
        if value == (53.0, 42.0, 42.0):
            print(' ... RS485 OK')
        else:
            self.fatal_error(" ... RS485 FAILED! ({})".format(value))
        print(' ... Done')

    def test_sd_card(self):
        print('Testing SD card...')
        print(' ... formating SD card')
        status = self.wem.format_sd(0x4223ABCD)
        if status == self.wem.FORMAT_STATUS_OK:
            print(' ... SD card format OK')
        else:
            self.fatal_error(" ... SD card format FAILED! ({})".format(status))

        print(' ... checking SD card status (may take a few seconds)')
        for i in range(10):
            time.sleep(0.5)

            info = self.wem.get_sd_information()
            if info.sd_status == 0 and  info.lfs_status == 0:
                print(' ... SD card status OK')
                break
        else:
            self.fatal_error(" ... SD card status FAILED! ({})".format(info))

        print(' ... Done')

    def print_labels(self):
        serial_number = get_next_serial_number(self.sn_prefix)
        production_date = datetime.datetime.now().strftime('%Y-%m')

        print('Printing labels...')

        if self.sku == 'SEB':
            arguments = [
                os.path.join(WARP_CHARGER_GIT_PATH, 'label', 'print-seb-label.py'),
            ]
        else:
            arguments = [
                os.path.join(WARP_CHARGER_GIT_PATH, 'label', 'print-wem-label.py'),
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
            print(' ... Label 1 OK')
        else:
            self.fatal_error(" ... Label 1 FAILED!")

        result = subprocess.check_output(arguments)
        if result == b'':
            print(' ... Label 2 OK')
        else:
            self.fatal_error(" ... Label 1 FAILED!")

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
        """
        self.wem.set_rgb_value(0, 100, 0)
        self.test_voltage_supply()
        self.wem.set_rgb_value(0, 125, 0)
        self.test_contactor()
        self.wem.set_rgb_value(0, 150, 0)
        self.test_gp_output()
        self.wem.set_rgb_value(0, 175, 0)
        self.test_input()
        self.wem.set_rgb_value(0, 200, 0)
        self.test_rs485()
        self.wem.set_rgb_value(0, 225, 0)
        self.test_sd_card()
        self.wem.set_rgb_value(0, 255, 0)"""

        self.print_labels()

        self.result["end"] = now()
        with open("{}_{}_report_stage_2.json".format(self.ssid, now().replace(":", "-")), "w") as f:
            json.dump(self.result, f, indent=4)

        print('Done!')
        #self.rgb_led.set_rgb_value(0, 255, 0)

if __name__ == "__main__":
    try:
        while True:
            emt = EnergyManagerTester()
            emt.test_all()
    except FatalError:
        sys.exit(1)
    except Exception as e:
        traceback.print_exc()
        sys.exit(1)
