#!/usr/bin/python3 -u

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'provisioning')

from PySide6.QtCore import QTimer, Qt
from PySide6.QtGui import QPixmap, QColorConstants
from PySide6.QtWidgets import QApplication, QWidget, QVBoxLayout, QHBoxLayout, QPushButton, QComboBox, QTextEdit, QAbstractSlider, QLabel, QSplashScreen

import io
import os
import signal
import sys
import time
import traceback
from threading import Thread, get_ident
from pathlib import Path
import termios
import fcntl
import contextlib
import queue

from provisioning.tinkerforge.ip_connection import IPConnection, base58encode, base58decode, BASE58, Error
from provisioning.tinkerforge.bricklet_industrial_quad_relay_v2 import BrickletIndustrialQuadRelayV2
from provisioning.tinkerforge.bricklet_rgb_led_v2 import BrickletRGBLEDV2
from provisioning.tinkerforge.bricklet_temperature_v2 import BrickletTemperatureV2
from provisioning.tinkerforge.bricklet_piezo_speaker_v2 import BrickletPiezoSpeakerV2

from provisioning.provision_common.provision_common import *

main_thread = threading.get_ident()
original_stdout = sys.stdout
original_stderr = sys.stderr

logs = {
    -1: [io.StringIO(""), io.StringIO("")],
    0: [io.StringIO(""), io.StringIO("")],
    1: [io.StringIO(""), io.StringIO("")],
    2: [io.StringIO(""), io.StringIO("")],
    3: [io.StringIO(""), io.StringIO("")]
}

thread_ids = {
}

class StdoutWrapper:
    def __getattribute__(self, name):
        if threading.get_ident() == main_thread or threading.get_ident() not in thread_ids:
            f = logs[-1][0]
        else:
            f = logs[thread_ids[threading.get_ident()]][0]

        return f.__getattribute__(name)

    def __setattr__(self, name, value):
        if threading.get_ident() == main_thread or threading.get_ident() not in thread_ids:
            f = logs[-1][0]
        else:
            f = logs[thread_ids[threading.get_ident()]][0]

        return f.__setattr__(name, value)

class StderrWrapper:
    def __getattribute__(self, name):
        if threading.get_ident() == main_thread or threading.get_ident() not in thread_ids:
            f = logs[-1][1]
        else:
            f = logs[thread_ids[threading.get_ident()]][1]

        return f.__getattribute__(name)

    def __setattr__(self, name, value):
        if threading.get_ident() == main_thread or threading.get_ident() not in thread_ids:
            f = logs[-1][1]
        else:
            f = logs[thread_ids[threading.get_ident()]][1]

        return f.__setattr__(name, value)

class ThreadWithReturnValue(Thread):
    def __init__(self, group=None, target=None, name=None,
                args=(), kwargs={}, Verbose=None):
        Thread.__init__(self, group, target, name, args, kwargs)
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
        Thread.join(self, *args)
        return self._success, self._return, self._exception

def test_bricklet_ports_warp3(ipcon):
    enums = enumerate_devices(ipcon)

    expected = {
        'A': BrickletTemperatureV2.DEVICE_IDENTIFIER,
        'B': BrickletTemperatureV2.DEVICE_IDENTIFIER,
        'C': BrickletRGBLEDV2.DEVICE_IDENTIFIER,
        'D': BrickletTemperatureV2.DEVICE_IDENTIFIER,
    }

    if len(enums) != 4:
        fatal_error("Expected 4 bricklets but found {}".format("\n\t".join("Port {}: {}".format(x.position, x.device_identifier) for x in enums)))

    for x in enums:
        if expected[x.position] != x.device_identifier:
            fatal_error("Expected {} at port {} but found {}".format(expected[x.position], x.position, "\n\t".join("Port {}: {}".format(x.position, x.device_identifier) for x in enums)))
        expected.pop(x.position)

    # This should never happen, as we have 4 expected bricklets and 4 enumerations
    if len(expected) > 0:
        fatal_error("Expected other bricklets but found {}".format("\n\t".join("Port {}: {}".format(x.position, x.device_identifier) for x in enums)))

    enums = sorted(enums, key=lambda x: x.position)

    bricklets = {enum.position: BrickletTemperatureV2(enum.uid, ipcon) if enum.device_identifier == BrickletTemperatureV2.DEVICE_IDENTIFIER else BrickletRGBLEDV2(enum.uid, ipcon) for enum in enums}
    error_count = 0

    temps = []

    for port, bricklet in bricklets.items():
        bricklet.set_response_expected_all(True)

        try:
            if port == 'C':
                bricklet.set_rgb_value(63, 63, 0)
                time.sleep(0.5)
                if bricklet.get_rgb_value() == (63, 63, 0):
                    bricklet.set_rgb_value(0, 0, 0)
                else:
                    print(red("Setting color failed on port {}.".format(port)))
                    error_count += 1
            else:
                temps.append(bricklet.get_temperature() / 100)
        except Exception as e:
            print("Failed to communicate with bricklet on port {}: {}".format(port, e))
            error_count += 1

    if error_count != 0:
        fatal_error("")

    return sum(temps) / len(temps), bricklets['C'].uid_string

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

def get_esp_ssid(serial_port, result):

    print("Checking ESP state")
    mac_address = check_if_esp_is_sane_and_get_mac(allowed_revision=[3.0, 3.1], override_port=serial_port)
    print("MAC Address is {}".format(mac_address))
    result["mac"] = mac_address

    set_voltage_fuses, set_block_3, passphrase, uid = get_espefuse_tasks(override_port=serial_port)
    if set_voltage_fuses or set_block_3:
        fatal_error("Fuses are not set. Re-run stage 0!")

    esptool(["--after", "hard_reset", "run"], override_port=serial_port)

    result["uid"] = uid

    ssid = "warp3-" + uid

    return ssid, passphrase

def test_wifi(ssid, passphrase, ethernet_ip, ethernet_gateway, ethernet_subnet, ethernet_dns, result):
    print("Waiting for ESP wifi. Takes about one minute.")
    if not wait_for_wifi(ssid, 90):
        fatal_error("ESP wifi not found after 90 seconds")

    print("Testing ESP Wifi.")
    with wifi(ssid, passphrase):
        req = urllib.request.Request("http://10.0.0.1/ethernet/config_update",
                                     data=json.dumps({"enable_ethernet":True,
                                                      "ip": ethernet_ip,
                                                      "gateway": ethernet_gateway,
                                                      "subnet": ethernet_subnet,
                                                      "dns": ethernet_dns,
                                                      "dns2":"0.0.0.0"}).encode("utf-8"),
                                     method='PUT',
                                     headers={"Content-Type": "application/json"})
        try:
            with urllib.request.urlopen(req, timeout=10) as f:
                f.read()
        except Exception as e:
            print(e)
            fatal_error("Failed to set ethernet config!")
        req = urllib.request.Request("http://10.0.0.1/reboot", data=b'null', method='PUT', headers={"Content-Type": "application/json"})
        try:
            with urllib.request.urlopen(req, timeout=10) as f:
                f.read()
        except Exception as e:
            print("Failed to initiate reboot! Attempting to connect via ethernet anyway.")

        result["wifi_test_successful"] = True

def run_stage_1_tests(serial_port, ethernet_ip, power_off_fn, power_on_fn, result):
    connect_ethernet(ethernet_ip)

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

    avg_bricklet_temp, rgb_led_uid = test_bricklet_ports_warp3(ipcon)
    result["bricklet_port_test_successful"] = True

    ipcon.disconnect()

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

    test_rtc_time(ethernet_ip, wait_for_ntp=True)

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

    connect_ethernet(ethernet_ip)

    test_rtc_time(ethernet_ip, wait_for_ntp=False)

    result["rtc_test_successful"] = True

    try:
        with urllib.request.urlopen(f"http://{ethernet_ip}/ntp/config_reset", timeout=1) as f:
            f.read()
    except:
        traceback.print_exc()
        fatal_error("Failed to re-enable NTP")

    try:
        with urllib.request.urlopen(f"http://{ethernet_ip}/ethernet/config_reset", timeout=1) as f:
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
    result["end"] = now()

    rgb_led = BrickletRGBLEDV2(rgb_led_uid, ipcon)
    rgb_led.set_rgb_value(0, 127, 0)

    return ipcon, rgb_led_uid


def print_label(ssid, passphrase, stage_1_test_report, relay_to_rgb_led):
    global restart_clicked
    global reprint_clicked
    with open("{}_{}_report_stage_1.json".format(ssid, now().replace(":", "-")), "w") as f:
        json.dump(stage_1_test_report, f, indent=4)

    removed_brick = None
    while removed_brick is None and not restart_clicked:
        if restart_clicked:
            return
        reprint_clicked = False
        run(["python3", "print-esp32-label.py", ssid, passphrase, "-c", "4"])
        label_prompt = "Stick one label on the ESP, put ESP{} in the ESD bag. Press 'Print label again' button to retry printing the label{}. Remove next ESP to continue. Press 'Restart' when all finished ESPs are removed and new ESPs are inserted.".format(
                " and the other three labels",
                "s")

        try:
            while not restart_clicked and not reprint_clicked:
                for k, rgb in list(relay_to_rgb_led.items()):
                    rgb.set_rgb_value(0,63,0)

                time.sleep(0.5)

                for k, rgb in list(relay_to_rgb_led.items()):
                    rgb.set_rgb_value(0,0,0)

                time.sleep(0.5)
        except Error as e:
            if e.value in (Error.TIMEOUT, Error.NOT_CONNECTED):
                return k
            else:
                raise

    return removed_brick


IQR_UID_BLACKLIST = [
    "2dVX",
    "2bnK",
    "2bmW",
    "2bmU"
]

def ansi_escape_to_html(s):
    return s.replace(colors["blue"],  '<font color="#0000FF">')\
            .replace(colors["cyan"],  '<font color="#00FFFF">')\
            .replace(colors["green"], '<font color="#00AA00">')\
            .replace(colors["red"],   '<font color="#FF0000">')\
            .replace(colors["gray"],  '<font color="#555555">')\
            .replace(colors["blink"], '<font>')\
            .replace(colors["off"], '</font>')\
            .replace("\n", "<br/>")

reprint_enabled = False
reprint_clicked = False
def reprint_label():
    global reprint_clicked
    reprint_clicked = True

restart_enabled = False
restart_clicked = False
def restart(app):
    global restart_clicked
    restart_clicked = True
    original_stdout.write("vor disconnect")
    app.aboutToQuit.disconnect(quit)
    original_stdout.write("nach disconnect")
    app.quit()

def update_logs(edits, restart_button, reprint_button):
    global restart_enabled
    global reprint_enabled
    restart_button.setEnabled(restart_enabled)
    reprint_button.setEnabled(reprint_enabled)

    for k, v in edits.items():
        v.setHtml(ansi_escape_to_html(logs[k][0].getvalue() + "\n---\n" + logs[k][1].getvalue()))
        v.verticalScrollBar().triggerAction(QAbstractSlider.SliderToMaximum)

def quit():
    global restart_clicked
    os.killpg(0, signal.SIGKILL)
    #os._exit(42 if restart_clicked else 0)

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

        app.aboutToQuit.connect(quit)

        window = QWidget()
        window.setWindowTitle('WARP ESP32 Provisioning')
        layout = QVBoxLayout()

        tester_widget = QWidget()
        tester_layout = QHBoxLayout()

        buttons_widget = QWidget()
        buttons_layout = QHBoxLayout()

        reprint_button = QPushButton("Print label again")
        reprint_button.clicked.connect(reprint_label)
        reprint_button.setEnabled(reprint_enabled)

        restart_button = QPushButton("Restart")
        restart_button.clicked.connect(lambda: restart(app))
        restart_button.setEnabled(restart_enabled)

        buttons_layout.addWidget(reprint_button)
        buttons_layout.addWidget(restart_button)

        buttons_widget.setLayout(buttons_layout)

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
        layout.addWidget(buttons_widget)

        layout.addWidget(edits[-1])

        window.setLayout(layout)
        window.show()
        splash.finish(window)

        log_timer = QTimer(window)
        log_timer.timeout.connect(lambda: update_logs(edits, restart_button, reprint_button))
        log_timer.setInterval(100)
        log_timer.start()
    except Exception as e:
        original_stdout.write(traceback.format_exc())
        original_stdout.flush()

    app.exec()

def main():
    os.setpgrp()

    q = queue.Queue()

    try:
        qt_thread = ThreadWithReturnValue(target=lambda: run_gui(q))
        qt_thread.start()
        thread_ids[qt_thread.ident] = -1
        qt_thread.start_semaphore.release()

        q.put("Connecting to Brick Daemon")

        global reprint_enabled

        ipcon = IPConnection()
        ipcon.connect("localhost", 4223)

        q.put(f"Searching {BrickletIndustrialQuadRelayV2.DEVICE_DISPLAY_NAME} and {BrickletPiezoSpeakerV2.DEVICE_DISPLAY_NAME}")

        iqr_uid = None
        ps_uid = None

        def search_devices(uid, connected_uid, position, hardware_version, firmware_version,
                    device_identifier, enumeration_type):
            nonlocal iqr_uid
            nonlocal ps_uid
            if device_identifier == BrickletIndustrialQuadRelayV2.DEVICE_IDENTIFIER and uid not in IQR_UID_BLACKLIST:
                iqr_uid = uid

            if device_identifier == BrickletPiezoSpeakerV2.DEVICE_IDENTIFIER:
                ps_uid = uid

        start = time.time()
        ipcon.register_callback(IPConnection.CALLBACK_ENUMERATE, search_devices)
        for i in range(100):
            if iqr_uid is not None and ps_uid is not None:
                break

            if i % 10 == 0:
                ipcon.enumerate()
            time.sleep(0.1)
        else:
            if iqr_uid is None:
                fatal_error(f"{BrickletIndustrialQuadRelayV2.DEVICE_DISPLAY_NAME} not found.")
            elif ps_uid is None:
                fatal_error(f"{BrickletPiezoSpeakerV2.DEVICE_DISPLAY_NAME} not found.")
            else:
                fatal_error("Should be unreachable")

        iqr = BrickletIndustrialQuadRelayV2(iqr_uid, ipcon)
        iqr.set_response_expected_all(True)

        ps = BrickletPiezoSpeakerV2(ps_uid, ipcon)
        ps.set_response_expected_all(True)

        q.put("Powering off testers")

        # This clears the RGB LEDs by powering down the ESP bricks.
        iqr.set_value([False] * 4)
        time.sleep(1)

        q.put("Powering on testers")

        iqr.set_value([True] * 4)
        time.sleep(3)

        relay_to_serial = {k: f"/dev/ttyUSBESPTESTER{k}" for k in range(4) if os.path.exists(f"/dev/ttyUSBESPTESTER{k}")}

        q.put(f"Found {len(relay_to_serial)} testers. Starting GUI.")

        q.put(list(relay_to_serial.keys()))
    except Exception:
        q.put([])
        raise

    config = json.loads(Path("provision_warp_esp32_ethernet.config").read_text())
    static_ips = config["static_ips"]
    subnet = config["subnet"]
    gateway = config["gateway"]
    dns = config["dns"]

    print(green(f"Flashing {len(relay_to_serial)} ESPs..."))

    threads = []
    for k, v in relay_to_serial.items():
        # lambda with default parameter value to fix the late binding issue. If v was used directly, it would behave as if "captured by reference" -> fun with multithreading.
        t = ThreadWithReturnValue(target=lambda port=v: \
            subprocess.run(
                ['python', 'provision_stage_0_warp2.py', '../../firmwares/bricks/warp3_charger/brick_warp3_charger_firmware_latest.bin', port, "warp3"],
                capture_output=True,
                encoding='utf-8'))
        t.start()
        thread_ids[t.ident] = k
        t.start_semaphore.release()
        threads.append((k, v, t))

    for k, v, t in threads:
        success, result, exception = t.join()
        if not success:
            print(red(f"Failed to run stage 0 for {k} {v}: {exception}"), file=logs[k][1])
            relay_to_serial.pop(k)

        if result.returncode != 0:
            print(red(f"Failed to run stage 0 for {k} {v}: {result.stdout}\n{result.stderr}"), file=logs[k][1])
            relay_to_serial.pop(k)

    threads.clear()

    print(green(f"{len(relay_to_serial)} ESPs flashed successfully"))

    def get_esp_ssid_fn(serial_port, result):
        return lambda: get_esp_ssid(serial_port, result)

    test_reports = {k: {"start": now()} for k in relay_to_serial.keys()}

    relay_to_ssid = {}
    relay_to_passphrase = {}

    for k, v in relay_to_serial.items():
        t = ThreadWithReturnValue(target=get_esp_ssid_fn(v, test_reports[k]))
        t.start()
        thread_ids[t.ident] = k
        t.start_semaphore.release()
        threads.append((k, v, t))

    for k, v, t in threads:
        success, result, exception = t.join()
        if not success:
            print(red(f"Failed to run get_esp_ssid for {k} {v}: {exception}"), file=logs[k][1])
            relay_to_serial.pop(k)
        else:
            relay_to_ssid[k] = result[0]
            relay_to_passphrase[k] = result[1]

    print(green(str(relay_to_ssid)))

    threads.clear()

    run(["sudo", "systemctl", "restart", "NetworkManager.service"])
    run(["sudo", "iw", "reg", "set", "DE"])

    for k, v in list(relay_to_serial.items()):
        with contextlib.redirect_stdout(logs[k][0]):
            with contextlib.redirect_stdout(logs[k][1]):
                try:
                    test_wifi(relay_to_ssid[k], relay_to_passphrase[k], static_ips[k], gateway, subnet, dns, test_reports[k])
                except BaseException as e:
                    print(red(f"Failed to test WiFi for {k} {v}: {e}"))
                    relay_to_serial.pop(k)

    def run_stage_1_tests_fn(serial_port, ethernet_ip, relay_pin, test_report):
        return lambda: run_stage_1_tests(serial_port, ethernet_ip, lambda: iqr.set_selected_value(relay_pin, False), lambda: iqr.set_selected_value(relay_pin, True), test_report)

    for k, v in relay_to_serial.items():
        print(green(f"{k}: {static_ips[k]}"))
        t = ThreadWithReturnValue(target=run_stage_1_tests_fn(v, static_ips[k], k, test_reports[k]))
        t.start()
        thread_ids[t.ident] = k
        t.start_semaphore.release()
        threads.append((k, v, t))

    relay_to_rgb_led = {}

    for k, v, t in threads:
        success, result, exception = t.join()
        if not success:
            print(red(f"Failed to run run_stage_1_tests for {k} {v}: {exception}"), file=logs[k][1])
            relay_to_serial.pop(k)
        else:
            ipcon, rgb_led_uid = result
            rgb_led = BrickletRGBLEDV2(rgb_led_uid, ipcon)
            rgb_led.set_response_expected_all(True)
            ipcon.set_timeout(0.5)

            relay_to_rgb_led[k] = rgb_led

    threads.clear()

    global restart_clicked
    global restart_enabled
    restart_clicked = False
    restart_enabled = True

    ps.set_beep(262, 0, 250)
    time.sleep(0.25)
    ps.set_beep(330, 0, 250)
    time.sleep(0.25)
    ps.set_beep(392, 0, 250)
    time.sleep(0.25)
    ps.set_beep(523, 0, 1000)

    while len(relay_to_rgb_led) > 0 and not restart_clicked:
        if restart_clicked:
            return

        print(green(f"ESPs in testers {', '.join(str(x) for x in relay_to_rgb_led.keys())} tested successfully. Remove one of the ESPs to print its label!"))
        try:
            while not restart_clicked:
                for k, rgb in list(relay_to_rgb_led.items()):
                    rgb.set_rgb_value(0,63,0)

                time.sleep(0.5)

                for k, rgb in list(relay_to_rgb_led.items()):
                    rgb.set_rgb_value(0,0,0)

                time.sleep(0.5)
        except Error as e:
            if e.value in (Error.TIMEOUT, Error.NOT_CONNECTED):
                reprint_enabled = True
                next_brick = k
                while next_brick is not None and len(relay_to_rgb_led) > 0 and not restart_clicked:
                    if restart_clicked:
                        print("restart")
                        return
                    print(green(f"Removed {next_brick}"))
                    iqr.set_selected_value(next_brick, False)
                    rgb_led = relay_to_rgb_led.pop(next_brick)
                    rgb_led.ipcon.disconnect()
                    next_brick = print_label(relay_to_ssid[next_brick], relay_to_passphrase[next_brick], test_reports[next_brick], relay_to_rgb_led)
                    print("Next brick", next_brick)
            else:
                raise
    original_stdout.write("before join")
    qt_thread.join()

    result = 42 if restart_clicked else 0
    original_stdout.write("returning" + str(result) + "\n")
    original_stdout.flush()
    return result

if __name__ == "__main__":
    with contextlib.redirect_stdout(StdoutWrapper()):
        with contextlib.redirect_stderr(StderrWrapper()):
            try:
                os._exit(main())
            except Exception:
                logs[-1][1].write(traceback.format_exc())
                raise
