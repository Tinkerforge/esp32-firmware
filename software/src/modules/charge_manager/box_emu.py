#!/usr/bin/python3 -u

import socket
import struct
import sys
import time
import ipaddress
import math
from dataclasses import dataclass, field

from PyQt5.QtWidgets import *
from PyQt5.QtCore import QTimer

structs = """
    struct cm_packet_header {
        uint16_t magic;
        uint16_t length;
        uint16_t seq_num;
        uint8_t version;
        uint8_t padding;
    } __attribute__((packed));

    struct cm_command_v1 {
        uint16_t allocated_current;
        /* command_flags
        bit 6 - control pilot permanently disconnected
        Other bits must be sent unset and ignored on reception.
        */
        uint8_t command_flags;
        uint8_t padding;
    } __attribute__((packed));

    struct cm_command_packet {
        cm_packet_header header;
        cm_command_v1 v1;
    } __attribute__((packed));

    struct cm_state_v1 {
        uint32_t feature_flags; /* unused */
        uint32_t esp32_uid;
        uint32_t evse_uptime;
        uint32_t charging_time;
        uint16_t allowed_charging_current;
        uint16_t supported_current;

        uint8_t iec61851_state;
        uint8_t charger_state;
        uint8_t error_state;
        /* state_flags
        bit 7 - managed
        bit 6 - control_pilot_permanently_disconnected
        bit 5 - L1_connected
        bit 4 - L2_connected
        bit 3 - L3_connected
        bit 2 - L1_active
        bit 1 - L2_active
        bit 0 - L3_active
        */
        uint8_t state_flags;
        float line_voltages[3];
        float line_currents[3];
        float line_power_factors[3];
        float power_total;
        float energy_rel;
        float energy_abs;
    } __attribute__((packed));

    struct cm_state_v2 {
        uint32_t time_since_state_change;
    } __attribute__((packed));

    struct cm_state_packet {
        cm_packet_header header;
        cm_state_v1 v1;
        cm_state_v2 v2;
    } __attribute__((packed));
"""

header_format = "<HHHBx"
command_format = header_format + "HBx"
COMMAND_VERSION = 1
state_format = header_format + "IIIIHHBBBBffffffffffff" + "I"
STATE_VERSION = 2

command_len = struct.calcsize(command_format)
state_len = struct.calcsize(state_format)

@dataclass
class ChargerState:
    uid: int
    charging_time_start: float = 0
    charging_time: int = 0
    next_seq_num: int = 0
    start: float = time.time()
    time_since_state_change = time.time()
    command_version = 1
    state_version = 2
    uptime: int = 0
    uptime_blocked: bool = False
    iec61851_state: int = 0
    last_iec61851_state: int = 0
    managed: bool = True
    cp_disconnect: bool = False
    allowed_charging_current: int = 0
    supported_current: int = 0
    charger_state: int = 0
    error_state: int = 0
    line_voltages: list[float] = field(default_factory=lambda: [230, 0, 0])
    line_currents: list[float] = field(default_factory=lambda: [0, 0, 0])
    line_power_factors: list[float] = field(default_factory=lambda: [1, 0, 0])
    energy_rel: float = 0
    energy_abs: float = 0
    auto_mode: bool = False

    def tick(self):
        if not self.uptime_blocked:
            self.uptime = int((time.time() - self.start) * 1000) % (1 << 32)

        if self.charging_time_start == 0 and self.iec61851_state == 2:
            self.charging_time_start = time.time()

        if self.charging_time_start != 0 and self.iec61851_state == 0:
            self.charging_time_start = 0

        if self.charging_time_start == 0:
            self.charging_time = 0
        else:
            self.charging_time = int((time.time() - self.charging_time_start) * 1000)

        if self.auto_mode:
            self.iec61851_state = {0: 0, 1: 1, 2: 1, 3: 2, 4: 4}[self.charger_state]

        if self.last_iec61851_state != self.iec61851_state:
            self.time_since_state_change = time.time()
            self.last_iec61851_state = self.iec61851_state

class Charger:
    def __init__(self, layout, row, col, listen_addr):
        self.auto_mode = True
        self.hide_on_auto = True

        self.state = ChargerState(uid=struct.unpack('>I', ipaddress.ip_address(listen_addr).packed)[0])
        self.state.auto_mode = self.auto_mode

        self.layout = layout
        self.row = row
        self.col = col
        self.row_counter = 0
        self.add_labels = col == 1
        self.listen_addr = listen_addr

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind((listen_addr, 34128))
        self.sock.setblocking(False)
        self.addr = None

        self.build_ui()


        self.recv_timer = QTimer()
        self.recv_timer.timeout.connect(lambda: self.receive())
        self.recv_timer.start(100)

        self.send_timer = QTimer()
        self.send_timer.timeout.connect(lambda: self.send())
        self.send_timer.start(1000)

    def addRow(self, title_or_widget, widget=None):
        if widget is None:
            widget = title_or_widget
            title_or_widget = None
        self.layout.addWidget(widget, self.row + self.row_counter, self.col)
        if self.add_labels:
            if title_or_widget is not None:
                l = QLabel(title_or_widget)
                l.setStyleSheet("font-weight: bold;")
                self.layout.addWidget(l, self.row + self.row_counter, 0)
            else:
                self.layout.addWidget(widget, self.row + self.row_counter, 0)


        self.row_counter += 1


    def build_ui(self):
        def handle_auto(title, widget):
            if self.auto_mode:
                widget.setEnabled(False)
            if self.auto_mode and self.hide_on_auto:
                widget.setHidden(True)
            else:
                self.addRow(title, widget)

        self.title = QLabel(str(self.listen_addr))
        self.title.setStyleSheet("font-weight: bold;")
        self.addRow(self.title)

        self.req_seq_num = QLabel("no packet received yet")
        self.addRow("Req Sequence number", self.req_seq_num)

        self.req_version = QLabel("no packet received yet")
        handle_auto("Req Protocol version", self.req_version)

        self.req_allocated_current = QLabel("no packet received yet")
        self.addRow("Req Allocated current", self.req_allocated_current)

        self.req_cp_disconnect = QLabel("no packet received yet")
        self.addRow("Req CP disconnect", self.req_cp_disconnect)

        self.resp_seq_num = QLabel("no packet sent yet")
        self.addRow("Sequence number", self.resp_seq_num)

        self.resp_block_seq_num = QCheckBox("Block sequence number")
        self.addRow("", self.resp_block_seq_num)

        self.resp_version = QLabel("no packet sent yet")
        self.addRow("Protocol version", self.resp_version)

        self.resp_wrong_proto_version = QCheckBox("Wrong protocol version")
        self.addRow("", self.resp_wrong_proto_version)

        self.resp_iec61851_state = QComboBox()
        self.resp_iec61851_state.addItem("0: A - Not connected")
        self.resp_iec61851_state.addItem("1: B - Connected")
        self.resp_iec61851_state.addItem("2: C - Charging")
        self.resp_iec61851_state.addItem("3: D - Not supported")
        self.resp_iec61851_state.addItem("4: E/F - Error")
        handle_auto("IEC state", self.resp_iec61851_state)

        self.resp_charger_state = QComboBox()
        self.resp_charger_state.addItem("0: Not connected")
        self.resp_charger_state.addItem("1: Waiting for release")
        self.resp_charger_state.addItem("2: Ready")
        self.resp_charger_state.addItem("3: Charging")
        self.resp_charger_state.addItem("4: Error")
        self.addRow("Vehicle state", self.resp_charger_state)

        self.resp_error_state = QSpinBox()
        self.resp_error_state.setMinimum(0)
        self.resp_error_state.setMaximum(5)
        self.addRow("Error state", self.resp_error_state)

        self.resp_uptime = QLabel("no packet sent yet")
        self.addRow("Uptime", self.resp_uptime)

        self.resp_block_uptime = QCheckBox("Block uptime")
        self.addRow("", self.resp_block_uptime)

        self.resp_charging_time = QLabel("no packet sent yet")
        self.addRow("Charging time", self.resp_charging_time)

        self.resp_allowed_charging_current = QSpinBox()
        self.resp_allowed_charging_current.setMinimum(0)
        self.resp_allowed_charging_current.setMaximum(32)
        self.resp_allowed_charging_current.setSuffix(" A")
        handle_auto("Allowed charging current", self.resp_allowed_charging_current)

        self.resp_supported_current = QSpinBox()
        self.resp_supported_current.setMinimum(0)
        self.resp_supported_current.setMaximum(32)
        self.resp_supported_current.setSuffix(" A")
        self.addRow("Supported current", self.resp_supported_current)

        self.resp_max_line_current = QSpinBox()
        self.resp_max_line_current.setMinimum(0)
        self.resp_max_line_current.setMaximum(32)
        self.resp_max_line_current.setSuffix(" A")
        self.addRow("Max line current", self.resp_max_line_current)

        self.resp_managed = QCheckBox("")
        self.resp_managed.setChecked(True)
        self.addRow("Managed", self.resp_managed)

        self.resp_cp_disconnect = QCheckBox("CP disconnected")
        handle_auto("CP disconnect state", self.resp_cp_disconnect)

    def receive(self):
        try:
            data, self.addr = self.sock.recvfrom(command_len)
        except BlockingIOError:
            return
        if len(data) != command_len:
            return

        magic, length, seq_num, version, allocated_current, command_flags = struct.unpack(command_format, data)

        self.req_seq_num.setText(str(seq_num))
        self.req_version.setText(str(version))
        self.req_allocated_current.setText("{:2.3f} A".format(allocated_current / 1000.0))
        if self.auto_mode:
            self.state.allowed_charging_current = allocated_current

        should_disconnect_cp = (command_flags & 0x40) >> 6
        self.req_cp_disconnect.setText(str(should_disconnect_cp))
        if self.auto_mode:
            self.state.cp_disconnect = should_disconnect_cp

        if self.auto_mode and allocated_current == 0 and self.state.iec61851_state == 2:
            self.state.charging_time_start = 0
            self.state.iec61851_state = 1
            self.state.charger_state = 1
            self.resp_iec61851_state.setCurrentIndex(self.state.iec61851_state)
            self.resp_charger_state.setCurrentIndex(self.state.charger_state)

        if self.auto_mode and allocated_current > 0 and self.state.charger_state == 1:
            self.state.charger_state = 2
            self.resp_charger_state.setCurrentIndex(self.state.charger_state)

    def update_ui_from_state(self):
        self.resp_seq_num.setText(str(self.state.next_seq_num))
        self.resp_version.setText(str(self.state.state_version))
        self.resp_uptime.setText("{} ms".format(self.state.uptime))
        self.resp_charging_time.setText("{} ms".format(self.state.charging_time))

        if not self.auto_mode:
            return

        self.resp_allowed_charging_current.setValue(int(self.state.allowed_charging_current / 1000))
        self.resp_cp_disconnect.setChecked(self.state.cp_disconnect)
        self.resp_iec61851_state.setCurrentIndex(self.state.iec61851_state)

    def update_state_from_ui(self):
        self.state.uptime_blocked = self.resp_block_uptime.isChecked()
        self.state.charger_state = self.resp_charger_state.currentIndex()
        self.state.managed = self.resp_managed.isChecked()
        self.state.state_version = STATE_VERSION if not self.resp_wrong_proto_version.isChecked() else 0
        self.state.supported_current = self.resp_supported_current.value() * 1000
        self.state.error_state = self.resp_error_state.value()
        self.state.current_l1 = self.resp_max_line_current.value()

        if self.auto_mode:
            return

        self.state.cp_disconnect = self.resp_cp_disconnect.isChecked()
        self.state.allowed_charging_current = self.resp_allowed_charging_current.value() * 1000
        self.state.iec61851_state = self.resp_iec61851_state.currentIndex()

    def send(self):
        self.state.tick()
        self.update_ui_from_state()
        self.update_state_from_ui()

        if self.addr is None:
            return

        flags = 0
        flags |= 0x80 if self.state.managed else 0
        flags |= 0x40 if self.state.cp_disconnect else 0

        b = struct.pack(state_format,
                        34127, # magic
                        state_len,
                        self.state.next_seq_num,
                        self.state.state_version,
                        0x7F,  # features
                        self.state.uid,
                        self.state.uptime,
                        self.state.charging_time,
                        self.state.allowed_charging_current,
                        self.state.supported_current,
                        self.state.iec61851_state,
                        self.state.charger_state,
                        self.state.error_state,
                        flags,
                        *self.state.line_voltages,
                        *self.state.line_currents,
                        *self.state.line_power_factors,
                        sum(u * i for u, i in zip(self.state.line_voltages, self.state.line_currents)), # power total
                        self.state.energy_rel,  # energy_rel
                        self.state.energy_abs,  # energy_abs
                        int(1000 * (time.time() - self.state.time_since_state_change))
        )
        if not self.resp_block_seq_num.isChecked():
            self.state.next_seq_num += 1
            self.state.next_seq_num %= 65536

        self.sock.sendto(b, self.addr)

app = QApplication([])
window = QWidget()

window.setWindowTitle(",".join(sys.argv[1:]))

chargers = len(sys.argv) - 1
cols = math.ceil(chargers / 13)
rows = math.ceil(chargers / cols)

top_level_layout = QGridLayout()

last_row_counter = 0

chargers = []

for i, listen_addr in enumerate(sys.argv[1:]):
    col = int(i % rows)
    row = int(i / rows) + last_row_counter
    print(col, row, cols, rows)

    chargers.append(Charger(top_level_layout, row, col + 1, listen_addr))
    if col == rows - 1:
        print("Updating last row counter")
        last_row_counter += chargers[-1].row_counter

window.setLayout(top_level_layout)
window.show()
app.exec_()
