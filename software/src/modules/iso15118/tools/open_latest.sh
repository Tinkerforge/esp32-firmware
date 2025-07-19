#!/bin/bash

latest=$(ls -t /home/olaf/Downloads/warp4-*Debug-Report* | head -1)
./debug_report_to_pcap.py $latest
base_name=$(basename ${latest})
wireshark "${base_name%.txt}.pcap"
