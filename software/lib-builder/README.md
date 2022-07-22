## Tutorial to build the precompiled ESP-IDF libs for the arduino-esp32 fork used in our firmwares.

Ignore this if you only want to build/modify firmwares!
This is only relevant if you want to build updates for https://github.com/Tinkerforge/arduino-esp32.

(Skip to "Setup lib builder" if on debian-esque distribution)

### Create container
(see https://wiki.archlinux.org/title/systemd-nspawn#Create_a_Debian_or_Ubuntu_environment)

Install debootstrap and ubuntu-keyring.

    sudo debootstrap --include=systemd-container --components=main,universe jammy esp32-lib-builder http://archive.ubuntu.com/ubuntu
    sudo systemd-nspawn -D ./esp32-lib-builder
    passwd
    [set password]
    logout

### Boot container

    sudo systemd-nspawn --resolv-conf=replace-host -b -D ./esp32-lib-builder
    [login as root]
    adduser user
    adduser user sudo
    logout
    [login as user]

### Setup lib builder
(see https://github.com/espressif/esp32-arduino-lib-builder README)

    sudo apt update
    sudo apt-get install jq git wget curl libssl-dev libncurses-dev flex bison gperf python3 python-is-python3 python3-pip python3-setuptools python3-serial python3-click python3-cryptography python3-future python3-pyparsing python3-pyelftools cmake ninja-build ccache python3-venv libffi-dev libssl-dev dfu-util libusb-1.0-0
    git config --global user.name username
    git config --global user.email user@example.com
    sudo pip install --upgrade pip
    cd ~
    git clone https://github.com/espressif/esp32-arduino-lib-builder
    cd esp32-arduino-lib-builder
    ./build.sh -t esp32 -b menuconfig
    [Press S Enter to write sdkconfig, Q to quit]
    cp sdkconfig sdkconfig.vanilla

### Clone esp32-firmware

    cd ~
    git clone https://github.com/Tinkerforge/esp32-firmware

### Apply patches to ESP-IDF and components

    ~/esp32-firmware/software/lib-builder/apply_patches.py ~/esp32-arduino-lib-builder ~/esp32-firmware/software/patches/lib-builder

(Replace .esp32brick with .esp32ethernetbrick in the next sections if necessary)

### To modify settings:

    ~/esp32-firmware/software/lib-builder/menuconfig.py ~/esp32-arduino-lib-builder ~/esp32-firmware/software/lib-builder/defconfig.esp32brick

Press S Enter to write sdkconfig (don't change the filename!), ~/esp32-firmware/software/lib-builder/defconfig.esp32brick will be updated automatically

### To build the libs:

    ~/esp32-firmware/software/lib-builder/build_idf_libs.py ~/esp32-arduino-lib-builder ~/esp32-firmware/software/lib-builder/defconfig.esp32brick

### Add built libs to arduino-esp32 Repo:

    (git clone https://github.com/Tinkerforge/arduino-esp32)
    cd ~/arduino-esp32
    git switch master
    git pull
    git checkout -b warp-x.y.z (or warp2-x.y.z for ESP32 Ethernet Brick)
    rm platform.txt
    rm -r tools/sdk
    cp ~/esp32-arduino-lib-builder/out/* ~/arduino-esp32/

### Apply patches to arduino-esp32

    ~/esp32-firmware/software/lib-builder/apply_patches.py ~/arduino-esp32 ../patches/arduino-esp32
