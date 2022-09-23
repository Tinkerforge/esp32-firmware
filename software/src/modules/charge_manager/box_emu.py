import socket
import struct
import sys
import time

"""
struct packet_header {
    uint8_t seq_num;
    uint8_t version;
    uint16_t padding;
} __attribute__ ((packed));

struct request_packet {
    packet_header header;

    uint16_t allocated_current;
} __attribute__ ((packed));

struct response_packet {
    packet_header header;

    uint8_t iec61851_state;
    uint8_t vehicle_state;
    uint8_t error_state;
    uint32_t uptime;
    uint32_t charging_time;
    uint16_t allowed_charging_current;
    uint16_t supported_current;
    bool managed;
} __attribute__ ((packed));
"""

header_format = "<BBH"
request_format = header_format + "H"
response_format = header_format + "BBBIIHH?"

request_len = struct.calcsize(request_format)
response_len = struct.calcsize(response_format)
print(response_len)


listen_addr = sys.argv[1]

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
resp_error_state.setMaximum(4)
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
resp_supported_current.setMinimum(6)
resp_supported_current.setMaximum(32)
resp_supported_current.setSuffix(" A")
layout.addRow("Supported current", resp_supported_current)

resp_managed = QCheckBox("")
resp_managed.setChecked(True)
layout.addRow("Managed", resp_managed)


next_seq_num = 0
protocol_version = 3
start = time.time()
charging_time_start = 0

def recieve():
    global addr
    global charging_time_start
    try:
        data, addr = sock.recvfrom(request_len)
    except BlockingIOError:
        return
    if len(data) != request_len:
        return

    seq_num, version, _, allocated_current = struct.unpack(request_format, data)

    req_seq_num.setText(str(seq_num))
    req_version.setText(str(version))
    req_allocated_current.setText("{:2.3f} A".format(allocated_current / 1000.0))
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

    b = struct.pack(response_format,
                    next_seq_num,
                    protocol_version if not resp_wrong_proto_version.isChecked() else 234,
                    0,
                    resp_iec61851_state.currentIndex(),
                    resp_charger_state.currentIndex(),
                    resp_error_state.value(),
                    uptime,
                    charging_time,
                    resp_allowed_charging_current.value() * 1000,
                    resp_supported_current.value() * 1000,
                    resp_managed.isChecked())

    if not resp_block_seq_num.isChecked():
        next_seq_num += 1
        next_seq_num %= 256

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
