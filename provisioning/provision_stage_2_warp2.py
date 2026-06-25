#!/usr/bin/env -S uv run --active --script

import datetime
import getpass
import json
import os
import re
import sys
import time
import urllib.request
import csv
import traceback
import glob
from pathlib import Path
from collections import namedtuple
from selenium import webdriver
from tinkerforge_util.colored import red, green, blink
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'provisioning')

from provisioning.tinkerforge.ip_connection import IPConnection, BASE58
from provisioning.tinkerforge.bricklet_nfc import BrickletNFC, SimpleGetTagID
from provisioning.tinkerforge.bricklet_evse_v2 import BrickletEVSEV2
from provisioning.provision_common.provision_common import *
from provisioning.provision_common.zbase32 import ZBASE32
from provisioning.provision_stage_3_warp2 import Stage3
from provisioning.pib_compare import compare_data as pib_compare_data

evse = None
host = None
power_off_on_error = True
generation = None
sys_print = print

def orig_print(*args, **kwargs):
    sys_print(*args, **kwargs)
    sys.stdout.flush()
    sys.stderr.flush()

def tprint(*args, **kwargs):
    global orig_print

    prefix = ''
    args = tuple(str(x) for x in args)

    if len(args) > 0 and args[0].startswith('\r'):
        args = (args[0][1:],) + args[1:]
        prefix = '\r'

    orig_print(prefix + datetime.datetime.now().isoformat(), *args, **kwargs)

print = tprint
dprint = tprint

def run_bricklet_tests(ipcon, result, scanner, ssid, stage3):
    global evse
    global generation

    enumerations = enumerate_devices(ipcon)

    master = next((e for e in enumerations if e.device_identifier == 13), None)
    evse_enum = next((e for e in enumerations if e.device_identifier == BrickletEVSEV2.DEVICE_IDENTIFIER), None)
    nfc_enum = next((e for e in enumerations if e.device_identifier == BrickletNFC.DEVICE_IDENTIFIER), None)
    warp4_co_enum = next((e for e in enumerations if e.device_identifier == 2184), None)

    if evse_enum is None:
        fatal_error("No EVSE Bricklet found!")

    # Don't check len(enumeraions) if this is a basic:
    # In this case we are connected to localhost and there's a lot of bricks and bricklets connected to the test PC

    if scanner.qr_variant != "B":
        if nfc_enum is None:
            fatal_error("No NFC Bricklet found!")

        if generation == 4:
            if warp4_co_enum is None:
                fatal_error("No WARP4-Co Bricklet found!")

            expected = 3
        else:
            expected = 2

        if len(enumerations) != expected:
            fatal_error("Unexpected number of devices! Expected {} but got {}.".format(expected, len(enumerations)))

    is_basic = master is not None

    evse = BrickletEVSEV2(evse_enum.uid, ipcon)
    jumper_config, has_lock_switch, evse_version, energy_meter_type = evse.get_hardware_configuration()

    is_smart = not is_basic and energy_meter_type == 0
    is_pro = not is_basic and energy_meter_type != 0

    if is_smart or is_pro:
        host = ssid + ".local"

    automatic = scanner.qr_stand == '0' or scanner.qr_stand_wiring == '0'

    stage3.test_front_panel_button(automatic)
    result["front_panel_button_tested"] = True

    if generation >= 3:
        def manual_check_color(color):
            stage3.beep_notify()

            input_result = "n"

            while input_result := input(green(f'Is front LED {color}? [y/n]')) not in ["y", "n"]:
                pass

            if input_result == "n":
                fatal_error(f"Front LED not {color}!")

        evse.set_indicator_led(255, 60000, 0, 255, 255)
        time.sleep(0.5)
        if automatic and not stage3.is_front_panel_led_red():
            fatal_error("Front LED not red!")
        elif not automatic:
            manual_check_color('red')

        evse.set_indicator_led(255, 60000, 120, 255, 255)
        time.sleep(0.5)
        if automatic and not stage3.is_front_panel_led_green():
            fatal_error("Front LED not green!")
        elif not automatic:
            manual_check_color('green')

        evse.set_indicator_led(255, 60000, 240, 255, 255)
        time.sleep(0.5)
        if automatic and not stage3.is_front_panel_led_blue():
            fatal_error("Front LED not blue!")
        elif not automatic:
            manual_check_color('blue')

        evse.set_indicator_led(255, 60000, 0, 0, 255)
        time.sleep(0.5)
        if automatic and not stage3.is_front_panel_led_white():
            fatal_error("Front LED not white!")
        elif not automatic:
            manual_check_color('white')

        evse.set_indicator_led(-1, 1000, 0, 0, 0)

        result["front_panel_led_tested"] = True

    if is_smart or is_pro:
        if scanner.qr_stand != '0' and scanner.qr_stand_wiring != '0':
            def download_seen_tags():
                with urllib.request.urlopen('http://{}/nfc/seen_tags'.format(host), timeout=3) as f:
                    nfc_str = f.read()

                local_seen_tags = []

                for tag_info in json.loads(nfc_str):
                    if len(tag_info['tag_id']) == 0:
                        continue

                    local_seen_tags.append(SimpleGetTagID(tag_info['tag_type'], [int(x, base=16) for x in tag_info['tag_id'].split(':')], tag_info['last_seen']))

                return local_seen_tags

            collect_nfc_tag_ids(stage3, download_seen_tags, True, expected_count=1)
        else:
            with urllib.request.urlopen('http://{}/nfc/seen_tags'.format(host), timeout=3) as f:
                nfc_str = f.read()

            nfc_data = json.loads(nfc_str)

            if nfc_data[0]['tag_type'] != 2 or \
               nfc_data[0]['tag_id'] != "04:BA:38:42:EF:6C:80" or \
               nfc_data[0]['last_seen'] > 100:
                fatal_error("Did not find NFC tag: {}".format(nfc_str))

    d = {"E": "Eichrecht", "P": "Pro", "S": "Smart", "B": "Basic"}

    if is_basic and scanner.qr_variant != "B":
        fatal_error("Scanned QR code implies variant {}, but detected was Basic (i.e. a Master Brick was found)".format(d[scanner.qr_variant]))

    if is_smart and scanner.qr_variant != "S":
        fatal_error("Scanned QR code implies variant {}, but detected was Smart: An ESP32 Brick was found, but no energy meter. Is the meter not connected or the display not lighting up? Is the QR code correct?".format(d[scanner.qr_variant]))

    if is_pro and scanner.qr_variant != "P":
        fatal_error("Scanned QR code implies variant {}, but detected was Pro: An ESP32 Brick and an energy meter was found. Is the QR code correct?".format(d[scanner.qr_variant]))

    result["evse_version"] = evse_version
    print("EVSE version is {}".format(evse_version))

    result["evse_uid"] = evse_enum.uid
    print("EVSE UID is {}".format(evse_enum.uid))

    if is_basic:
        result["master_uid"] = master.uid
        print("Master UID is {}".format(master.uid))

    if scanner.qr_power == "11" and jumper_config != 3:
        fatal_error("Wrong jumper config detected: {} but expected {} as the configured power is {} kW.".format(jumper_config, 3, scanner.qr_power))

    if scanner.qr_power == "22" and jumper_config != 6:
        fatal_error("Wrong jumper config detected: {} but expected {} as the configured power is {} kW.".format(jumper_config, 6, scanner.qr_power))

    result["jumper_config_checked"] = True
    if has_lock_switch:
        fatal_error("Charger has lock switch. Is the diode missing?")

    result["diode_checked"] = True

    if '--no-pp-resistor-check' in sys.argv:
        print('Skipping PP resistor check')
        result["resistor_checked"] = False
    else:
        outgoing = evse.get_charging_slot(1).max_current
        if scanner.qr_power == "11" and outgoing != 20000:
            fatal_error("Wrong type 2 cable config detected: Allowed current is {} but expected 20 A, as this is a 11 kW box.".format(outgoing / 1000))
        if scanner.qr_power == "22" and outgoing != 32000:
            fatal_error("Wrong type 2 cable config detected: Allowed current is {} but expected 32 A, as this is a 22 kW box.".format(outgoing / 1000))

        result["resistor_checked"] = True

    if is_pro:
        meter_str = urllib.request.urlopen('http://{}/meter/live'.format(host), timeout=3).read()
        meter_data = json.loads(meter_str)
        sps = meter_data["samples_per_second"]
        samples = meter_data["samples"]
        # if not 0.2 < sps < 2.5:
        #     fatal_error("Expected between 0.2 and 2.5 energy meter samples per second, but got {}".format(sps))
        if len(samples) < 2:
            fatal_error("Expected at least 2 samples but got {}".format(len(samples)))

        for i in range(3):
            error_count = evse.get_energy_meter_errors()
            if any(x != 0 for x in error_count):
                # Allow exactly one timeout in the third attempt. This can happen for some reason, but one timeout is "still fine™".
                if i == 2 and (any(x != 0 for x in error_count[1:]) or error_count[0] > 1):
                    fatal_error("Energy meter error count is {}, expected only zeros or at most one timeout (first member)! ".format(error_count) + blink("Complain to Erik!"))
                else:
                    print(".")
            else:
                break
            time.sleep(3)

        result["energy_meter_reachable"] = True

        meter_str = urllib.request.urlopen('http://{}/meter/values'.format(host), timeout=3).read()
        meter_data = json.loads(meter_str)
        if meter_data["energy_abs"] >= 1:
            stage3.beep_notify()
            while my_input(f'Energy meter reports {meter_data["energy_abs"]:.3f} kWh. Only < 1 kWh is allowed. Check if this is okay and press y + return to continue') != "y":
                pass

def exists_evse_test_report(evse_uid):
    global generation
    evse_version = {2: 2, 3: 3, 4: 4}[generation]
    with open(os.path.join("..", "..", "test-reports", f"evse_v{evse_version}", "full_test_log.csv"), newline='') as csvfile:
        reader = csv.reader(csvfile, delimiter=',')
        for row in reader:
            if row[0] == evse_uid:
                return True
    return False

def retry_wrapper(fn, s):
    for i in range(3):
        try:
            return fn()
        except Exception as e:
            print("Failed to {}. ".format(s), end='')
            if i == 2:
                print("(3/3) Giving up.")
                raise e
            print("({}/3). Retrying in 3 seconds.".format(i + 1))
            time.sleep(3)

def is_front_panel_button_pressed():
    global evse
    global generation
    assert generation in (2, 3, 4)
    return retry_wrapper(lambda: evse.get_low_level_state().gpio[{2: 6, 3: 5, 4: 5}[generation]], "check if front panel button is pressed")

def get_iec_state():
    global evse
    return retry_wrapper(lambda: chr(ord('A') + evse.get_state().iec61851_state), "get IEC state")

def reset_dc_fault():
    global evse
    return retry_wrapper(lambda: evse.reset_dc_fault_current_state(0xDC42FA23), "reset DC fault current state")

def has_evse_error():
    global evse
    return retry_wrapper(lambda: evse.get_state().error_state != 0, "get EVSE error state")

def get_contactor_state():
    global evse
    return retry_wrapper(lambda: evse.get_state().contactor_state, "get EVSE contactor error")

def switch_phases(phases):
    global evse
    return retry_wrapper(lambda: evse.set_phase_control(phases), "set phases")

def get_cp_pwm():
    global evse
    return retry_wrapper(lambda: evse.get_low_level_state().cp_pwm_duty_cycle, "get cp pwm")

def get_evse_uptime():
    global evse
    return retry_wrapper(lambda: evse.get_low_level_state().uptime, "get EVSE uptime")

def get_meter_voltages():
    global evse
    meter_values = retry_wrapper(lambda: evse.get_all_energy_meter_values(), "get EVSE energy meter values")
    # The EVSE firmware will always return the LX-N voltages as the first three values if they are available
    # All meters that we support currently (and probably will in the future) support measuring phase voltages
    # The old (non-dynamic) variant of this function also returns the voltages first
    # -> This will work
    if len(meter_values) >= 3:
        return meter_values[:3]
    return None

def reset_evse():
    global evse
    retry_wrapper(lambda: evse.reset(), "reset EVSE")
    time.sleep(0.5)
    for i in range(3):
        retry_wrapper(lambda: evse.set_test_mode(True, 0xdeadbeef), "enable EVSE test mode")
        if retry_wrapper(lambda: evse.get_test_mode(), "read back EVSE test mode"):
            break

class Scanner:
    def __init__(self):
        # T:WARP2-CP-22KW-50;V:2.1;S:5000000001;B:2021-09;A:0;;;
        pattern_4 = r'^T:(WARP(4)-C(S|P|E)-(SS|PC)-((?:11|22)?(?:50|75)?|CC)-(W|C));V:(\d+\.\d+);S:(5\d{9});B:(\d{4}-\d{2})(?:;A:(0|1))?;;;*$'
        pattern_3_2 = r'^T:(WARP(2|3)-C(B|S|P)-(11|22)KW-(50|75|CC)(?:-(PC))?);V:(\d+\.\d+);S:(5\d{9});B:(\d{4}-\d{2})(?:;A:(0|1))?;;;*$'

        self.qr_charger_code = my_input("Scan the charger QR code:")
        m_4 = re.match(pattern_4, self.qr_charger_code)
        if m_4 == None:
            m_3_2 = re.match(pattern_3_2, self.qr_charger_code)

        while not m_4 and not m_3_2:
            self.qr_charger_code = my_input("Scan the charger QR code:", red)
            m_4 = re.match(pattern_4, self.qr_charger_code)
            if m_4 == None:
                m_3_2 = re.match(pattern_3_2, self.qr_charger_code)

        if m_4 != None:
            self.qr_sku = m_4.group(1)
            self.qr_gen = m_4.group(2)
            self.qr_variant = m_4.group(3)

            if m_4.group(5).startswith('11'):
                self.qr_power = '11'
            elif m_4.group(5).startswith('22'):
                self.qr_power = '22'
            elif m_4.group(5) == 'CC':
                self.qr_power = 'CC'

            if m_4.group(5).endswith('50'):
                self.qr_cable_len = '50'
            elif m_4.group(5).endswith('75'):
                self.qr_cable_len = '75'
            elif m_4.group(5) == 'CC':
                self.qr_cable_len = 'CC'

            self.qr_material = m_4.group(4)
            self.qr_custom_engraving = m_4.group(6) == 'C'
            self.qr_hw_version = m_4.group(7)
            self.qr_serial = m_4.group(8)
            self.qr_built = m_4.group(9)
            self.qr_extras = m_4.group(10)
        else:
            self.qr_sku = m_3_2.group(1)
            self.qr_gen = m_3_2.group(2)
            self.qr_variant = m_3_2.group(3)
            self.qr_power = m_3_2.group(4)
            self.qr_cable_len = m_3_2.group(5)
            self.qr_material = m_3_2.group(6) if m_3_2.group(6) != None else 'SS'
            self.qr_custom_engraving = '?'
            self.qr_hw_version = m_3_2.group(7)
            self.qr_serial = m_3_2.group(8)
            self.qr_built = m_3_2.group(9)
            self.qr_extras = m_3_2.group(10)

        if self.qr_extras == None:
            self.qr_extras = '0'

        print("Charger QR code data:")
        print("    WARP{} Charger {}".format(self.qr_gen, {"B": "Basic", "S": "Smart", "P": "Pro", "E": "Eichrecht"}[self.qr_variant]))

        if self.qr_power == 'CC' and self.qr_cable_len == 'CC':
            print("    Custom Cable")
        else:
            print("    {} kW".format(self.qr_power))

            if self.qr_cable_len == 'CC':
                print("    Custom Cable")
            else:
                print("    {:1.1f} m".format(int(self.qr_cable_len) / 10.0))

        print("    Material: {}".format(self.qr_material))
        print("    Custom Engraving: {}".format(self.qr_custom_engraving))
        print("    HW Version: {}".format(self.qr_hw_version))
        print("    Serial: {}".format(self.qr_serial))
        print("    Build month: {}".format(self.qr_built))
        print("    Extras: {}".format(self.qr_extras))

        if self.qr_extras == '0':
            self.qr_extras_code = None
            self.qr_stand = '0'
            self.qr_stand_wiring = '0'
            self.qr_stand_lock = False
            self.qr_supply_cable = 0.0
            self.qr_cee = False
            self.qr_custom_engraving = False
            self.qr_custom_type2_cable = False
        else:
            # S:1;W:1;E:2.5;C:1;CFP:1;CT2:1;;;
            pattern = r'^(?:S:(0|1|2|1-PC|2-PC);)?(?:W:(0|1|2);)?(?:L:(0|1);)?E:(\d+\.\d+);C:(0|1);(?:(?:CFP|CE):(0|1);)?(?:CT2:(0|1|M(?:H|F)?(?:9|10|11|12|13|14|15)0|T(?:H|F)?(?:3|4|5|6|7|8|9|10|11|12|13|14|15)0|C(?:H|F)?\d+);)?;;*$'
            self.qr_extras_code = my_input("Scan the extras QR code:")
            m = re.match(pattern, self.qr_extras_code)

            while not m:
                self.qr_extras_code = my_input("Scan the extras QR code:", red)
                m = re.match(pattern, self.qr_extras_code)

            self.qr_stand = m.group(1) if m.group(1) != None else '0'
            self.qr_stand_wiring = m.group(2) if m.group(2) != None else '0'
            self.qr_stand_lock = bool(int(m.group(3) if m.group(3) != None else '0'))
            self.qr_supply_cable = float(m.group(4))
            self.qr_cee = bool(int(m.group(5)))
            self.qr_custom_engraving = bool(int(m.group(6) if m.group(6) != None else '0'))

            if m.group(7) != None and m.group(7) != '0':
                parts = m.group(7).replace('M', 'Metron ').replace('T', 'Tesla ').replace('C', 'Custom ').replace('H', '11kW ').replace('F', '22kW ').split(' ')
                parts[-1] = str(round(int(parts[-1]) / 10, 1))
                self.qr_custom_type2_cable = ' '.join(parts) + ' m'
            else:
                self.qr_custom_type2_cable = False

            print("Extras QR code data:")
            print("    Stand: {}".format(self.qr_stand))
            print("    Stand Wiring: {}".format(self.qr_stand_wiring))
            print("    Stand Lock: {}".format(self.qr_stand_lock))
            print("    Supply Cable: {} m".format(self.qr_supply_cable))
            print("    CEE: {}".format(self.qr_cee))
            print("    Custom Engraving: {}".format(self.qr_custom_engraving))
            print("    Custom Type 2 Cable: {}".format(self.qr_custom_type2_cable))

        if self.qr_variant != "B":
            pattern = rf"^WIFI:S:(warp{self.qr_gen})-([{BASE58}]{{3,6}}|[{ZBASE32}{{3,7}}]);T:WPA;P:([{BASE58}]{{4}}-[{BASE58}]{{4}}-[{BASE58}]{{4}}-[{BASE58}]{{4}});;$"
            self.qr_esp_code = getpass.getpass(green("Scan the ESP Brick QR code: "))
            m = re.match(pattern, self.qr_esp_code)

            while not m:
                self.qr_esp_code = getpass.getpass(red("Scan the ESP Brick QR code: "))
                m = re.match(pattern, self.qr_esp_code)

            self.qr_hardware_type = m.group(1)
            self.qr_esp_uid = m.group(2)
            self.qr_passphrase = m.group(3)

            print("ESP Brick QR code data:")
            print("    Hardware type: {}".format(self.qr_hardware_type))
            print("    UID: {}".format(self.qr_esp_uid))


def set_iso15118_enabled(enable: bool):
    req = urllib.request.Request("http://{}/iso15118/config".format(host), data=json.dumps({
        "autocharge": False,
        "read_soc": enable,
        "charge_via_iso15118": False,
        "min_charge_current": 1000
        }).encode('utf-8'))
    try:
        with urllib.request.urlopen(req, timeout=6) as f:
            f.read()
    except urllib.error.HTTPError as e:
        fatal_error("Failed to enable ISO 15118: {} {}".format(e, e.read()))
    except Exception as e:
        fatal_error("Failed to enable ISO 15118: {}".format(e))


def get_iso15118_ev_mac():
    try:
        with urllib.request.urlopen("http://{}/iso15118/state_slac/pev_mac".format(host), timeout=5) as f:
            return ":".join([hex(x)[2:] for x in json.loads(f.read())])
    except urllib.error.HTTPError as e:
        fatal_error("Failed to get ISO 15118 EV MAC address: {} {}".format(e, e.read()))
    except Exception as e:
        fatal_error("Failed to get ISO 15118 EV MAC address: {}".format(e))


def get_iso15118_attenuation_profile():
    try:
        with urllib.request.urlopen("http://{}/iso15118/state_slac/attenuation_profile".format(host), timeout=5) as f:
            return json.loads(f.read())
    except urllib.error.HTTPError as e:
        fatal_error("Failed to get ISO 15118 attenuation profile: {} {}".format(e, e.read()))
    except Exception as e:
        fatal_error("Failed to get ISO 15118 attenuation profile: {}".format(e))

def upload_iso15118_pib():
    pib_filename = "qca7000_lab_x2.pib"
    pib = Path(pib_filename).read_bytes()
    req = urllib.request.Request("http://{}/iso15118/pib_write".format(host), data=pib)
    try:
        with urllib.request.urlopen(req, timeout=6) as f:
            f.read()
    except urllib.error.HTTPError as e:
        fatal_error("Failed to upload ISO 15118 PIB: {} {}".format(e, e.read()))
    except Exception as e:
        fatal_error("Failed to upload ISO 15118 PIB: {}".format(e))

    start = time.time()
    ex = None
    while time.time() - start < 10:
        time.sleep(0.1)
        req = urllib.request.Request("http://{}/iso15118/pib_read".format(host))
        try:
            with urllib.request.urlopen(req, timeout=2) as f:
                pid_readback = f.read()
                if not pib_compare_data(pib, pid_readback):
                    Path(pib_filename + ".readback").write_bytes(pid_readback)
                    fatal_error(f"ISO 15118 PIB not applied! See {pib_filename} vs {pib_filename}.readback file")
                ex = None
                break
        except urllib.error.HTTPError as e:
            ex = str(e) + e.read().decode('utf-8')
            #fatal_error("Failed to read back ISO 15118 PIB: {} {}".format(e, e.read()))
        except Exception as e:
            ex = e
            #fatal_error("Failed to read back ISO 15118 PIB: {}".format(e))
    if ex is not None:
        fatal_error("Failed to read back ISO 15118 PIB: {}".format(ex))

def led_wrap():
    dprint("pre scanner")

    scanner = Scanner()

    dprint("post scanner")

    stage3 = Stage3(int(scanner.qr_gen),
                    is_front_panel_button_pressed_function=is_front_panel_button_pressed,
                    has_evse_error_function=has_evse_error,
                    get_iec_state_function=get_iec_state,
                    reset_dc_fault_function=reset_dc_fault,
                    switch_phases_function=switch_phases,
                    get_contactor_state_function=get_contactor_state,
                    get_evse_uptime_function=get_evse_uptime,
                    reset_evse_function=reset_evse,
                    get_cp_pwm_function=get_cp_pwm,
                    get_meter_voltages_function=get_meter_voltages,
                    set_iso15118_enabled_function=set_iso15118_enabled,
                    get_iso15118_ev_mac_function=get_iso15118_ev_mac,
                    get_iso15118_attenuation_profile_function=get_iso15118_attenuation_profile)

    stage3.setup()
    stage3.set_led_strip_color((0, 0, 255))

    result = {}

    try:
        main(stage3, scanner, result)
    except BaseException as e:
        result['failure_exception'] = str(e)
        result['failure_traceback'] = traceback.format_exc()

        if power_off_on_error:
            stage3.power_off()

        stage3.set_led_strip_color((255, 0, 0))
        stage3.beep_failure()

        try:
            if scanner.qr_variant != "B":
                product = scanner.qr_hardware_type
                ssid = f'{product}-{scanner.qr_esp_uid}'
            else:
                product = f'warp{scanner.qr_gen}'
                ssid = f'{product}-{result.get("evse_uid", "unknown")}'

            report_path_json = os.path.join("..", "..", "test-reports", product, "{}_{}_report_stage_2_failure.json".format(ssid, now().replace(":", "-")))

            with mkdir_open(report_path_json, "w") as f:
                json.dump(result, f, indent=4)
        except Exception as e:
            print(red(f'Failed to write failure report: {e}'))

        raise
    else:
        stage3.power_off()
        stage3.set_led_strip_color((0, 255, 0))
        stage3.beep_success()

    dprint("exit led_wrap")

blink_start = None
blink_count = 0

def start_blink(count):
    global blink_start, blink_count
    assert count in [3, 2, 1, 0], count
    blink_start = time.monotonic()
    blink_count = count

def blink_tick(stage3):
    diff = (time.monotonic() - blink_start)
    diff -= int(diff)
    color = (255, 127, 0) if diff <= 0.5 else (0, 0, 0)
    if blink_count >= 3:
        stage3.set_led_strip_color(color)
    elif blink_count == 2:
        stage3.set_led_strip_color((0, 0, 255), color)
    elif blink_count == 1:
        stage3.set_led_strip_color((0, 0, 255), (0, 0, 255), color)
    else:
        stage3.set_led_strip_color((0, 0, 255))

def stop_blink(stage3):
    global blink_start, blink_count
    blink_start = None
    blink_count = 0
    stage3.set_led_strip_color((0, 0, 255))

class ContentTypeRemover(urllib.request.BaseHandler):
    def http_request(self, req):
        if req.has_header('Content-type'):
            req.remove_header('Content-type')
        return req
    https_request = http_request

def factory_reset(ssid):
    host = ssid + ".local"
    print("Triggering factory reset")
    print("Connecting via ethernet to {}".format(host), end="")
    for i in range(45):
        start = time.monotonic()
        try:
            req = urllib.request.Request(f"http://{host}/factory_reset",
                                         data=json.dumps({"do_i_know_what_i_am_doing": True}).encode("utf-8"),
                                         method='PUT',
                                         headers={"Content-Type": "application/json"})
            with urllib.request.urlopen(req, timeout=1) as f:
                f.read()
                break
        except Exception as e:
            pass
        t = max(0, 1 - (time.monotonic() - start))
        time.sleep(t)
        orig_print(".", end="")
    else:
        fatal_error("Failed to connect via ethernet!")
    orig_print(" Connected.")
    print("Factory reset triggered.. Waiting 10 seconds")
    time.sleep(10)

def connect_to_ethernet(ssid, url):
    host = ssid + ".local"
    print("Connecting via ethernet to {} [{}]".format(host, url), end="")
    for i in range(45):
        start = time.monotonic()
        try:
            with urllib.request.urlopen(f"http://{host}/{url}", timeout=1) as f:
                result = f.read()
                break
        except:
            pass
        t = max(0, 1 - (time.monotonic() - start))
        time.sleep(t)
        orig_print(".", end="")
    else:
        fatal_error("Failed to connect via ethernet!")
    orig_print(" Connected.")
    return result

TagInfo = namedtuple('TagInfo', 'tag_type tag_id')

def collect_nfc_tag_ids(stage3, getter, beep_notify, expected_count=3):
    print(green("Waiting for NFC tags"), end="")
    seen_tags = []
    last_len = 0
    start_blink(expected_count)
    while len(seen_tags) < expected_count:
        seen_tags = [x for x in getter() if any(y != 0 for y in x.tag_id)]
        if len(seen_tags) != last_len:
            if beep_notify:
                stage3.beep_notify()
            start_blink(max(expected_count - len(seen_tags), 0))
            last_len = len(seen_tags)
        print("\r" + green("Waiting for NFC tags. {} seen".format(len(seen_tags))), end="")
        blink_tick(stage3)
        time.sleep(0.1)
    stop_blink(stage3)
    print(f"\r{expected_count} NFC tag{'s' if expected_count != 1 else ''} seen." + " " * 20)
    return [TagInfo(x.tag_type, ":".join("{:02X}".format(i) for i in x.tag_id)) for x in seen_tags]

def pull_git(name):
    github_reachable = True
    try:
        with urllib.request.urlopen(f'https://github.com/Tinkerforge/{name}', timeout=5.0) as req:
            req.read()
    except:
        print(f"github.com not reachable: Will not pull {name} git.")
        github_reachable = False

    dprint("post github reachable")

    if github_reachable:
        with tfutil.ChangedDirectory(os.path.join("..", "..", name)):
            run(["git", "pull"])

    dprint("post git pull")

def flash_firmware(firmware_path, ssid):
    host = ssid + '.local'

    with open(firmware_path, "rb") as f:
        fw = f.read()

    opener = urllib.request.build_opener(ContentTypeRemover())
    for i in range(5):
        try:
            req = urllib.request.Request("http://{}/flash_firmware".format(host), fw)
            print(opener.open(req).read().decode())
            break
        except urllib.error.HTTPError as e:
            print("HTTP error", e)
            if e.code == 423:
                fatal_error("Charger blocked firmware update. Is the EVSE working correctly?")
            else:
                fatal_error(e.read().decode("utf-8"))
        except urllib.error.URLError as e:
            print("URL error", e)
            if i != 4:
                print("Failed to flash firmware. Retrying...")
                time.sleep(3)
            else:
                if isinstance(e.reason, ConnectionResetError):
                    fatal_error("Charger blocked firmware update. Is the EVSE working correctly?")
                fatal_error("Can't flash firmware!")

    time.sleep(3)
    connect_to_ethernet(ssid, "firmware_update/validate")
    factory_reset(ssid)

def main(stage3, scanner, result):
    global host

    result["start"] = now()

    dprint("main")

    result["serial"] = scanner.qr_serial
    result["qr_code"] = scanner.qr_charger_code

    if scanner.qr_extras_code != None:
        result["extras_qr_code"] = scanner.qr_extras_code

    global generation
    assert scanner.qr_gen in ("2", "3", "4")
    generation = int(scanner.qr_gen)

    result["electrical_tests"] = {}

    if scanner.qr_variant != "B":
        if (scanner.qr_stand != '0' and scanner.qr_stand_wiring != '0') or scanner.qr_supply_cable != 0 or scanner.qr_cee:
            stage3.power_on('CEE')
        elif generation >= 3:
            stage3.power_on('Smart')
        else:
            stage3.power_on({"S": "Smart", "P": "Pro", "E": "Eichrecht"}[scanner.qr_variant])

        dprint("pre nfc tags")

        tag_infos = collect_nfc_tag_ids(stage3, stage3.get_nfc_tag_ids, False)

        dprint("post nfc tags")

        stage3.measure_front_panel_rlow(result["electrical_tests"])

        result["uid"] = scanner.qr_esp_uid
        ssid = scanner.qr_hardware_type + "-" + scanner.qr_esp_uid
        host = ssid + ".local"
        provisioning_firmware_flashed = False
        validate_factory_data = False

        result["factory_data_written"] = False
        result["factory_data_validated"] = False

        if generation >= 4:
            provisioning_firmware_names = glob.glob(f"warp{generation}_firmware-PROVISIONING*_ota.bin")

            if len(provisioning_firmware_names) != 1:
                fatal_error(f"Wrong number of WARP{generation} provisioning firmwares found: {len(provisioning_firmware_names)}")

            latest_provisioning_version = "{}.{}.{}+{}".format(*re.search(rf"warp{generation}_firmware-PROVISIONING(?:.*)_(\d+)_(\d+)_(\d+)_([a-f0-9]+)(?:.*)_ota.bin", provisioning_firmware_names[0]).groups())

            factory_data = json.loads(connect_to_ethernet(ssid, "factory_data/read").decode('utf-8'))
            factory_data_write = False

            if not factory_data['locked']:
                print("Factory data is not locked")

                info_version = json.loads(connect_to_ethernet(ssid, "info/version").decode('utf-8'))

                if latest_provisioning_version != info_version['firmware']:
                    print(f'Flashed provisioning firmware {info_version['firmware']} is outdated! Flashing {latest_provisioning_version}...')
                    flash_firmware(provisioning_firmware_names[0], ssid)
                else:
                    print("Flashed provisioning firmware is up-to-date")

                factory_data_write = True
                provisioning_firmware_flashed = True
            else:
                print("Factory data is already locked")

                if factory_data['sku'] != scanner.qr_sku:
                    print(f"Mismatch between already locked factory data SKU {factory_data["sku"]} and scanned SKU {scanner.qr_sku}")
                    factory_data_write = True

                factory_tag_infos = [TagInfo(x['tag_type'], x['tag_id']) for x in factory_data['nfc_tags']]

                if set(factory_tag_infos) != set(tag_infos):
                    print(f"Mismatch between already locked factory data NFC tags {factory_tag_infos} and seen NFC tag {tag_infos}")
                    factory_data_write = True

                if not factory_data_write:
                    print("Already locked factory data is matching")

                    # use factory_tag_infos even if they match as a set with the seen ones,
                    # but the order might be different. preserve the tag to user assignment order
                    tag_infos = factory_tag_infos

                    result["factory_data_validated"] = True
                else:
                    print("Flashing provisioning firmware to fix factory data mismatch")
                    flash_firmware(provisioning_firmware_names[0], ssid)

                    provisioning_firmware_flashed = True
                    validate_factory_data = True

            dprint("post factory data check")

            if factory_data_write:
                print("Writing factory data")
                req = urllib.request.Request(f"http://{host}/factory_data/write",
                                             data=json.dumps({
                                                 "sku": scanner.qr_sku,
                                                 "nfc_tags": [
                                                     {"tag_type": tag_infos[0].tag_type, "tag_id": tag_infos[0].tag_id},
                                                     {"tag_type": tag_infos[1].tag_type, "tag_id": tag_infos[1].tag_id},
                                                     {"tag_type": tag_infos[2].tag_type, "tag_id": tag_infos[2].tag_id},
                                                 ]
                                             }).encode("utf-8"),
                                             method='PUT',
                                             headers={"Content-Type": "application/json"})
                try:
                    with urllib.request.urlopen(req, timeout=1) as f:
                        response = f.read()

                    if len(response) > 0:
                        print(response)
                except urllib.error.HTTPError as e:
                    fatal_error(f"Failed to write factory data: {e} {e.read()}")
                except Exception as e:
                    fatal_error(f"Failed to write factory data: {e}")

                result["factory_data_written"] = True
                validate_factory_data = True

                dprint("post factory data write")

            security_info = json.loads(connect_to_ethernet(ssid, "esp32/security_info").decode('utf-8'))

            if not security_info['secure_boot'] or \
               not security_info['app_encrypted'] or \
               not security_info['data_encrypted'] or \
               not security_info['lockdown']:
                fatal_error(f"Unexpected ESP32 security info: {security_info}")

            result["security_info_checked"] = True

        info_version = json.loads(connect_to_ethernet(ssid, "info/version").decode('utf-8'))
        version_parts = info_version['firmware'].split('+')
        version = [int(x) for x in version_parts[0].split('.')] + [int(version_parts[1], base=16)]

        dprint("post version fetch")

        if not provisioning_firmware_flashed and '--no-firmware-update' in sys.argv:
            print('Skipping firmware update')
        else:
            pull_git('warp-charger')

            firmware_path = None

            for arg in sys.argv:
                if arg.startswith('--latest-firmware='):
                    firmware_path = arg.removeprefix('--latest-firmware=')

            if firmware_path == None:
                firmware_directory = os.path.join("..", "..", "warp-charger", "firmwares")
                firmware_path = os.readlink(os.path.join(firmware_directory, f"warp{scanner.qr_gen}_firmware_latest_ota.bin"))
                firmware_path = os.path.join(firmware_directory, firmware_path)

            latest_version_parts = list(re.search(rf"warp{scanner.qr_gen}_firmware_(\d+)_(\d+)_(\d+)_([a-f0-9]+)_ota.bin", firmware_path).groups())
            latest_version = [int(x) for x in latest_version_parts] + [int(latest_version_parts[3], base=16)]
            flash_latest_version = False

            if version > latest_version:
                fatal_error("Flashed firmware {}.{}.{}+{:x} is not released yet! Latest release is {}.{}.{}+{:x}".format(*version, *latest_version))
            elif provisioning_firmware_flashed:
                flash_latest_version = True
            elif version < latest_version:
                print("Flashed firmware {}.{}.{}+{:x} is outdated!".format(*version))
                flash_latest_version = True
            else:
                print("Flashed firmware is up-to-date.")

            if flash_latest_version:
                print("Flashing firmware {}.{}.{}+{:x}...".format(*latest_version))
                flash_firmware(firmware_path, ssid)

                result["firmware_file"] = firmware_path.split("/")[-1]
                info_version = json.loads(connect_to_ethernet(ssid, "info/version").decode('utf-8'))

        result["firmware_version"] = info_version['firmware']

        dprint("post firmware update")

        if validate_factory_data:
            factory_data = json.loads(connect_to_ethernet(ssid, "factory_data/read").decode('utf-8'))

            if not factory_data['locked']:
                fatal_error("Factory data is not locked")
            else:
                if factory_data['sku'] != scanner.qr_sku:
                    fatal_error(f"Mismatch between locked factory data SKU {factory_data["sku"]} and scanned SKU {scanner.qr_sku}")

                factory_tag_infos = [TagInfo(x['tag_type'], x['tag_id']) for x in factory_data['nfc_tags']]

                if factory_tag_infos != tag_infos:
                    fatal_error(f"Mismatch between locked factory data NFC tags {factory_tag_infos} and seen NFC tag {tag_infos}")

                print("Locked factory data is matching")

                result["factory_data_validated"] = True

            dprint("post factory data validate")

        connect_to_ethernet(ssid, "hidden_proxy/enable")
        ipcon = IPConnection()

        try:
            ipcon.connect(host, 4223)
        except Exception:
            fatal_error("Failed to connect to ESP proxy. Is the router's DHCP cache full?")

        dprint("post ipcon connect")

        run_bricklet_tests(ipcon, result, scanner, ssid, stage3)
        dprint("post bricklet tests")

        if generation >= 4:
            upload_iso15118_pib()
            dprint("post iso15118 pib")

        user_config = json.loads(connect_to_ethernet(ssid, "users/config").decode('utf-8'))
        do_factory_reset = False
        do_configure_users = True

        if len(user_config["users"]) != 4:
            do_factory_reset = len(user_config["users"]) != 1
        else:
            for i, u in enumerate(user_config["users"][1:]):
                if u["roles"] != 2 ** 16 - 1 or \
                   u["current"] != 32000 or \
                   u["display_name"] != "Benutzer {}".format(i + 1) or \
                   u["username"] != "user{}".format(i + 1) or \
                   u["digest_hash"] != "":
                    do_factory_reset = True
                    break
            else:
                do_configure_users = False

        if do_factory_reset:
            print("Invalid user configuration.")
            factory_reset(ssid)
            connect_to_ethernet(ssid, "hidden_proxy/enable")

        dprint("post user config factory reset")

        if not do_configure_users:
            print("Users already configured")
        else:
            print("Configuring users")
            for i in range(3):
                req = urllib.request.Request("http://{}/users/add".format(host),
                                             data=json.dumps({
                                                 "id": i + 1,
                                                 "roles": 2 ** 16 - 1,
                                                 "current": 32000,
                                                 "display_name": "Benutzer {}".format(i + 1),
                                                 "username": "user{}".format(i + 1),
                                                 "digest_hash": ""
                                             }).encode("utf-8"),
                                             method='PUT',
                                             headers={"Content-Type": "application/json"})
                try:
                    with urllib.request.urlopen(req, timeout=6) as f:
                        response = f.read()

                    if len(response) > 0:
                        print(response)
                except urllib.error.HTTPError as e:
                    fatal_error("Failed to configure user {}: {} {}".format(i, e, e.read()))
                except Exception as e:
                    fatal_error("Failed to configure user {}: {}".format(i, e))

        dprint("post user config")

        print("Configuring tags")
        req = urllib.request.Request("http://{}/nfc/config_update".format(host),
                                     data=json.dumps({
                                         "deadtime_post_start": None,
                                         "authorized_tags": [
                                             {"user_id": 1, "tag_type": tag_infos[0].tag_type, "tag_id": tag_infos[0].tag_id},
                                             {"user_id": 2, "tag_type": tag_infos[1].tag_type, "tag_id": tag_infos[1].tag_id},
                                             {"user_id": 3, "tag_type": tag_infos[2].tag_type, "tag_id": tag_infos[2].tag_id},
                                         ]
                                     }).encode("utf-8"),
                                     method='PUT',
                                     headers={"Content-Type": "application/json"})
        try:
            with urllib.request.urlopen(req, timeout=1) as f:
                response = f.read()

            if len(response) > 0:
                print(response)
        except urllib.error.HTTPError as e:
            fatal_error("Failed to configure NFC tags: {} {}".format(e, e.read()))
        except Exception as e:
            fatal_error("Failed to configure NFC tags: {}".format(e))
        result["nfc_tags_configured"] = True

        dprint("post nfc config")
    else:
        if (scanner.qr_stand != '0' and scanner.qr_stand_wiring != '0') or scanner.qr_supply_cable != 0 or scanner.qr_cee:
            stage3.power_on('CEE')
        else:
            stage3.power_on('Basic')

        stage3.measure_front_panel_rlow(result["electrical_tests"])

        result["uid"] = None

        ipcon = IPConnection()
        ipcon.connect("localhost", 4223)

        enumerations = enumerate_devices(ipcon)
        evse_enum = next((e for e in enumerations if e.device_identifier == 2167), None)

        if evse_enum is None:
            fatal_error("No EVSE Bricklet found!")

        pull_git('firmwares')

        evse_directory = os.path.join("..", "..", "firmwares", "bricklets", "evse_v2")
        evse_path = os.readlink(os.path.join(evse_directory, "bricklet_evse_v2_firmware_latest.zbin"))
        evse_path = os.path.join(evse_directory, evse_path)

        print("Flashing EVSE")
        run(["./comcu_flasher.py", evse_enum.uid, evse_path])
        result["evse_firmware"] = evse_path.split("/")[-1]

        run_bricklet_tests(ipcon, result, scanner, None, stage3)

    if '--no-evse-test-report-check' in sys.argv:
        print('Skipping EVSE test report check')
        result["evse_test_report_found"] = False
    else:
        print("Checking if EVSE was tested...")
        if not exists_evse_test_report(result["evse_uid"]):
            print("No test report found for EVSE {}. Checking for new test reports...".format(result["evse_uid"]))
            with tfutil.ChangedDirectory(os.path.join("..", "..", "test-reports")):
                run(["git", "pull"])
            if not exists_evse_test_report(result["evse_uid"]):
                fatal_error("Still no test report found for EVSE {}.".format(result["evse_uid"]))

        print("EVSE test report found")
        result["evse_test_report_found"] = True

    dprint("post evse test report")

    browser = None
    try:
        if scanner.qr_variant != "B":
            browser = webdriver.Firefox()
            browser.get("http://{}/#evse".format(host))

        print("Performing the electrical tests")
        stage3.test_charger(result, has_phase_switch=generation >= 3, is_warp2=generation == 2)
    finally:
        if browser is not None:
            browser.quit()

    print("Electrical tests passed")
    result["electrical_tests_passed"] = True

    if scanner.qr_variant != "B":
        print("Removing tracked charges")
        print("Connecting via ethernet to {}".format(host), end="")
        for i in range(45):
            start = time.monotonic()
            try:
                req = urllib.request.Request(f"http://{host}/charge_tracker/remove_all_charges",
                                             data=json.dumps({"do_i_know_what_i_am_doing": True}).encode("utf-8"),
                                             method='PUT',
                                             headers={"Content-Type": "application/json"})
                with urllib.request.urlopen(req, timeout=1) as f:
                    f.read()
                    break
            except Exception as e:
                pass
            t = max(0, 1 - (time.monotonic() - start))
            time.sleep(t)
            orig_print(".", end="")
        else:
            fatal_error("Failed to connect via ethernet!")
        orig_print(" Connected.")
        print("Tracked charges removed.")

        print("Erasing other app partition")
        try:
            with urllib.request.urlopen(f"http://{host}/firmware_update/erase_other", timeout=45) as f:
                response = f.read()

            if len(response) > 0:
                print(response)
        except urllib.error.HTTPError as e:
            fatal_error(f"Failed to erase other app partition: {e} {e.read()}")
        except Exception as e:
            fatal_error(f"Failed to erase other app partition: {e}")

    result["end"] = now()

    if scanner.qr_variant != "B":
        product = scanner.qr_hardware_type
    else:
        product = f'warp{scanner.qr_gen}'

    report_path_json = os.path.join("..", "..", "test-reports", product, "{}_{}_report_stage_2.json".format(ssid, now().replace(":", "-")))
    report_path_pdf = report_path_json.replace('.json', '.pdf')

    with mkdir_open(report_path_json, "w") as f:
        json.dump(result, f, indent=4)

    if os.system(f"./report_to_pdf.py {report_path_json} {report_path_pdf} > /dev/null") != 0:
        fatal_error(f"Could not generate PDF report file from {report_path_json}")

    print(f"Printing report {report_path_pdf}")

    if os.system(f'lpr {report_path_pdf}') != 0:
        fatal_error(f"Could not print report")

    print('Done!')

if __name__ == "__main__":
    try:
        led_wrap()
        input('Press return to exit ')
    except FatalError:
        input('Press return to exit ')
        sys.exit(1)
    except Exception as e:
        traceback.print_exc()
        input('Press return to exit ')
        sys.exit(1)
