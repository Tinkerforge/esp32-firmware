#!/usr/bin/env -S uv run --group iso15118 --script

import sys
import os
import json
import re
from datetime import datetime, timezone
import scapy.layers.l2
import scapy.utils

QCA700X_ETHERNET_FRAME_MIN_SIZE = 60
QCA700X_RECV_HEADER_SIZE     = (4+4+2+2)
QCA700X_RECV_FOOTER_SIZE     = 2
QCA700X_RECV_BUFFER_MIN_SIZE = (QCA700X_ETHERNET_FRAME_MIN_SIZE + QCA700X_RECV_HEADER_SIZE + QCA700X_RECV_FOOTER_SIZE)

def get_boot_epoch(data):
    # Extract uptime (ms) from the first JSON key in the report
    m = re.search(r'"uptime":(\d+)', data)
    if not m:
        return None
    uptime_ms = int(m.group(1))

    # Extract rtc/time
    m = re.search(r'"rtc/time":\s*(\{[^}]+\})', data)
    if not m:
        return None
    try:
        rtc = json.loads(m.group(1))
        rtc_dt = datetime(rtc['year'], rtc['month'], rtc['day'],
                          rtc['hour'], rtc['minute'], rtc['second'],
                          tzinfo=timezone.utc)
    except (json.JSONDecodeError, KeyError, ValueError):
        return None

    return rtc_dt.timestamp() - uptime_ms / 1000.0

def main(filepath):
    filename = os.path.basename(filepath)
    pcap_filename = os.path.splitext(filename)[0] + ".pcap"

    packet_list = []

    with open(filepath, "r") as f:
        data = f.read()
        # Note: This assumes that there has not yet been a uptime-overflow
        boot_epoch = get_boot_epoch(data)
        if boot_epoch is None:
            print("Could not determine RTC/uptime from debug report, using uptime-only timestamps.")
            boot_epoch = 0

        iso_data = data.split('__begin_iso15118_ll__\n')[1].split('__end_iso15118_ll__')[0]
        qca_list = []
        for line in iso_data.splitlines():
            t, qca = line.split(' ')
            qca_list.extend([int(qca[i:i+2], 16) for i in range(0, len(qca), 2)])

            # TODO: This is not perfect. If there is a recv-buffer without footer
            #       and the next packet is a send-buffer it will append the send buffer to
            #       the recv-buffer. We can only fix this by adding a r/w flag to the debug data

            if len(qca_list) < 8:
                continue

            minimum = QCA700X_RECV_BUFFER_MIN_SIZE
            if not (qca_list[4] == 0xaa and qca_list[5] == 0xaa and qca_list[6] == 0xaa and qca_list[7] == 0xaa):
                minimum = 60

            while len(qca_list) >= minimum:
                # Remove QCA header and footer if it is in the data.
                # The QCA stuff is in the received data, but not in the written data.
                if qca_list[4] == 0xaa and qca_list[5] == 0xaa and qca_list[6] == 0xaa and qca_list[7] == 0xaa:
                    ethernet_frame_length = qca_list[8] | (qca_list[9] << 8)
                    qca_list_packet = qca_list[QCA700X_RECV_HEADER_SIZE:ethernet_frame_length+QCA700X_RECV_HEADER_SIZE]
                    qca_list = qca_list[QCA700X_RECV_HEADER_SIZE+ethernet_frame_length+QCA700X_RECV_FOOTER_SIZE:]
                else:
                    qca_list_packet = qca_list
                    qca_list = []

                packet = scapy.layers.l2.Ether(bytearray(qca_list_packet))
                packet.time = boot_epoch + int(t)/1000.0
                packet_list.append(packet)

    scapy.utils.wrpcap(pcap_filename, packet_list)

if "__main__" == __name__:
    if len(sys.argv) < 2:
        print(f"usage: {sys.argv[0]} {{DEBUG_REPORT_PATH}}")
        sys.exit(1)

    main(sys.argv[1])
