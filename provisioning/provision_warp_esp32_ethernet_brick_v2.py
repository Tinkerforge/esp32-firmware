#!/usr/bin/env -S uv run --active --script

import shutil
import io
import os
import signal
import sys
import time
import traceback
import threading
import json
from pathlib import Path
import contextlib
import queue
from PySide6.QtCore import QTimer, Qt, Signal
from PySide6.QtGui import QPixmap, QColorConstants
from PySide6.QtWidgets import QApplication, QWidget, QVBoxLayout, QHBoxLayout, QPushButton, QTextEdit, QAbstractSlider, QLabel, QSplashScreen
from tinkerforge_util import colored
from tinkerforge_util.colored import red, green
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'provisioning')

from provisioning.tinkerforge.ip_connection import IPConnection, Error
from provisioning.tinkerforge.bricklet_industrial_dual_relay import BrickletIndustrialDualRelay
from provisioning.tinkerforge.bricklet_rgb_led_v2 import BrickletRGBLEDV2
from provisioning.tinkerforge.bricklet_temperature_v2 import BrickletTemperatureV2
from provisioning.tinkerforge.bricklet_piezo_speaker_v2 import BrickletPiezoSpeakerV2
from provisioning.tinkerforge.bricklet_led_strip_v2 import BrickletLEDStripV2
from provisioning.tinkerforge.bricklet_rgb_led_button import BrickletRGBLEDButton
from provisioning.tinkerforge.bricklet_industrial_dual_analog_in_v2 import BrickletIndustrialDualAnalogInV2
from provisioning.tinkerforge.bricklet_io4_v2 import BrickletIO4V2
from provisioning.tinkerforge.brick_master import BrickMaster
from provisioning.provision_common.provision_common import *
from provisioning.ntpserver import start_ntpserver

from provisioning.xmc_flash_bootloader import xmc_flash_bootloader

class ThreadWithReturnValue(threading.Thread):
    def __init__(self, group=None, target=None, name=None,
                    args=(), kwargs={}, Verbose=None):
        super().__init__(group, target, name, args, kwargs)
        self._return = None
        self._success = False
        self._exception = None
        self.start_semaphore = threading.Semaphore(0)

    def run(self):
        self.start_semaphore.acquire()
        if self._target is not None:
            try:
                self._return = self._target(*self._args, **self._kwargs)
                self._success = True
            except BaseException as e:
                self._exception = e

    def join(self, *args):
        super().join(*args)
        return self._success, self._return, self._exception

def excepthook(cls, exception, traceback):
    P.original_stderr.write("{}".format(exception))
    P.original_stderr.flush()

sys.excepthook = excepthook

def dbg(*args):
    P.original_stdout.write(" ".join([str(x) for x in args]))
    P.original_stdout.flush()

def slot_except_hook(fn):
    def inner():
        try:
            fn()
        except:
            P.original_stderr.write(traceback.format_exc())
            P.original_stderr.flush()

    return inner

class P:
    main_thread = threading.get_ident()
    quit_requested = False
    original_stdout = sys.stdout
    original_stderr = sys.stderr

    logs = {
        -1: [io.StringIO(""), io.StringIO("")],
        0: [io.StringIO(""), io.StringIO("")],
        1: [io.StringIO(""), io.StringIO("")],
        2: [io.StringIO(""), io.StringIO("")],
        3: [io.StringIO(""), io.StringIO("")]
    }

    thread_ids = {}

    idrs: (BrickletIndustrialDualRelay, BrickletIndustrialDualRelay) = (None, None)
    idais: (BrickletIndustrialDualAnalogInV2, BrickletIndustrialDualAnalogInV2) = (None, None)
    btns: (BrickletRGBLEDButton, BrickletRGBLEDButton, BrickletRGBLEDButton, BrickletRGBLEDButton) = (None, None, None, None)
    ps: BrickletPiezoSpeakerV2 = None
    ls: BrickletLEDStripV2 = None
    io4: BrickletIO4V2 = None

    master_uids = [
        None,
        "6nhqFd",
        "6a2ZWs",
        "6nhpsA"
    ]

    last_edits_content = {}

    testers_connected = 0


    class StdoutWrapper:
        def __getattribute__(self, name):
            if threading.get_ident() == P.main_thread or threading.get_ident() not in P.thread_ids:
                f = P.logs[-1][0]
            else:
                f = P.logs[P.thread_ids[threading.get_ident()]][0]

            return f.__getattribute__(name)

        def __setattr__(self, name, value):
            if threading.get_ident() == P.main_thread or threading.get_ident() not in P.thread_ids:
                f = P.logs[-1][0]
            else:
                f = P.logs[P.thread_ids[threading.get_ident()]][0]

            return f.__setattr__(name, value)

    class StderrWrapper:
        def __getattribute__(self, name):
            if threading.get_ident() == P.main_thread or threading.get_ident() not in P.thread_ids:
                f = P.logs[-1][1]
            else:
                f = P.logs[P.thread_ids[threading.get_ident()]][1]

            return f.__getattribute__(name)

        def __setattr__(self, name, value):
            if threading.get_ident() == P.main_thread or threading.get_ident() not in thread_ids:
                f = P.logs[-1][1]
            else:
                f = P.logs[thread_ids[threading.get_ident()]][1]

            return f.__setattr__(name, value)

    def test_bricklet_ports_warp4(ipcon):
        enums = enumerate_devices(ipcon)

        expected = {
            'A': BrickletTemperatureV2.DEVICE_IDENTIFIER,
            'B': BrickletTemperatureV2.DEVICE_IDENTIFIER,
            'C': BrickletTemperatureV2.DEVICE_IDENTIFIER,
            'D': BrickletTemperatureV2.DEVICE_IDENTIFIER,
            'E': BrickletTemperatureV2.DEVICE_IDENTIFIER,
            'F': 2184, # WARP ESP32 Ethernet 2.0 Co
        }

        if len(enums) != len(expected):
            fatal_error("Expected {} bricklets but found {}".format(len(expected), "\n\t".join("Port {}: {}".format(x.position, x.device_identifier) for x in enums)))

        for x in enums:
            if expected[x.position] != x.device_identifier:
                fatal_error("Expected {} at port {} but found {}".format(expected[x.position], x.position, "\n\t".join("Port {}: {}".format(x.position, x.device_identifier) for x in enums)))
            expected.pop(x.position)

        # This should never happen, as we have 4 expected bricklets and 4 enumerations
        if len(expected) > 0:
            fatal_error("Expected other bricklets but found {}".format("\n\t".join("Port {}: {}".format(x.position, x.device_identifier) for x in enums)))

        enums = sorted(enums, key=lambda x: x.position)

        bricklets = {enum.position: BrickletTemperatureV2(enum.uid, ipcon) for enum in enums if enum.device_identifier == BrickletTemperatureV2.DEVICE_IDENTIFIER}
        error_count = 0

        temps = []

        for port, bricklet in bricklets.items():
            bricklet.set_response_expected_all(True)

            try:
                temps.append(bricklet.get_temperature() / 100)
            except Exception as e:
                print("Failed to communicate with bricklet on port {}: {}".format(port, e))
                error_count += 1

        if error_count != 0:
            fatal_error("")

        return sum(temps) / len(temps)

    def connect_ethernet(ip):
        print(f"Connecting via ethernet to {ip}", end="")
        for i in range(30):
            start = time.time()
            try:
                with urllib.request.urlopen(f"http://{ip}/info/modules", timeout=1) as f:
                    f.read()
                    break
            except:
                pass
            t = max(0, 1 - (time.time() - start))
            time.sleep(t)
            print(".", end="")
        else:
            fatal_error("Failed to connect via ethernet!")

        print(" Connected.")

    def test_rtc_time(ip, wait_for_ntp):
        print("Testing RTC")
        if wait_for_ntp:
            print("    Waiting for NTP sync")
            for i in range(30):
                start = time.time()
                try:
                    with urllib.request.urlopen(f"http://{ip}/ntp/state", timeout=1) as f:
                        if json.loads(f.read())["synced"]:
                            break
                except:
                    pass
                t = max(0, 1 - (time.time() - start))
                time.sleep(t)
                print(".", end="")
            else:
                fatal_error("NTP did not sync in 30 seconds!")

        try:
            with urllib.request.urlopen(f"http://{ip}/rtc/time", timeout=10) as f:
                t = json.loads(f.read())
                esp_time = datetime.datetime(t["year"], t["month"], t["day"], t["hour"], t["minute"], t["second"], tzinfo=datetime.timezone.utc)
        except Exception as e:
            print(e)
            fatal_error("Failed to read RTC time!")

        print(esp_time, datetime.datetime.now(datetime.timezone.utc))

        if (esp_time - datetime.datetime.now(datetime.timezone.utc)).total_seconds() > 3:
            fatal_error("RTC time wrong!")

    def get_esp_ssid(serial_port, result, ssid_prefix):
        print("Checking ESP state")
        mac_address = check_if_esp_is_sane_and_get_mac(allowed_revision=[3.0, 3.1], override_port=serial_port)
        print("MAC Address is {}".format(mac_address))
        result["mac"] = mac_address

        set_voltage_fuses, set_block_3, passphrase, uid = get_espefuse_tasks(override_port=serial_port, secure_mode=True, mac_address=mac_address)
        if set_voltage_fuses or set_block_3:
            fatal_error("Fuses are not set. Re-run stage 0!")

        esptool(["--after", "hard-reset", "run"], override_port=serial_port)

        result["uid"] = uid

        ssid = f"{ssid_prefix}-{uid}"

        return ssid, passphrase

    def test_wifi(ssid, passphrase, host_ip, ethernet_ip, ethernet_gateway, ethernet_subnet, ethernet_dns, result):
        print("Waiting for ESP wifi. Takes about one minute.")
        if not wait_for_wifi(ssid, 90):
            fatal_error("ESP wifi not found after 90 seconds")

        print("Testing ESP Wifi.")
        with wifi(ssid, passphrase):
            req = urllib.request.Request("http://10.0.0.1/ethernet/config_update",
                                        data=json.dumps({"enable_ethernet": True,
                                                        "ip": ethernet_ip,
                                                        "gateway": ethernet_gateway,
                                                        "subnet": ethernet_subnet,
                                                        "dns": ethernet_dns,
                                                        "dns2": "0.0.0.0"}).encode("utf-8"),
                                        method='PUT',
                                        headers={"Content-Type": "application/json"})
            try:
                with urllib.request.urlopen(req, timeout=10) as f:
                    f.read()
            except Exception as e:
                print(e)
                fatal_error("Failed to set ethernet config!")

            req = urllib.request.Request("http://10.0.0.1/ntp/config_update",
                                        data=json.dumps({"enable": None,
                                                        "use_dhcp": False,
                                                        "timezone": None,
                                                        "server": host_ip,
                                                        "server2": None}).encode("utf-8"),
                                        method='PUT',
                                        headers={"Content-Type": "application/json"})
            try:
                with urllib.request.urlopen(req, timeout=10) as f:
                    f.read()
            except Exception as e:
                print(e)
                fatal_error("Failed to set NTP config!")

            result["wifi_test_successful"] = True

    def run_stage_1_tests(serial_port, ethernet_ip, power_off_fn, power_on_fn, result):
        P.connect_ethernet(ethernet_ip)

        req = urllib.request.Request(f"http://{ethernet_ip}/info/version")
        try:
            with urllib.request.urlopen(req, timeout=10) as f:
                fw_version = json.loads(f.read().decode("utf-8"))["firmware"].split("-")[0].split("+")[0]
        except Exception as e:
            traceback.print_exc()
            fatal_error("Failed to read firmware version!")

        try:
            with urllib.request.urlopen(f"http://{ethernet_ip}/hidden_proxy/enable", timeout=10) as f:
                f.read()
        except Exception as e:
            traceback.print_exc()
            fatal_error("Failed to enable hidden_proxy!")

        time.sleep(3)
        ipcon = IPConnection()
        ipcon.connect(ethernet_ip, 4223)
        result["ethernet_test_successful"] = True
        print("Connected. Testing bricklet ports")

        avg_bricklet_temp = P.test_bricklet_ports_warp4(ipcon)
        result["bricklet_port_test_successful"] = True

        ipcon.disconnect()

        print("Testing PLC modem connection")
        try:
            with urllib.request.urlopen(f"http://{ethernet_ip}/iso15118/state_slac/modem_found", timeout=10) as f:
                if not json.loads(f.read()):
                    fatal_error("PLC modem not found!")
        except Exception as e:
            traceback.print_exc()
            fatal_error("Failed to read 'PLC modem found' API!")

        print("Testing temperature sensor")
        try:
            with urllib.request.urlopen(f"http://{ethernet_ip}/esp32/temperature", timeout=10) as f:
                esp_temp = float(json.loads(f.read())["temperature"]) / 100
        except Exception as e:
            traceback.print_exc()
            fatal_error("Failed to read temperature value!")

        if abs(avg_bricklet_temp - esp_temp) > 15:
            fatal_error(f"ESP temperature sensor value not in expected range: {esp_temp=} {avg_bricklet_temp=}")

        result["temperature_test_successful"] = True

        P.test_rtc_time(ethernet_ip, wait_for_ntp=True)

        # Wait to be sure the ESP has updated the RTC
        time.sleep(2)

        print("Testing RTC supercap")

        req = urllib.request.Request(f"http://{ethernet_ip}/ntp/config_update",
                                    data=json.dumps({"enable":False,
                                                    "use_dhcp":None,
                                                    "timezone":None,
                                                    "server":None,
                                                    "server2":None}).encode("utf-8"),
                                    method='PUT',
                                    headers={"Content-Type": "application/json"})
        try:
            with urllib.request.urlopen(req, timeout=1) as f:
                f.read()
        except:
            traceback.print_exc()
            fatal_error("Failed to disable NTP")

        power_off_fn()
        time.sleep(10)
        power_on_fn()

        P.connect_ethernet(ethernet_ip)

        P.test_rtc_time(ethernet_ip, wait_for_ntp=False)

        result["rtc_test_successful"] = True

        req = urllib.request.Request(f"http://{ethernet_ip}/config/reset",
                                    data=json.dumps("ntp/config").encode("utf-8"),
                                    method='PUT',
                                    headers={"Content-Type": "application/json"})
        try:
            with urllib.request.urlopen(req, timeout=1) as f:
                f.read()
        except:
            traceback.print_exc()
            fatal_error("Failed to re-enable NTP")

        try:
            with urllib.request.urlopen(f"http://{ethernet_ip}/hidden_proxy/enable", timeout=10) as f:
                f.read()
        except Exception as e:
            traceback.print_exc()
            fatal_error("Failed to enable hidden_proxy!")

        time.sleep(3)

        ipcon.connect(ethernet_ip, 4223)

        result["tests_successful"] = True

    def encrypt_and_secure(serial_port, ethernet_ip, result):
        print("Securing device")
        req = urllib.request.Request(f"http://{ethernet_ip}/esp32/secure_device",
                                    data=json.dumps({"destroy_my_data":True, "deterministic":False}).encode("utf-8"),
                                    method='PUT',
                                    headers={"Content-Type": "application/json"})
        try:
            with urllib.request.urlopen(req, timeout=120) as f:
                f.read()
        except urllib.request.HTTPError as e:
            if e.code != 404:
                traceback.print_exc()
                fatal_error(f"Failed to secure device {e.code=} {e.read().decode('utf-8')}")
        except:
            traceback.print_exc()
            fatal_error("Failed to secure device")

        time.sleep(5)
        P.connect_ethernet(ethernet_ip)

        print("Encrypting data")
        req = urllib.request.Request(f"http://{ethernet_ip}/esp32/encrypt_data")
        try:
            with urllib.request.urlopen(req, timeout=60) as f:
                f.read()
        except urllib.request.HTTPError as e:
            if e.code != 404:
                traceback.print_exc()
                fatal_error(f"Failed to encrypt data {e.code=} {e.read().decode('utf-8')}")
        except:
            traceback.print_exc()
            fatal_error("Failed to encrypt data")

        print("Checking secure boot")
        req = urllib.request.Request(f"http://{ethernet_ip}/esp32/check_sbv2")
        try:
            with urllib.request.urlopen(req, timeout=30) as f:
                check_sbv2_result = f.read()
        except:
            traceback.print_exc()
            fatal_error("Failed to check secure boot")

        if check_sbv2_result.decode('utf-8') != "Success":
            fatal_error("Failed to check secure boot")

        print("Erasing other app partition")
        req = urllib.request.Request(f"http://{ethernet_ip}/firmware_update/erase_other")
        try:
            with urllib.request.urlopen(req, timeout=10) as f:
                f.read()
        except:
            traceback.print_exc()
            fatal_error("Failed to erase other partition")


        import socket
        if socket.gethostname() != "warp4-esp-tester":
            print("Skipping lock down!")
            result["locked_down"] = False
        else:
            print("Locking down device")
            req = urllib.request.Request(f"http://{ethernet_ip}/esp32/lockdown")
            try:
                with urllib.request.urlopen(req, timeout=30) as f:
                    f.read()
            except urllib.request.HTTPError as e:
                if e.code != 404:
                    traceback.print_exc()
                    fatal_error("Failed to lock down device")
            except:
                traceback.print_exc()
                fatal_error("Failed to lock down device")
            result["locked_down"] = True

        req = urllib.request.Request(f"http://{ethernet_ip}/config/reset",
                                    data=json.dumps("ethernet/config").encode("utf-8"),
                                    method='PUT',
                                    headers={"Content-Type": "application/json"})
        try:
            with urllib.request.urlopen(req, timeout=1) as f:
                f.read()
        except:
            traceback.print_exc()
            fatal_error("Failed to re-enable ethernet DHCP")


        print("Done")

    def print_label(ssid, passphrase):
        run(["uv", "run", "print-esp32-label.py", ssid, passphrase, "-c", "3"])

    def terminal_to_html(s):
        mapping = {
            'red':   '#FF0000',
            'green': '#00AA00',
            'blue':  '#0000FF',
            'cyan':  '#00FFFF',
            'gray':  '#555555',
        }

        result = '#FFFFFF'

        for color in colored.findall(s):
            if color in mapping:
                result = mapping[color]

        return colored.strip(s).replace('\n', '<br/>'), result

    def reprint_label():
        P.reprint_clicked = True

    def restart(app):
        P.restart_clicked = True
        P.original_stdout.write("vor disconnect")
        app.aboutToQuit.disconnect(quit)
        P.original_stdout.write("nach disconnect")
        app.quit()

    def update_logs(app, edits):
        if P.quit_requested:
            app.quit()
            return

        for k, v in edits.items():
            new_log, back_color = P.terminal_to_html(P.logs[k][0].getvalue() + "\n---\n" + P.logs[k][1].getvalue())

            if P.last_edits_content.get(k) == (new_log, back_color):
                continue

            P.last_edits_content[k] = (new_log, back_color)
            v.setHtml(new_log)
            v.setStyleSheet(f"background-color: {back_color};")
            v.verticalScrollBar().triggerAction(QAbstractSlider.SliderToMaximum)

    def quit():
        os.killpg(0, signal.SIGKILL)
        #os._exit(42 if P.restart_clicked else 0)

    def run_gui(q: queue.Queue):
        try:
            app = QApplication([])

            splash = QSplashScreen(QPixmap("./favicon_512.png"))
            splash.showMessage("Hallo Welt!", Qt.AlignCenter | Qt.AlignBottom, QColorConstants.White)
            splash.show()
            while True:
                app.processEvents()
                try:
                    x = q.get(timeout=0.1)
                    if isinstance(x, str):
                        splash.showMessage(x, Qt.AlignCenter | Qt.AlignBottom, QColorConstants.White)
                    else:
                        testers = x
                        break
                except queue.Empty:
                    pass

            app.aboutToQuit.connect(slot_except_hook(P.quit))

            window = QWidget()
            window.setWindowTitle('WARP ESP32 Provisioning')
            layout = QVBoxLayout()

            tester_widget = QWidget()
            tester_layout = QHBoxLayout()

            if len(testers) == 0:
                tester_layout.addWidget(QLabel("NO ESP TESTERS ATTACHED!"))

            edits = {
                -1: QTextEdit()
            }
            edits[-1].setReadOnly(True)

            for i in range(4):
                if i in testers:
                    edits[i] = QTextEdit()
                    edits[i].setReadOnly(True)
                    tester_layout.addWidget(edits[i])
                else:
                    tester_layout.addWidget(QLabel(f"Tester {i} not attached"))

            tester_widget.setLayout(tester_layout)

            layout.addWidget(tester_widget)

            layout.addWidget(edits[-1])

            window.setLayout(layout)
            window.show()
            splash.finish(window)

            log_timer = QTimer(window)
            log_timer.timeout.connect(slot_except_hook(lambda: P.update_logs(app, edits)))
            log_timer.setInterval(100)
            log_timer.start()

            app.exec()
        except Exception:
            P.original_stdout.write(traceback.format_exc())
            P.original_stdout.flush()

    blue = [0, 0, 255]
    green = [0, 255, 0]
    yellow = [255, 128, 0]
    red = [255, 0, 0]

    def set_progress(i, val, color):
        P.ls.set_led_values((24 - ((i - 1)  * 8 + val) - 1) * 3, [c * 0.33 for c in color])


    def set_relay(i, val):
        P.idrs[i // 2].set_selected_value(i % 2, val)

    def main():
        os.setpgrp()

        q = queue.Queue()

        try:
            qt_thread = ThreadWithReturnValue(target=lambda: P.run_gui(q))
            qt_thread.start()
            P.thread_ids[qt_thread.ident] = -1
            qt_thread.start_semaphore.release()

            q.put("Connecting to Brick Daemon")

            ipcon = IPConnection()
            ipcon.connect("localhost", 4223)

            q.put(f"Searching {BrickletIndustrialDualRelay.DEVICE_DISPLAY_NAME}s, {BrickletRGBLEDButton.DEVICE_DISPLAY_NAME}s, {BrickletPiezoSpeakerV2.DEVICE_DISPLAY_NAME} and {BrickletLEDStripV2.DEVICE_DISPLAY_NAME}")

            idr_uids = [None, None]
            idai_uids = [None, None]
            btn_uids = [None, None, None]
            ps_uid = None
            ls_uid = None
            io4_uid = None
            master_uids_cpy = [x for x in P.master_uids if x is not None]


            def search_devices(uid, connected_uid, position, hardware_version, firmware_version,
                        device_identifier, enumeration_type):
                nonlocal idr_uids
                nonlocal idai_uids
                nonlocal btn_uids
                nonlocal ps_uid
                nonlocal ls_uid
                nonlocal io4_uid
                nonlocal master_uids_cpy
                if device_identifier == BrickletIndustrialDualRelay.DEVICE_IDENTIFIER:
                    idr_uids[ord(position.upper()) - ord('A')] = uid
                if device_identifier == BrickletIndustrialDualAnalogInV2.DEVICE_IDENTIFIER:
                    idai_uids[ord(position.upper()) - ord('C')] = uid

                if device_identifier == BrickletRGBLEDButton.DEVICE_IDENTIFIER:
                    # HACK: because this script is forked from provision_warp_esp32_ethernet, we support up to 4 testers.
                    # The new test device is one with exactly 3 testers, but wired as if tester 0 is not connected and testers 1 to 3 are.
                    # We still want len(btn_uids) to be three so that the check below expects 3 buttons for now.
                    btn_uids[ord(position.upper()) - ord('A') - 1] = uid

                if device_identifier == BrickletPiezoSpeakerV2.DEVICE_IDENTIFIER:
                    ps_uid = uid

                if device_identifier == BrickletLEDStripV2.DEVICE_IDENTIFIER:
                    ls_uid = uid

                if device_identifier == BrickletIO4V2.DEVICE_IDENTIFIER:
                    io4_uid = uid

                if device_identifier == BrickMaster.DEVICE_IDENTIFIER:
                    try:
                        master_uids_cpy.remove(uid)
                    except ValueError:
                        pass

            def check_bricklets(fail_if_missing=False):
                if any(x is None for x in idr_uids):
                    if fail_if_missing:
                        fatal_error(f"Not exactly {len(idr_uids)} {BrickletIndustrialDualRelay.DEVICE_DISPLAY_NAME}s found.")
                    return False
                if any(x is None for x in idai_uids):
                    if fail_if_missing:
                        fatal_error(f"Not exactly {len(idai_uids)} {BrickletIndustrialDualAnalogInV2.DEVICE_DISPLAY_NAME}s found.")
                    return False
                if any(x is None for x in btn_uids):
                    if fail_if_missing:
                        fatal_error(f"Not exactly {len(btn_uids)} {BrickletRGBLEDButton.DEVICE_DISPLAY_NAME}s found.")
                    return False
                elif ps_uid is None:
                    if fail_if_missing:
                        fatal_error(f"{BrickletPiezoSpeakerV2.DEVICE_DISPLAY_NAME} not found.")
                    return False
                elif ls_uid is None:
                    if fail_if_missing:
                        fatal_error(f"{BrickletLEDStripV2.DEVICE_DISPLAY_NAME} not found.")
                    return False
                elif io4_uid is None:
                    if fail_if_missing:
                        fatal_error(f"{BrickletIO4V2.DEVICE_DISPLAY_NAME} not found.")
                    return False
                elif len(master_uids_cpy) > 0:
                    if fail_if_missing:
                        fatal_error(f"{BrickMaster.DEVICE_DISPLAY_NAME}{"s" if len(master_uids_cpy) > 1 else ""} with hardcoded UID {master_uids_cpy} not found.")
                return True


            start = time.time()
            ipcon.register_callback(IPConnection.CALLBACK_ENUMERATE, search_devices)
            for i in range(100):
                if check_bricklets(False):
                    break

                if i % 10 == 0:
                    ipcon.enumerate()
                time.sleep(0.1)
            else:
                check_bricklets(True)
            del i

            P.idrs = (BrickletIndustrialDualRelay(idr_uids[0], ipcon), BrickletIndustrialDualRelay(idr_uids[1], ipcon))
            for idr in P.idrs:
                idr.set_response_expected_all(True)

            P.idais = (BrickletIndustrialDualAnalogInV2(idai_uids[0], ipcon), BrickletIndustrialDualAnalogInV2(idai_uids[1], ipcon))
            for idai in P.idais:
                idai.set_response_expected_all(True)

            P.btns = (None, BrickletRGBLEDButton(btn_uids[0], ipcon), BrickletRGBLEDButton(btn_uids[1], ipcon), BrickletRGBLEDButton(btn_uids[2], ipcon))
            for btn in P.btns:
                if btn is None:
                    continue
                btn.set_response_expected_all(True)

            P.ps = BrickletPiezoSpeakerV2(ps_uid, ipcon)
            P.ps.set_response_expected_all(True)

            P.ls = BrickletLEDStripV2(ls_uid, ipcon)
            P.ls.set_response_expected_all(True)
            P.ls.set_chip_type(BrickletLEDStripV2.CHIP_TYPE_WS2812)
            P.ls.set_channel_mapping(BrickletLEDStripV2.CHANNEL_MAPPING_GRB)
            P.ls.set_led_values(0, [0, 0, 0] * 24)

            P.io4 = BrickletIO4V2(io4_uid, ipcon)
            P.io4.set_response_expected_all(True)

            q.put("Powering off testers")

            for id_ in P.idrs:
                id_.set_value(False, False)
            time.sleep(1)

            q.put("Powering on testers")

            for id_ in P.idrs:
                id_.set_value(True, True)
            time.sleep(3)

            relay_to_serial = {k: f"/dev/ttyUSBESPTESTER{k}" for k in range(4) if os.path.exists(f"/dev/ttyUSBESPTESTER{k}")}

            P.testers_connected = len(relay_to_serial)

            q.put(f"Found {P.testers_connected} testers. Starting GUI.")

            q.put(list(relay_to_serial.keys()))
        except Exception:
            q.put([])
            raise

        test_report_pull()

        config = json.loads(Path("provision_warp_esp32_ethernet_v2.config").read_text())
        ssid_prefix = config["ssid_prefix"] # warp4
        firmware_prefix = config["firmware_prefix"] # warp4
        product_name = config["product_name"] # WARP4 Charger
        host_ip = config["host_ip"]
        static_ips = config["static_ips"]
        subnet = config["subnet"]
        gateway = config["gateway"]
        dns = config["dns"]

        print("Starting NTP server")
        threading.Thread(target=start_ntpserver, args=("0.0.0.0", 1234)).start()

        if P.io4.get_value()[0]:
            print("Close lid!")

        while P.io4.get_value()[0]:
            time.sleep(0.1)

        time.sleep(1)

        stage = 0

        threads = []
        print(f"Checking secure boot state of {len(relay_to_serial)} ESPs")

        for k, v in relay_to_serial.items():
            # lambda with default parameter value to fix the late binding issue.
            # If v was used directly, it would behave as if "captured by reference"
            # -> fun with multithreading.
            t = ThreadWithReturnValue(target=lambda port=v:
                                          espefuse(["summary", "--format", "value_only", "ABS_DONE_1"], override_port=port))
            t.start()
            P.thread_ids[t.ident] = k
            t.start_semaphore.release()
            threads.append((k, t))
            P.set_progress(k, stage, P.blue)

        already_secured_esps = {}
        for k, t in threads:
            success, result, exception = t.join()

            if not success:
                print(f"Failed to check secure boot state of ESP{k}. {result} {exception}", file=P.logs[k][1])
                relay_to_serial.pop(k)
                P.set_progress(k, stage, P.red)
            elif "True" in result:
                print(f"Secure boot already active. Skipping to tests", file=P.logs[k][1])
                already_secured_esps[k] = relay_to_serial.pop(k)
                P.set_progress(k, stage, P.yellow)
            else:
                P.set_progress(k, stage, P.green)

        threads.clear()

        stage += 1

        print(f"Flashing bootloader to {len(relay_to_serial)} Co-Bricklets")

        for idr in P.idrs:
            idr.set_value(False, False)
        time.sleep(1)

        for k, v in (relay_to_serial | already_secured_esps).items():
            # lambda with default parameter value to fix the late binding issue.
            # If v was used directly, it would behave as if "captured by reference"
            # -> fun with multithreading.
            t = ThreadWithReturnValue(target=lambda uid=P.master_uids[k]:
                                          xmc_flash_bootloader(f'../../firmwares/bricklets/warp_esp32_ethernet_v2_co/bricklet_warp_esp32_ethernet_v2_co_firmware_latest.zbin', uid))
            t.start()
            P.thread_ids[t.ident] = k
            t.start_semaphore.release()
            threads.append((k, t))
            P.set_progress(k, stage, P.blue)

        failed_bricklet_bootloader_flashs = []
        for k, t in threads:
            success, result, exception = t.join()
            if not success:
                print(f"Failed to flash bootloader to co-bricklet {k}. Assuming already flashed {result} {exception}", file=P.logs[k][1])
                failed_bricklet_bootloader_flashs.append(i)
                P.set_progress(k, stage, P.yellow)
            else:
                P.set_progress(k, stage, P.green)

        threads.clear()

        for idr in P.idrs:
            idr.set_value(True, True)

        print(f"Flashing {len(relay_to_serial)} ESPs...")

        fws = list(Path(".").glob(f"{firmware_prefix}_firmware-PROVISIONING*_esptool.bin"))
        if len(fws) != 1:
            fatal_error(f"Expected exactly one {firmware_prefix}_firmware-PROVISIONING*_esptool.bin, but found {fws}")
        fw = fws[0]

        stage += 1
        for k, v in relay_to_serial.items():
            # lambda with default parameter value to fix the late binding issue.
            # If v was used directly, it would behave as if "captured by reference"
            # -> fun with multithreading.
            t = ThreadWithReturnValue(target=lambda port=v:
                    subprocess.run(
                        [sys.executable, 'provision_stage_0_warp2.py', fw, port, firmware_prefix],
                        capture_output=True,
                        encoding='utf-8'))
            t.start()
            P.thread_ids[t.ident] = k
            t.start_semaphore.release()
            threads.append((k, v, t))
            P.set_progress(k, stage, P.blue)

        for k, v, t in threads:
            success, result, exception = t.join()
            if not success:
                print(red(f"Failed to run stage 0 for {k} {v}: {exception}"), file=P.logs[k][1])
                relay_to_serial.pop(k)
                P.set_progress(k, stage, P.red)
            elif result.returncode != 0:
                print(red(f"Failed to run stage 0 for {k} {v}: {result.stdout}\n{result.stderr}"), file=P.logs[k][1])
                relay_to_serial.pop(k)
                P.set_progress(k, stage, P.red)
            else:
                P.set_progress(k, stage, P.green)

        threads.clear()

        print(f"{len(relay_to_serial)} ESPs flashed successfully")

        relay_to_serial.update(already_secured_esps)

        def get_esp_ssid_fn(serial_port, result):
            return lambda: P.get_esp_ssid(serial_port, result, ssid_prefix)

        test_reports = {k: {"start": now()} for k in relay_to_serial.keys()}

        relay_to_ssid = {}
        relay_to_passphrase = {}

        stage += 1
        for k, v in relay_to_serial.items():
            t = ThreadWithReturnValue(target=get_esp_ssid_fn(v, test_reports[k]))
            t.start()
            P.thread_ids[t.ident] = k
            t.start_semaphore.release()
            threads.append((k, v, t))
            P.set_progress(k, stage, P.blue)

        for k, v, t in threads:
            success, result, exception = t.join()
            if not success:
                print(red(f"Failed to run get_esp_ssid for {k} {v}: {exception}"), file=P.logs[k][1])
                relay_to_serial.pop(k)
                P.set_progress(k, stage, P.red)
            else:
                relay_to_ssid[k] = result[0]
                relay_to_passphrase[k] = result[1]
                P.set_progress(k, stage, P.green)

        print(str(relay_to_ssid))

        threads.clear()

        stage += 1

        for k, v in list(relay_to_serial.items()):
            P.set_progress(k, stage, P.blue)

        run(["sudo", "systemctl", "restart", "NetworkManager.service"])
        if shutil.which("iw") is not None:
            run(["sudo", "iw", "reg", "set", "DE"])

        for k, v in list(relay_to_serial.items()):
            with contextlib.redirect_stdout(P.logs[k][0]):
                with contextlib.redirect_stdout(P.logs[k][1]):
                    try:
                        P.test_wifi(relay_to_ssid[k], relay_to_passphrase[k], host_ip, static_ips[k], gateway, subnet, dns, test_reports[k])
                    except BaseException as e:
                        print(red(f"Failed to test WiFi for {k} {v}: {e}"), file=P.logs[k][1])
                        relay_to_serial.pop(k)
                        P.set_progress(k, stage, P.red)
                    else:
                        P.set_progress(k, stage, P.green)

        stage += 1

        failed = []
        for k, v in relay_to_serial.items():
            voltage = P.idais[(k - 1) // 2].get_voltage((k - 1) % 2)
            expected = 3300 # mV
            min_e = expected * 0.95
            max_e = expected * 1.05

            test_reports[k]["voltage"] = voltage

            if min_e <= voltage <= max_e:
                print(f"Voltage is {voltage / 1000}", file=P.logs[k][1])
                P.set_progress(k, stage, P.green)
            else:
                print(f"Voltage {voltage / 1000} V out of range [{min_e/1000} V{max_e/1000}] V", file=P.logs[k][1])
                P.set_progress(k, stage, P.red)
                failed.append(k)

        for k in failed:
            relay_to_serial.pop(k)

        threads.clear()

        stage += 1

        def run_stage_1_tests_fn(serial_port, ethernet_ip, relay_pin, test_report):
            return lambda: P.run_stage_1_tests(serial_port, ethernet_ip, lambda: P.set_relay(relay_pin, False), lambda: P.set_relay(relay_pin, True), test_report)

        for k, v in relay_to_serial.items():
            print(f"{k}: {static_ips[k]}")
            t = ThreadWithReturnValue(target=run_stage_1_tests_fn(v, static_ips[k], k, test_reports[k]))
            t.start()
            P.thread_ids[t.ident] = k
            t.start_semaphore.release()
            threads.append((k, v, t))
            P.set_progress(k, stage, P.blue)

        for k, v, t in threads:
            success, result, exception = t.join()
            if not success:
                P.set_progress(k, stage, P.red)
                print(red(f"Failed to run run_stage_1_tests for {k} {v}: {exception}"), file=P.logs[k][1])
                relay_to_serial.pop(k)
            else:
                P.set_progress(k, stage, P.green)

        threads.clear()

        def encrypt_and_secure_fn(serial_port, ethernet_ip, relay_pin, test_report):
            return lambda: P.encrypt_and_secure(serial_port, ethernet_ip, test_report)

        stage += 1
        for k, v in relay_to_serial.items():
            print(f"{k}: {static_ips[k]}")
            t = ThreadWithReturnValue(target=encrypt_and_secure_fn(v, static_ips[k], k, test_reports[k]))
            t.start()
            P.thread_ids[t.ident] = k
            t.start_semaphore.release()
            threads.append((k, v, t))
            P.set_progress(k, stage, P.blue)

        for k, v, t in threads:
            success, result, exception = t.join()
            if not success:
                print(red(f"Failed to run encrypt_and_secure for {k} {v}: {exception}"), file=P.logs[k][1])
                relay_to_serial.pop(k)
                P.set_progress(k, stage, P.red)
            else:
                P.set_progress(k, stage, P.green)
                test_reports[k]["end"] = now()

        threads.clear()

        P.ps.set_beep(262, 0, 250)
        time.sleep(0.25)
        P.ps.set_beep(330, 0, 250)
        time.sleep(0.25)
        P.ps.set_beep(392, 0, 250)
        time.sleep(0.25)
        P.ps.set_beep(523, 0, 1000)

        if not P.io4.get_value()[0]:
            print("Open lid!")

        while not P.io4.get_value()[0]:
            time.sleep(0.1)

        time.sleep(1)

        for k in relay_to_serial.keys():
            report_path = os.path.join(TEST_REPORTS_DIRECTORY, firmware_prefix, "{}_{}_report_stage_1.json".format(relay_to_ssid[k], now().replace(":", "-")))

            with mkdir_open(report_path, "w") as f:
                json.dump(test_reports[k], f, indent=4)

            test_report_commit_and_push(f'Add stage 1 test report for {product_name} with UID {relay_to_ssid[k].split("-")[-1]}', [report_path])

        while P.io4.get_value()[0]:
            print(green(f"ESPs in testers {', '.join(str(x) for x in relay_to_serial.keys())} tested successfully. Press one of the print buttons!"))

            relay_to_pressed = {}

            while not any(relay_to_pressed.values()) and P.io4.get_value()[0]:
                time.sleep(0.01)
                relay_to_pressed = {k: (P.btns[k].get_button_state() == BrickletRGBLEDButton.BUTTON_STATE_PRESSED) for k, v in relay_to_serial.items()}

            for k, p in relay_to_pressed.items():
                if not p:
                    continue

                print(f"Printing labels for ESP in tester {k}. Stick one label on the ESP, put ESP in the ESD bag. Press button again to retry printing the labels. Press next button to continue.")
                P.print_label(relay_to_ssid[k], relay_to_passphrase[k])
                break

        P.original_stdout.write("before join\n")
        P.original_stdout.flush()
        P.quit_requested = True
        qt_thread.join()

        result = 42
        P.original_stdout.write("returning" + str(result) + "\n")
        P.original_stdout.flush()
        return result

if __name__ == "__main__":
    with contextlib.redirect_stdout(P.StdoutWrapper()):
        with contextlib.redirect_stderr(P.StderrWrapper()):
            try:
                os._exit(P.main())
            except Exception:
                P.logs[-1][1].write(traceback.format_exc())
                raise
