#!/bin/bash

pushd ../../firmwares
git pull
popd

parallel -j 9 -u python3 provision_stage_0_warp2.py ../../firmwares/bricks/smart_energy_broker/brick_smart_energy_broker_firmware_latest.bin '{}' smart_energy_broker ::: /dev/ttyUSB*
