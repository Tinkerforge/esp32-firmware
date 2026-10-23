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
git clone https://github.com/espressif/esp32-arduino-lib-builder
cd esp32-arduino-lib-builder
../esp32-firmware/software/lib-builder/apply_patches.py . ../esp32-firmware/software/patches/lib-builder-pre
AR_SOURCE_BRANCH="tf-3-3-12" ./build.sh -t esp32 -b reconfigure # Replace branch name with the one you want to build from the arduino-esp32 git.
cp sdkconfig sdkconfig.vanilla
```

### Apply patches to ESP-IDF and components

```bash
cd ~/esp32-arduino-lib-builder
../esp32-firmware/software/lib-builder/apply_patches.py . ../esp32-firmware/software/patches/lib-builder
```

**Replace .esp32brick with .esp32ethernetbrick in the next sections for ESP32 Ethernet, WARP2, WARP3 and WEM firmwares**

**For WARP4, use .esp32secure instead.**

### To modify settings:

```bash
cd ~/esp32-arduino-lib-builder
../esp32-firmware/software/lib-builder/menuconfig.py . ../esp32-firmware/software/lib-builder/defconfig.esp32brick
# press [s] [return] [return] to write sdkconfig (don't change the filename!), [q] to quit, ~/esp32-firmware/software/lib-builder/defconfig.esp32brick will be updated automatically
```

### To build the libs:

```bash
cd ~/esp32-arduino-lib-builder
../esp32-firmware/software/lib-builder/build_idf_libs.py . ../esp32-firmware/software/lib-builder/defconfig.esp32brick
```

### Test built libs:

```bash
# Update platformio.ini's platform_packages entry (of base_esp32 for WARP1, base_esp32_ethernet for WARP2/3/WEM)
# to point to the output directory of the lib builder:
# Change
platform_packages = platformio/framework-arduinoespressif32-libs @ symlink://packages/esp32-arduino-libs#warp-x.y.z_commit_id
# to
platform_packages = platformio/framework-arduinoespressif32-libs @ symlink:///home/[user]/esp32-arduino-lib-builder/out/tools/esp32-arduino-libs
# Then rebuild the firmware as usual
uv run pio run -e warp
```

### Ethernet sharing on the WiFi access point

The WiFi AP setting `share_ethernet` uses ESP-IDF's IPv4 NAPT support. It
defaults to false. The Ethernet and secure defconfigs enable
`CONFIG_LWIP_IP_FORWARD` and `CONFIG_LWIP_IPV4_NAPT`; the ESP32 Brick without
Ethernet keeps forwarding disabled. Per-interface DNS keeps Ethernet's DNS
separate from a simultaneous WiFi station connection. Apply the lwIP patches
as described above:
patch 0005 keeps forwarding disabled unless a NAPT interface is active.

For WARP4, build with `defconfig.esp32secure` and point `platform_packages` in
`warp4.ini` at the resulting libraries, using the same `symlink://` syntax as
in "Test built libs" above. Build the firmware with `uv run pio run -e warp4`.
Changing a defconfig does not modify the currently pinned precompiled packages.
Until updated packages are published, a local library build is required; the
web interface disables sharing when the compiled libraries lack NAPT support.

In the web interface, select **Network > WiFi Access Point**, set the access
point to **Enabled**, then enable **Share Ethernet internet**. Sharing requires
Ethernet to have an IPv4 address and gateway and to be the default interface.
The AP and Ethernet subnets must not overlap. DHCP clients receive the AP's
address as their gateway and Ethernet's IPv4 DNS server. Reconnect clients after
changing sharing settings or the Ethernet DNS server so they renew their leases.
Ethernet link loss disables NAPT; reconnecting enables it again automatically.

For hardware verification, connect a client to the AP and check its DHCP
gateway and DNS, IPv4 internet access by address and by hostname, Ethernet
unplug/replug, and persistence across reboot. With sharing disabled, verify
that AP clients can still open the device's web interface but cannot route
through it. Also test DHCP and static Ethernet configurations and overlapping
subnets. API validation tests are in `src/modules/wifi/tests/ethernet_sharing.py`.

### Publish built libraries

```bash
(git clone https://github.com/Tinkerforge/esp32-arduino-libs)
# Go to https://github.com/Tinkerforge/esp32-arduino-libs and click "sync fork" (so that our main == espressif's main)
cd ~/esp32-arduino-libs
git switch main
git pull
git checkout -b tf-esp-[yyyymmdd] #or tf-esp-ethernet-[yyyymmdd] for ESP32 Ethernet Brick
cp ~/esp32-arduino-lib-builder/out/tools/esp32-arduino-libs/* .
git add .
git commit -m "Update precompiled libs."
git push
```

### Update packages

```bash
cd ~/esp32-firmware/software
python manage_packages.py
```
