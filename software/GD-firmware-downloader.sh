#!/bin/bash
#
# First parameter to the script can be a firmware file name, but it is optional.
# Default is set below.

FW="${1:-AC011K-AE-25_V1.2.460}"
PREFIX="src/modules/ac011k/"

function finish {
  popd > /dev/null
}
trap finish EXIT

# main

pushd ${PREFIX} > /dev/null

if ! command -v curl &> /dev/null
then
    echo "Curl could not be found, but is needed to download the GD firmware."
    echo "Please install curl (https://curl.se/)."
    exit 1
fi

if ! command -v srec_cat &> /dev/null
then
    echo "Srec_cat could not be found, but is needed to convert the GD firmware to binary."
    echo "Please install srec_cat, it is part of the SRecord package (https://srecord.sourceforge.net/download.html)."
    exit 1
fi

if ! command -v xxd &> /dev/null
then
    echo "Xxd could not be found, but is needed to convert the GD firmware for embedding."
    echo "Please install xxd (it's part of the vim-common package)."
    exit 1
fi


if [[ -r $FW.hex ]] && [[ ! -s $FW.hex ]] ; then
    echo "The existing firmware file $FW.hex was empty, so I removed it right away."
    rm $FW.hex
fi
if ! [[ -r $FW.hex ]]
then
    if curl --silent --show-error --remote-header-name --remote-name --user-agent "esp-idf/1.0 esp32" "https://ocpp.dev.en-plus.cn/Charge/img/file/getUpgradeFileForMesh/$FW.hex" ; then
        echo "GD firmware $FW.hex downloaded."
        if [[ ! -s $FW.hex ]] ; then
            echo "But the file was empty. Exiting."
            exit 1
        fi
    else
        echo "Could not download GD firmware $FW.hex. Exiting."
        exit 1
    fi
else
    echo "$FW.hex already there."
fi

if ! [[ -r $FW.bin ]]
then
    if srec_cat '(' $FW.hex -intel -fill 0xff -over $FW.hex -intel ')' -offset -0x8000000 -o $FW.bin -binary 2> >(grep -v "ignoring garbage") ; then
        echo "GD firmware $FW.hex converted to $FW.bin."
    else
        echo "Could not convert GD firmware $FW.hex to $FW.bin. Exiting."
        exit 1
    fi
else
    echo "$FW.bin already there."
fi


if ! command -v sed &> /dev/null
then
    echo "Sed could not be found, but is needed to convert the GD firmware for embedding."
    echo "Please install sed. Alternatively you can edit the GD_firmware.h file by hand and replace 'unsigned char' with 'const unsigned char'."
fi

if ! [[ -r GD_firmware.h ]]
then
    ln -sf $FW.bin GD_firmware
    if xxd -i GD_firmware GD_firmware.h && sed -i '' 's/^unsigned char/const unsigned char/' GD_firmware.h ; then
        echo "GD firmware $FW.bin converted to GD_firmware.h."
    else
        echo "Could not convert GD firmware $FW.bin to GD_firmware.h. Exiting."
        exit 1
    fi
else
    echo "GD_firmware.h already there."
fi

echo "GD firmware ready to be embedded. :-)"

