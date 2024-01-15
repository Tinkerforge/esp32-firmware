#!/bin/bash

pushd ../../firmwares
git pull
popd

parallel -j 9 -u python3 provision_stage_0_warp2.py ../../firmwares/bricks/warp2_charger/brick_warp2_charger_firmware_latest.bin '{}' warp3 ::: /dev/ttyUSB*
