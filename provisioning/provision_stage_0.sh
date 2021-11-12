#!/bin/bash

pushd ../../firmwares
git pull
popd

parallel -j 9 -u python3 provision_stage_0.py ../../firmwares/bricks/warp_charger/brick_warp_charger_firmware_latest.bin '{}' ::: /dev/ttyUSB*
