#!/usr/bin/python3 -u

from contextlib import contextmanager
import getpass
import json
import os
import re
import sys
import time
import urllib.request
import traceback

from tinkerforge.ip_connection import IPConnection, base58encode, base58decode, BASE58
from tinkerforge.bricklet_rgb_led_v2 import BrickletRGBLEDV2
from tinkerforge.bricklet_industrial_quad_relay_v2 import BrickletIndustrialQuadRelayV2
from tinkerforge.bricklet_industrial_dual_analog_in_v2 import BrickletIndustrialDualAnalogInV2
from tinkerforge.bricklet_industrial_dual_ac_in import BrickletIndustrialDualACIn
from tinkerforge.bricklet_rs485 import BrickletRS485
from tinkerforge.bricklet_io4_v2 import BrickletIO4V2
from tinkerforge.bricklet_warp_energy_manager import BrickletWARPEnergyManager

from provision_common.provision_common import *
from provision_common.sdm_simulator import SDMSimulator

from provision_stage_2_warp2 import ContentTypeRemover, factory_reset, connect_to_ethernet

class EnergyManagerTester:
    def __init__(self):
        self.result = {}
        self.ipcon = IPConnection()
        self.idai = BrickletIndustrialDualAnalogInV2('24yU', self.ipcon)
        self.rgb_led = BrickletRGBLEDV2('VRF', self.ipcon)
        self.iqr = BrickletIndustrialQuadRelayV2('21wZ', self.ipcon)
        self.idaci = BrickletIndustrialDualACIn('acin', self.ipcon)
        self.io4 = BrickletIO4V2('Q4U', self.ipcon)
        self.sdm_sim = SDMSimulator('25in')
        self.sdm_sim.noise = (1.0, 1.0)

        self.ipcon.connect('localhost', 4223)

        self.result["start"] = now()

        self.rgb_led.set_rgb_value(0, 0, 255)
        self.iqr.set_selected_value(0, 1)

        with ChangedDirectory(os.path.join("..", "..", "firmwares")):
            run(["git", "pull"])

        wem_bricklet_directory = os.path.join("..", "..", "firmwares", "bricklets", "warp_energy_manager")
        wem_bricklet_path = os.readlink(os.path.join(wem_bricklet_directory, "bricklet_warp_energy_manager_firmware_latest.zbin"))
        wem_bricklet_path = os.path.join(wem_bricklet_directory, wem_bricklet_path)

        wem_brick_directory = os.path.join("..", "..", "firmwares", "bricks", "warp_energy_manager")
        wem_brick_path = os.readlink(os.path.join(wem_brick_directory, "brick_warp_energy_manager_firmware_latest.bin"))
        wem_brick_path = os.path.join(wem_brick_directory, wem_brick_path)


        pattern = r"^WIFI:S:(esp32|warp|warp2|wem)-([{BASE58}]{{3,6}});T:WPA;P:([{BASE58}]{{4}}-[{BASE58}]{{4}}-[{BASE58}]{{4}}-[{BASE58}]{{4}});;$".format(BASE58=BASE58)
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

        self.ssid = "wem-" + esp_uid_qr

        event_log = connect_to_ethernet(self.ssid, "event_log").decode('utf-8')
        print(event_log)

        m = re.search(r"WARP (?:ENERGY MANAGER|Energy Manager) V(\d+).(\d+).(\d+)", event_log)
        if not m:
            self.fatal_error("Failed to find version number in event log!" + event_log)

        version = [int(x) for x in m.groups()]
        latest_version = [int(x) for x in re.search(r"energy_manager_firmware_(\d+)_(\d+)_(\d+).bin", wem_brick_path).groups()]

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
                        self.fatal_error("WEM blocked firmware update. Is the Bricklet working correctly?")
                    else:
                        self.fatal_error(e.read().decode("utf-8"))
                except urllib.error.URLError as e:
                    print("URL error", e)
                    if i != 4:
                        print("Failed to flash firmware. Retrying...")
                        time.sleep(3)
                    else:
                        if isinstance(e.reason, ConnectionResetError):
                            self.fatal_error("WEM blocked firmware update. Is the Bricklet working correctly?")
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
        self.wem.set_rgb_value(0, 0, 255)
    
    def fatal_error(self, string):
        self.rgb_led.set_rgb_value(255, 0, 0)
        self.wem.set_rgb_value(255, 0, 0)
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

    def test_all(self):
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
        self.wem.set_rgb_value(0, 255, 0)


        self.result["end"] = now()
        with open("{}_{}_report_stage_2.json".format(self.ssid, now().replace(":", "-")), "w") as f:
            json.dump(self.result, f, indent=4)
    
        print('Done!')
        self.rgb_led.set_rgb_value(0, 255, 0)

if __name__ == "__main__":
    try:
        emt = EnergyManagerTester()
        emt.test_all()
    except FatalError:
        sys.exit(1)
    except Exception as e:
        traceback.print_exc()
        sys.exit(1)
