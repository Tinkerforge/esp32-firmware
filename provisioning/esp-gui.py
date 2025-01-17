#!/usr/bin/env python3
from PyQt5.QtCore import QTimer
from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout, QPushButton, QComboBox

from collections import namedtuple
import functools
import os
import shlex
import subprocess
import queue
import threading

selected_firmware_type = ""

Action = namedtuple("Action", "name pwd cmd run_as_root shell detach show_filter_fn")

actions = [
    Action("ESP Parallel-Flash", ".", "./provision_stage_0_{{{firmware_type}}}.sh", True, "hold_shell", False, lambda x: x != "warp3"),
    Action("ESP Test", ".", "python3 -u provision_stage_1_{{{brick_type}}}.py {{{firmware_type}}}", True, "shell", False, lambda x: x != "warp3"),
    Action("ESP Print Label (Skip Test)", ".", "python3 -u provision_stage_1_{{{brick_type}}}.py {{{firmware_type}}} --skip-tests", False, "shell", False, lambda x: x != "warp3"),
    Action("WARP ESP Flash and Test", ".", "python3 -u provision_warp_esp32_ethernet_brick.py", True, "hold_shell", False, lambda x: x == "warp3")
]

# use "with ChangedDirectory('/path/to/abc')" instead of "os.chdir('/path/to/abc')"
class ChangedDirectory(object):
    def __init__(self, path):
        self.path = path
        self.previous_path = None

    def __enter__(self):
        self.previous_path = os.getcwd()
        os.chdir(self.path)

    def __exit__(self, type_, value, traceback):
        os.chdir(self.previous_path)

work_queue = queue.Queue()
work_done_queue = queue.Queue()

def work(q: queue.Queue, done_q: queue.Queue):
    global selected_firmware_type
    while True:
        todo = q.get(block=True)
        if todo is None:
            return

        a, root_pw, btn = todo

        with ChangedDirectory(a.pwd):
            cmd = a.cmd if isinstance(a.cmd, list) else [a.cmd]

            for c in cmd:
                if selected_firmware_type == "esp32":
                    brick_type = "esp32"
                elif selected_firmware_type == "warp3":
                    brick_type = "warp_esp32_ethernet"
                else:
                    brick_type = "esp32_ethernet"

                c = c.replace("{{{firmware_type}}}", selected_firmware_type)
                c = c.replace("{{{brick_type}}}", brick_type)
                sub_cmd = shlex.split(c)

                if a.shell != "no_shell":
                    sub_cmd = ["konsole", "--hold" if a.shell == "hold_shell" else "", "-e", *sub_cmd]

                if a.run_as_root:
                    sub_cmd = ["sudo", "-S", *sub_cmd]

                if not a.detach:
                    subprocess.check_output(sub_cmd, input=root_pw.encode("utf-8") if a.run_as_root else "")
                else:
                    subprocess.Popen(sub_cmd)
                done_q.put(btn)

def run(a: Action, root_pw: str, btn: QPushButton):
    btn.setDisabled(True)
    work_queue.put((a, root_pw, btn))

def unblock_btn_check():
    try:
        btn = work_done_queue.get_nowait()
    except queue.Empty:
        return
    btn.setDisabled(False)

class App(QApplication):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.unblock_btn_timer = QTimer()
        self.unblock_btn_timer.timeout.connect(unblock_btn_check)
        self.unblock_btn_timer.setInterval(100)
        self.unblock_btn_timer.start()


def main():
    with open("root_password.txt") as f:
        root_pw = f.readline().strip()

    t = threading.Thread(target=work, args=[work_queue, work_done_queue], daemon=True)
    t.start()

    app = App([])
    window = QWidget()
    window.setWindowTitle('ESP GUI')
    layout = QVBoxLayout()
    combo_box = QComboBox()
    combo_box.addItem("Select Brick type...", "")
    combo_box.addItem("ESP32 Brick (w/o Ethernet)", "esp32")
    combo_box.addItem("ESP32 Ethernet Brick", "esp32_ethernet")
    combo_box.addItem("ESP32 Ethernet Brick (WARP2 Firmware)", "warp2")
    combo_box.addItem("ESP32 Ethernet Brick (Energy Manager Firmware)", "energy_manager")
    combo_box.addItem("ESP32 Ethernet Brick (Energy Manager 2.0 Firmware)", "energy_manager_v2")
    combo_box.addItem("ESP32 Ethernet Brick (Smart Energy Broker Firmware)", "smart_energy_broker")
    combo_box.addItem("WARP ESP32 Ethernet Brick", "warp3")
    layout.addWidget(combo_box)

    btn_action = {}

    for a in actions:
        btn = QPushButton(a.name, window)
        btn.setDisabled(True)
        btn.clicked.connect(functools.partial(run, a, root_pw, btn))
        layout.addWidget(btn)
        btn_action[btn] = a

    def currentIndexChanged(idx):
        global selected_firmware_type
        selected_firmware_type = combo_box.itemData(idx)
        for button in window.findChildren(QPushButton):
            button.setDisabled(idx == 0)
            if btn_action[button].show_filter_fn(selected_firmware_type):
                button.show()
            else:
                button.hide()

    combo_box.currentIndexChanged.connect(currentIndexChanged)

    window.setLayout(layout)
    window.show()
    app.exec_()
    work_queue.put(None)
    t.join()

if __name__ == "__main__":
    main()
