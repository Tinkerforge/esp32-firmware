#!/usr/bin/python3 -u

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'provisioning')

import sys
import time
import zipfile

from provisioning.tinkerforge.ip_connection import IPConnection
from provisioning.tinkerforge.bricklet_unknown import BrickletUnknown

def fail(s):
    print(s)
    sys.exit(1)

def set_comcu_bootloader_mode(bricklet, mode):
    counter = 0

    while True:
        try:
            return bricklet.set_bootloader_mode(mode)
        except:
            pass

        if counter == 10:
            break

        time.sleep(0.25)
        counter += 1

        return None

def wait_for_comcu_bootloader_mode(bricklet, mode):
    counter = 0

    while True:
        try:
            if bricklet.get_bootloader_mode() == mode:
                return True
        except:
            pass

        if counter == 10:
            break

        time.sleep(0.25)
        counter += 1

    return False

def write_firmware(plugin, bricklet, regular_plugin_upto, attempt='first'):
    if set_comcu_bootloader_mode(bricklet, bricklet.BOOTLOADER_MODE_BOOTLOADER) == None or \
        not wait_for_comcu_bootloader_mode(bricklet, bricklet.BOOTLOADER_MODE_BOOTLOADER):
        fail('Device did not enter bootloader mode in 2.5 seconds ({} attempt)'.format(attempt))

    num_packets = len(plugin) // 64

    # If the magic number is in in the last page of the
    # flash, we write the whole thing
    if regular_plugin_upto >= (len(plugin) - 64 * 4):
        index_list = list(range(num_packets))
    else:
        # We write the 64 byte packets up to the end of the last page that has meaningful data
        packet_up_to = ((regular_plugin_upto // 256) + 1) * 4
        index_list = list(range(0, packet_up_to)) + [num_packets - 4, num_packets - 3, num_packets - 2, num_packets - 1]

    print('Writing plugin ({} attempt)'.format(attempt))

    for position in index_list:
        start = position * 64
        end = (position + 1) * 64

        try:
            bricklet.set_write_firmware_pointer(start)
            bricklet.write_firmware(plugin[start:end])
        except:
            # retry block a second time to recover from Co-MCU bootloader
            # bug that results in lost request, especially when used in
            # combination with an Isolator Bricklet

            try:
                bricklet.set_write_firmware_pointer(start)
                bricklet.write_firmware(plugin[start:end])
            except Exception as e:
                fail('Could not write plugin: {} ({} attempt)'.format(e, attempt))

    print('Changing from bootloader mode to firmware mode ({} attempt)'.format(attempt))

    mode_ret = set_comcu_bootloader_mode(bricklet, bricklet.BOOTLOADER_MODE_FIRMWARE)

    if mode_ret == None:
        fail('Device did not enter firmware mode in 2.5 seconds ({} attempt)'.format(attempt))

    return mode_ret

def write_bricklet_plugin_comcu(plugin_path, bricklet):
    print('Starting bootloader mode')

    plugin_data = None

    try:
        with zipfile.ZipFile(plugin_path, 'r') as zf:
            for name in zf.namelist():
                if name.endswith('firmware.bin'):
                    plugin_data = zf.read(name)
                    break
    except Exception as e:
        fail('Could not read *.zbin file: {0}'.format(e))

    if plugin_data == None:
        fail('Could not find firmware in *.zbin file')

    # Now convert plugin to list of bytes
    plugin = plugin_data
    regular_plugin_upto = -1

    for i in reversed(range(4, len(plugin)-12)):
        if plugin[i] == 0x12 and plugin[i-1] == 0x34 and plugin[i-2] == 0x56 and plugin[i-3] == 0x78:
            regular_plugin_upto = i
            break

    if regular_plugin_upto == -1:
        fail('Could not find magic number in firmware')

    mode_ret = write_firmware(plugin, bricklet, regular_plugin_upto)

    if mode_ret != 0 and mode_ret != 2: # 0 = ok, 2 = no change
        if mode_ret == 1:
            error_str = 'Invalid mode (Error 1)'
        elif mode_ret == 3:
            error_str = 'Entry function not present (Error 3)'
        elif mode_ret == 4:
            error_str = 'Device identifier incorrect (Error 4)'
        elif mode_ret == 5:
            error_str = 'CRC Mismatch (Error 5)'
        else: # unknown error case
            error_str = 'Error ' + str(mode_ret)

        # In case of CRC error we try again with whole firmware.
        # If there happens to be garbage data between the actual firmware and the
        # firmware data at the end of the flash, the CRC does not match and we have
        # to overwrite it with zeros.
        # This sometimes seems to be the case with fresh XMCs. This should not
        # happen according to the specification in the datasheet...
        if mode_ret != 5:
            fail('Could not change from bootloader mode to firmware mode: ' + error_str)

        mode_ret = write_firmware(plugin_path, bricklet, len(plugin), 'second')

        if mode_ret != 0 and mode_ret != 2: # 0 = ok, 2 = no change
            if mode_ret == 1:
                error_str = 'Invalid mode (Error 1, second attempt)'
            elif mode_ret == 3:
                error_str = 'Entry function not present (Error 3, second attempt)'
            elif mode_ret == 4:
                error_str = 'Device identifier incorrect (Error 4, second attempt)'
            elif mode_ret == 5:
                error_str = 'CRC Mismatch (Error 5, second attempt)'
            else: # unknown error case
                error_str = 'Error ' + str(mode_ret)

            fail('Could not change from bootloader mode to firmware mode: ' + error_str)

    if not wait_for_comcu_bootloader_mode(bricklet, bricklet.BOOTLOADER_MODE_FIRMWARE):
        fail('Device did not enter firmware mode in 2.5 seconds')

    return True


def main():
    if len(sys.argv) != 3:
        fail("Usage: {} [UID] [path to firmware.zbin]".format(sys.argv[0]))

    _, uid, plugin_path = sys.argv

    ipcon = IPConnection()
    ipcon.connect("localhost", 4223)
    bricklet = BrickletUnknown(uid, ipcon)
    if write_bricklet_plugin_comcu(plugin_path, bricklet):
        print("Done")

if __name__ == "__main__":
    main()
