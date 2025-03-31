#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import scapy.layers.l2
import scapy.utils

def main(filepath):
    filename = os.path.basename(filepath)
    pcap_filename = os.path.splitext(filename)[0] + ".pcap"

    packet_list = []

    with open(filepath, "r") as f:
        data = f.read()
        iso_data = data.split('__begin_iso15118_ll__\n')[1].split('__end_iso15118_ll__')[0]
        for line in iso_data.splitlines():
            t, qca = line.split(' ')
            qca_list = [int(qca[i:i+2], 16) for i in range(0, len(qca), 2)]

            # Remove QCA header and footer if it is in the data.
            # The QCA stuff is in the received data, but not in the written data.
            if qca_list[4] == 0xaa and qca_list[5] == 0xaa and qca_list[6] == 0xaa and qca_list[7] == 0xaa and qca_list[-2] == 0x55 and qca_list[-1] == 0x55:
                qca_list = qca_list[12:-2]

            packet = scapy.layers.l2.Ether(bytearray(qca_list))
            packet.time = int(t)/1000.0
            packet_list.append(packet)

    scapy.utils.wrpcap(pcap_filename, packet_list)

if "__main__" == __name__:
    if len(sys.argv) < 2:
        print(f"usage: {sys.argv[0]} {{DEBUG_REPORT_PATH}}")
        sys.exit(1)

    main(sys.argv[1])