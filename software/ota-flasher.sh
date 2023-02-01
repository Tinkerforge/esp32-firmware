#!/bin/bash
# This script tests if there is a WARP software running on $HOST and if so, it
# flashes the latest $FIRMWARE available in $PROJECT_DIR Over The Air onto it.
#
# Parameters to the script are:
#   1: PROJECT_DIR (defaults to the current working dir)
#   2: BUILD_DIR   (defaults to .pio/build/warp${WARPTYPE})
#   3: HOST        (defaults to 10.0.0.1 [the default WIFI IP in the WARP software])
#
# Parameters can be omitted from the last to the first, but not the other way around.

WARPTYPE="AC011K"
PWD="$(pwd)"

PROJECT_DIR="${1:-$PWD}"
BUILD_DIR="${2:-.pio/build/warp${WARPTYPE}}"
HOST="${3:-10.0.0.1}"

FIRMWARE=${PROJECT_DIR}/build/$(cat $BUILD_DIR/firmware_basename)_merged.bin
INFO=$(curl -s --connect-timeout 2 "http://${HOST}/info/name")

if [ -f "$FIRMWARE" ]
then
    if [[ "$INFO" == *"${WARPTYPE}"* ]]; then
        echo -e "\nflashing\n\t${INFO}\n@ ${HOST} with\n\t${FIRMWARE}\n"
        curl -vv --show-error --fail --data-binary "@${FIRMWARE}" "http://${HOST}/flash_firmware"
    else
        echo "There is no ${WARPTYPE} WARP at ${HOST}, skip OTA flashing \"${FIRMWARE}\""
        exit 1
    fi
else
    echo "No firmware to flash found: \"${FIRMWARE}\""
    exit 1
fi
