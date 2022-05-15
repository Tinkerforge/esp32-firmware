#!/bin/bash

pushd ../../firmwares
git pull
popd

parallel -j 9 -u python3 provision_stage_0_warp2.py ../../firmwares/bricks/esp32_ethernet/brick_esp32_ethernet_firmware_latest.bin '{}' esp32_ethernet ::: /dev/ttyUSB*
