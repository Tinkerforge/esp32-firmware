#!/bin/bash

pushd ../../firmwares
git pull
popd

parallel -j 9 -u python3 provision_stage_0_warp2.py ../../firmwares/bricks/warp_energy_manager_v2/brick_warp_energy_manager_v2_firmware_latest.bin '{}' energy_manager_v2 ::: /dev/ttyUSB*
