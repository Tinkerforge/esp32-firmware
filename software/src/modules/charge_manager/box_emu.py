import socket
import struct
import sys
import time
import ipaddress

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

struct cm_state_packet {
    cm_packet_header header;
    cm_state_v1 v1;
} __attribute__((packed));
"""

header_format = "<HHHBx"
command_format = header_format + "HBx"
state_format = header_format + "IIIIHHBBBBffffffffffff"

command_len = struct.calcsize(command_format)
state_len = struct.calcsize(state_format)
print(state_len)


listen_addr = sys.argv[1]
uid = struct.unpack('>I', ipaddress.ip_address(listen_addr).packed)[0]

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((listen_addr, 34128))
sock.setblocking(False)

addr = None

from PyQt5.QtWidgets import *
from PyQt5.QtCore import QTimer

app = QApplication([])
window = QWidget()
window.setWindowTitle(sys.argv[1])
layout = QFormLayout()
layout.addRow(QLabel("Request"))

req_seq_num = QLabel("no packet received yet")
layout.addRow("Sequence number", req_seq_num)

req_version = QLabel("no packet received yet")
layout.addRow("Protocol version", req_version)

req_allocated_current = QLabel("no packet received yet")
layout.addRow("Allocated current", req_allocated_current)

req_cp_disconnect = QLabel("no packet received yet")
layout.addRow("CP disconnect", req_cp_disconnect)

layout.addRow(QLabel("Response"))

resp_seq_num = QLabel("no packet sent yet")
layout.addRow("Sequence number", resp_seq_num)

resp_block_seq_num = QCheckBox("Block sequence number")
layout.addRow("", resp_block_seq_num)

resp_version = QLabel("no packet sent yet")
layout.addRow("Protocol version", resp_version)

resp_wrong_proto_version = QCheckBox("Wrong protocol version")
layout.addRow("", resp_wrong_proto_version)

resp_iec61851_state = QComboBox()
resp_iec61851_state.addItem("0: A - Not connected")
resp_iec61851_state.addItem("1: B - Connected")
resp_iec61851_state.addItem("2: C - Charging")
resp_iec61851_state.addItem("3: D - Not supported")
resp_iec61851_state.addItem("4: E/F - Error")
layout.addRow("IEC state", resp_iec61851_state)

resp_charger_state = QComboBox()
resp_charger_state.addItem("0: Not connected")
resp_charger_state.addItem("1: Waiting for release")
resp_charger_state.addItem("2: Ready")
resp_charger_state.addItem("3: Charging")
resp_charger_state.addItem("4: Error")
layout.addRow("Vehicle state", resp_charger_state)

resp_error_state = QSpinBox()
resp_error_state.setMinimum(0)
resp_error_state.setMaximum(5)
layout.addRow("Error state", resp_error_state)

resp_uptime = QLabel("no packet sent yet")
layout.addRow("Uptime", resp_uptime)

resp_block_uptime = QCheckBox("Block uptime")
layout.addRow("", resp_block_uptime)

resp_charging_time = QLabel("no packet sent yet")
layout.addRow("Charging time", resp_charging_time)

resp_allowed_charging_current = QSpinBox()
resp_allowed_charging_current.setMinimum(0)
resp_allowed_charging_current.setMaximum(32)
resp_allowed_charging_current.setSuffix(" A")
layout.addRow("Allowed charging current", resp_allowed_charging_current)

resp_supported_current = QSpinBox()
resp_supported_current.setMinimum(0)
resp_supported_current.setMaximum(32)
resp_supported_current.setSuffix(" A")
layout.addRow("Supported current", resp_supported_current)

resp_managed = QCheckBox("")
resp_managed.setChecked(True)
layout.addRow("Managed", resp_managed)

resp_cp_disconnect = QCheckBox("CP disconnected")
layout.addRow("CP disconnect state", resp_cp_disconnect)


next_seq_num = 0
protocol_version = 1
start = time.time()
charging_time_start = 0

def recieve():
    global addr
    global charging_time_start
    try:
        data, addr = sock.recvfrom(command_len)
    except BlockingIOError:
        return
    if len(data) != command_len:
        return

    magic, length, seq_num, version, allocated_current, command_flags = struct.unpack(command_format, data)

    req_seq_num.setText(str(seq_num))
    req_version.setText(str(version))
    req_allocated_current.setText("{:2.3f} A".format(allocated_current / 1000.0))
    req_cp_disconnect.setText(str((command_flags & 0x40) >> 6))
    if allocated_current == 0 and resp_iec61851_state.currentIndex() == 2:
        charging_time_start = 0
        resp_iec61851_state.setCurrentIndex(1)
        resp_charger_state.setCurrentIndex(1)

def send():
    global next_seq_num
    global start
    global charging_time_start
    global addr

    if addr is None:
        return

    resp_seq_num.setText(str(next_seq_num))
    resp_version.setText(str(protocol_version))

    if resp_block_uptime.isChecked():
        uptime = int(resp_uptime.text().split(" ")[0])
    else:
        uptime = int((time.time() - start) * 1000)
    resp_uptime.setText("{} ms".format(uptime))

    if charging_time_start == 0 and resp_iec61851_state.currentIndex() == 2:
        charging_time_start = time.time()

    if charging_time_start != 0 and resp_iec61851_state.currentIndex() == 0:
        charging_time_start = 0

    charging_time = 0 if charging_time_start == 0 else int((time.time() - charging_time_start) * 1000)
    resp_charging_time.setText("{} ms".format(charging_time))

    flags = 0
    flags |= 0x80 if resp_managed.isChecked() else 0
    flags |= 0x40 if resp_cp_disconnect.isChecked() else 0

    b = struct.pack(state_format,
                    34127,                                      # magic
                    state_len,                                  # length
                    next_seq_num,
                    protocol_version if not resp_wrong_proto_version.isChecked() else 0,
                    0,                                          # features
                    uid,
                    uptime,
                    charging_time,
                    resp_allowed_charging_current.value() * 1000,
                    resp_supported_current.value() * 1000,
                    resp_iec61851_state.currentIndex(),
                    resp_charger_state.currentIndex(),
                    resp_error_state.value(),
                    flags,
                    0,  # LV0
                    0,  # LV1
                    0,  # LV2
                    0,  # LC0
                    0,  # LC1
                    0,  # LC2
                    0,  # LPF0
                    0,  # LPF1
                    0,  # LPF2
                    0,  # power_total
                    0,  # energy_rel
                    0)  # energy_abs
    if not resp_block_seq_num.isChecked():
        next_seq_num += 1
        next_seq_num %= 65536

    sock.sendto(b, addr)

recv_timer = QTimer()
recv_timer.timeout.connect(recieve)
recv_timer.start(100)

send_timer = QTimer()
send_timer.timeout.connect(send)
send_timer.start(1000)

window.setLayout(layout)
window.show()
app.exec_()
