#!/bin/bash

pushd ../../firmwares
git pull
popd

parallel -j 9 -u python3 provision_stage_0_warp2.py ../../firmwares/bricks/energy_manager/brick_energy_manager_firmware_latest.bin '{}' energy_manager ::: /dev/ttyUSB*
