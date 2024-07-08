#!/usr/bin/python3 -u

import socket
import struct
import sys
import time
import ipaddress
import math
from dataclasses import dataclass, field

"""
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
        bit 6    - control pilot permanently disconnected
        */
        uint8_t command_flags;
        uint8_t _padding;
    };

    struct cm_command_v2 {
        uint16_t _padding_0;
        uint8_t _padding_1;
        int8_t allocated_phases; // Was padding in CM_COMMAND_VERSION 1
    };

    struct cm_command_packet {
        cm_packet_header header;
        union {
            cm_command_v1 v1;
            cm_command_v2 v2;
        };
    };

    struct cm_state_v1 {
        /* feature_flags
        bit 7 - has phase_switch
        bit 6 - has cp_disconnect
        bit 5 - has evse
        bit 4 - has nfc
        bit 3 - has meter_all_values
        bit 2 - has meter_phases
        bit 1 - has meter
        bit 0 - has button_configuration
        Other bits must be sent unset and ignored on reception.
        */
        uint32_t feature_flags;
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

    struct cm_state_v3 {
        // bit 2: can switch phases now
        // bit 0-1: phases "connected"
        uint8_t phases;
        uint8_t padding[3];
    };

    struct cm_state_packet {
        cm_packet_header header;
        cm_state_v1 v1;
        cm_state_v2 v2;
        cm_state_v3 v3;
    } __attribute__((packed));
"""

header_format = "<HHHBx"
command_format = header_format + "HBb"
COMMAND_VERSION = 2
state_format = header_format + "IIIIHHBBBBffffffffffff" + "I" + "Bxxx"
STATE_VERSION = 3

command_len = struct.calcsize(command_format)
state_len = struct.calcsize(state_format)

assert(command_len == 12)
assert(state_len == 88)

@dataclass
class Charger:
    uid: int
    listen_addr: str
    auto_mode: bool

    # API
    uptime_blocked: bool = False
    seq_num_blocked: bool = False
    expected_command_version = COMMAND_VERSION
    state_version = STATE_VERSION

    managed: bool = True
    supported_current: int = 0

    error_state: int = 0
    line_voltages: list[float] = field(default_factory=lambda: [230, 0, 0])
    line_currents: list[float] = field(default_factory=lambda: [0, 0, 0])
    line_power_factors: list[float] = field(default_factory=lambda: [1, 0, 0])
    energy_rel: float = 0
    energy_abs: float = 0

    # API if uptime_blocked, automatic otherwise
    uptime: int = 0

    # API if seq_num_blocked, automatic otherwise
    next_seq_num: int = 0

    # API if not auto_mode, automatic otherwise
    iec61851_state: int = 0
    allowed_charging_current: int = 0
    allocated_phases: int = 0
    cp_disconnect: bool = False
    charger_state: int = 0

    # Always automatic
    charging_time_start: float = 0
    charging_time: int = 0
    time_since_state_change: float = field(default_factory=lambda: time.time())

    # Last received request data. Updated in recv()
    manager_addr = None
    req_seq_num: int = 0
    req_version: int = 0
    req_allocated_current: int = 0
    req_allocated_phases: int = 0
    req_should_disconnect_cp: bool = False

    # Internal
    _start: float = field(default_factory=lambda: time.time())
    _last_iec61851_state: int = 0
    _sock: socket.socket = field(default_factory=lambda: socket.socket(socket.AF_INET, socket.SOCK_DGRAM))

    def __post_init__(self):
        self._sock.bind((self.listen_addr, 34128))
        self._sock.setblocking(False)

    def reset(self):
        self._sock.close()
        self.__init__(self.uid, self.listen_addr, self.auto_mode)

    def tick(self):
        if self.auto_mode:
            # Apply last seen request info
            self.allowed_charging_current = self.req_allocated_current
            self.allocated_phases = self.req_allocated_phases
            self.cp_disconnect = self.req_should_disconnect_cp

            if self.req_allocated_current == 0 and self.iec61851_state == 2:
                self.charger_state = 1
                # Stop charging timer to differentiate between "full car" and "blocked by charge manager".
                # This is used to lower the priority of probably full cars
                self.charging_time_start = 0
            elif self.req_allocated_current > 0 and self.charger_state == 1:
                self.charger_state = 2

            # Apply charger state
            self.iec61851_state = {0: 0, 1: 1, 2: 1, 3: 2, 4: 4}[self.charger_state]

        if not self.uptime_blocked:
            self.uptime = int((time.time() - self._start) * 1000) % (1 << 32)

        # Start/stop charging timer
        if self.charging_time_start == 0 and self.iec61851_state == 2:
            self.charging_time_start = time.time()

        if self.charging_time_start != 0 and self.iec61851_state == 0:
            self.charging_time_start = 0

        # Update charging time from timer
        if self.charging_time_start == 0:
            self.charging_time = 0
        else:
            self.charging_time = int((time.time() - self.charging_time_start) * 1000)

        # Update time_since_state_change
        if self._last_iec61851_state != self.iec61851_state:
            self.time_since_state_change = time.time()
            self._last_iec61851_state = self.iec61851_state

    def send(self):
        if self.manager_addr is None:
            return

        flags = 0
        flags |= 0x80 if self.managed else 0
        flags |= 0x40 if self.cp_disconnect else 0

        b = struct.pack(state_format,
                        34127, # magic
                        state_len,
                        self.next_seq_num,
                        self.state_version,
                        0xFF,  # features
                        self.uid,
                        self.uptime,
                        self.charging_time,
                        self.allowed_charging_current,
                        self.supported_current,
                        self.iec61851_state,
                        self.charger_state,
                        self.error_state,
                        flags,
                        *self.line_voltages,
                        *self.line_currents,
                        *self.line_power_factors,
                        sum(u * i for u, i in zip(self.line_voltages, self.line_currents)), # power total
                        self.energy_rel,  # energy_rel
                        self.energy_abs,  # energy_abs
                        int(1000 * (time.time() - self.time_since_state_change)),
                        self.allocated_phases
        )

        if not self.seq_num_blocked:
            self.next_seq_num += 1
            self.next_seq_num %= 65536

        self._sock.sendto(b, self.manager_addr)

    def recv(self):
        try:
            data, self.manager_addr = self._sock.recvfrom(command_len)
        except BlockingIOError:
            return
        if len(data) != command_len:
            return

        magic, length, seq_num, version, allocated_current, command_flags, allocated_phases = struct.unpack(command_format, data)

        if version != self.expected_command_version:
            return

        self.req_seq_num = seq_num
        self.req_version = version
        self.req_allocated_current = allocated_current
        self.req_should_disconnect_cp = ((command_flags & 0x40) >> 6) == 1
        self.req_allocated_phases = allocated_phases

if __name__ == "__main__":
    from PyQt5.QtWidgets import *
    from PyQt5.QtCore import QTimer, Qt

    class ChargerUI:
        def __init__(self, layout, row, col, listen_addr):
            self.hide_on_auto = True

            self.state = Charger(uid=struct.unpack('>I', ipaddress.ip_address(listen_addr).packed)[0], listen_addr=listen_addr, auto_mode=True)

            self.layout = layout
            self.row = row
            self.col = col
            self.row_counter = 0
            self.add_labels = col == 1

            self.build_ui()
            self.register_signals()

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
                if self.state.auto_mode:
                    widget.setEnabled(False)
                if self.state.auto_mode and self.hide_on_auto:
                    widget.setHidden(True)
                else:
                    self.addRow(title, widget)

            self.title = QLabel(str(self.state.listen_addr))
            self.title.setStyleSheet("font-weight: bold;")
            self.addRow(self.title)

            self.req_seq_num = QLabel("no packet received yet")
            self.addRow("Req Sequence number", self.req_seq_num)

            self.req_version = QLabel("no packet received yet")
            handle_auto("Req Protocol version", self.req_version)

            self.req_allocated_current = QLabel("no packet received yet")
            self.addRow("Req Allocated current", self.req_allocated_current)

            self.req_allocated_phases = QLabel("no packet received yet")
            self.addRow("Req Allocated phases", self.req_allocated_phases)

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

            self.reset = QPushButton("Reset")
            self.addRow("", self.reset)

        def register_signals(self):
            # Python lambdas don't allow assignments
            self.resp_block_uptime.stateChanged.connect(lambda x: setattr(self.state, "uptime_blocked", x == Qt.CheckState.Checked))
            self.resp_block_seq_num.stateChanged.connect(lambda x: setattr(self.state, "seq_num_blocked", x == Qt.CheckState.Checked))
            self.resp_charger_state.currentIndexChanged.connect(lambda x: setattr(self.state, "charger_state", x))
            self.resp_managed.stateChanged.connect(lambda x: setattr(self.state, "managed", x == Qt.CheckState.Checked))
            self.resp_wrong_proto_version.stateChanged.connect(lambda x: setattr(self.state, "state_version", 0 if x == Qt.CheckState.Checked else STATE_VERSION))

            self.resp_supported_current.valueChanged.connect(lambda x: setattr(self.state, "supported_current", x * 1000))
            self.resp_error_state.valueChanged.connect(lambda x: setattr(self.state, "error_state", x))
            self.resp_max_line_current.valueChanged.connect(lambda x: setattr(self.state, "current_l1", x))

            self.resp_cp_disconnect.stateChanged.connect(lambda x: setattr(self.state, "cp_disconnect", x == Qt.CheckState.Checked))
            self.resp_allowed_charging_current.valueChanged.connect(lambda x: setattr(self.state, "allowed_charging_current", x * 1000))
            self.resp_iec61851_state.currentIndexChanged.connect(lambda x: setattr(self.state, "iec61851_state", x))
            self.reset.clicked.connect(lambda: self.state.reset())

        def update_ui_from_state(self):
            self.resp_seq_num.setText(str(self.state.next_seq_num))
            self.resp_version.setText(str(self.state.state_version))
            self.resp_uptime.setText("{} ms".format(self.state.uptime))
            self.resp_charging_time.setText("{} ms".format(self.state.charging_time))

            self.req_seq_num.setText(str(self.state.req_seq_num))
            self.req_version.setText(str(self.state.req_version))
            self.req_allocated_current.setText("{:2.3f} A".format(self.state.req_allocated_current / 1000.0))
            self.req_allocated_phases.setText(str(self.state.req_allocated_phases))

            self.req_cp_disconnect.setText(str(self.state.req_should_disconnect_cp))
            self.resp_iec61851_state.setCurrentIndex(self.state.iec61851_state)
            self.resp_charger_state.setCurrentIndex(self.state.charger_state)

            self.resp_allowed_charging_current.setValue(int(self.state.allowed_charging_current / 1000))
            self.resp_cp_disconnect.setChecked(self.state.cp_disconnect)
            self.resp_iec61851_state.setCurrentIndex(self.state.iec61851_state)

        def receive(self):
            self.state.recv()

        def send(self):
            self.state.tick()
            self.update_ui_from_state()
            self.state.send()

    def main():
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

            chargers.append(ChargerUI(top_level_layout, row, col + 1, listen_addr))
            if col == rows - 1:
                last_row_counter += chargers[-1].row_counter

        window.setLayout(top_level_layout)
        window.show()
        app.exec_()

    main()

