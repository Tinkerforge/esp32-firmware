## Tutorial to build the precompiled ESP-IDF libs for the arduino-esp32 fork used in our firmwares.

Ignore this if you only want to build/modify firmwares!
This is only relevant if you want to build updates for https://github.com/Tinkerforge/arduino-esp32.

(Skip to "Install required packages" if on debian-esque distribution)

### Create container
(see https://wiki.archlinux.org/title/systemd-nspawn#Create_a_Debian_or_Ubuntu_environment)

Install debootstrap and ubuntu-keyring.
```bash
sudo debootstrap --include=systemd-container --components=main,universe jammy esp32-lib-builder http://archive.ubuntu.com/ubuntu
sudo systemd-nspawn -D ./esp32-lib-builder
passwd
# set password
logout
```
### Boot container

```bash
sudo systemd-nspawn --resolv-conf=replace-host -b -D ./esp32-lib-builder
# login as root
adduser user
adduser user sudo
logout
# login as user
git config --global user.name username
git config --global user.email user@example.com
```

### Install required packages

```bash
sudo apt update
sudo apt-get install jq git wget curl libssl-dev libncurses-dev flex bison gperf python3 python-is-python3 python3-pip python3-setuptools python3-serial python3-click python3-cryptography python3-future python3-pyparsing python3-pyelftools cmake ninja-build ccache python3-venv libffi-dev libssl-dev dfu-util libusb-1.0-0
sudo pip install --upgrade pip
```

### Clone esp32-firmware

```bash
cd ~
git clone https://github.com/Tinkerforge/esp32-firmware
```

### Setup lib builder
(see https://github.com/espressif/esp32-arduino-lib-builder README)

```bash
cd ~
git clone --branch release/v4.4 https://github.com/espressif/esp32-arduino-lib-builder
cd esp32-arduino-lib-builder
~/esp32-firmware/software/lib-builder/apply_patches.py ~/esp32-arduino-lib-builder ~/esp32-firmware/software/patches/lib-builder-pre
./build.sh -t esp32 -b menuconfig
# press [s] [return] [return] to write sdkconfig, [q] to quit
cp sdkconfig sdkconfig.vanilla
```

### Apply patches to ESP-IDF and components

```bash
~/esp32-firmware/software/lib-builder/apply_patches.py ~/esp32-arduino-lib-builder ~/esp32-firmware/software/patches/lib-builder
```

**Replace .esp32brick with .esp32ethernetbrick in the next sections for ESP32 Ethernet, WARP2, WARP3 and WEM firmwares**

### To modify settings:

```bash
~/esp32-firmware/software/lib-builder/menuconfig.py ~/esp32-arduino-lib-builder ~/esp32-firmware/software/lib-builder/defconfig.esp32brick
# press [s] [return] to write sdkconfig (don't change the filename!), ~/esp32-firmware/software/lib-builder/defconfig.esp32brick will be updated automatically
```


### To build the libs:

```bash
~/esp32-firmware/software/lib-builder/build_idf_libs.py ~/esp32-arduino-lib-builder ~/esp32-firmware/software/lib-builder/defconfig.esp32brick
```

### Test built libs:

```bash
cd ~/esp32-firmware/software/packages/

# Copy the latest package; Directories ending in -dev are not cleaned up by the firmware build scripts. Remember to use warp2-x.y.z if building WARP2, WARP3 or WEM firmwares (i.e. something that runs on an ESP with ethernet and PSRAM)
cp -r arduino-esp32#warp-x.y.z_commit_id arduino-esp32#warp-x.y.z-dev
cd arduino-esp32#warp-x.y.z-dev
rm platform.txt
rm -r tools/sdk

# Either
cp ~/esp32-arduino-lib-builder/out/* .
# Or
ln -s ~/esp32-arduino-lib-builder/out/platform.txt platform.txt
ln -s ~/esp32-arduino-lib-builder/out/tools/sdk tools/sdk
# (If you set symlinks as above, you don't have to copy the lib-builder's output every time you recompile)

# Update platformio.ini's platform_packages entry (of env:base_esp32 for WARP1, env.base_esp32_ethernet for WARP2/3/WEM)
# to point to the dev-package:
# Change
platform_packages = platformio/framework-arduinoespressif32 @ symlink://packages/arduino-esp32#warp-x.y.z_commit_id
# to
platform_packages = platformio/framework-arduinoespressif32 @ symlink://packages/arduino-esp32#warp-x.y.z-dev
# Then rebuild the firmware as usual
pio run -e warp
```


### Add built libs to arduino-esp32 Repo:

```bash
(git clone https://github.com/Tinkerforge/arduino-esp32)
# Go to https://github.com/Tinkerforge/arduino-esp32, switch to "release/v2.x" and click "sync fork" (so that our "release/v2.x" == espressif's "release/v2.x")
cd ~/arduino-esp32
git switch release/v2.x
git pull
git checkout -b warp-x.y.z #or warp2-x.y.z for ESP32 Ethernet Brick
rm platform.txt
rm -r tools/sdk
cp ~/esp32-arduino-lib-builder/out/* .
git add .
git commit -m "Add libs for warp-x.y.z."
```

### Apply patches to arduino-esp32

```bash
~/esp32-firmware/software/lib-builder/apply_patches.py ~/arduino-esp32 ../patches/arduino-esp32
```

### Push arduino-esp32

```bash
git push -u origin warp-x.y.z #or warp2-x.y.z for ESP32 Ethernet Brick)
```

### Update packages

```bash
python manage_packages.py
```
