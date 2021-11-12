import sys
import time
import zipfile

# -*- coding: utf-8 -*-
#############################################################
# This file was automatically generated on 2021-02-03.      #
#                                                           #
# Python Bindings Version 2.1.28                            #
#                                                           #
# If you have a bugfix for this file and want to commit it, #
# please fix the bug in the generator. You can find a link  #
# to the generators git repository on tinkerforge.com       #
#############################################################

#### __DEVICE_IS_NOT_RELEASED__ ####

from collections import namedtuple

from tinkerforge.ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data

GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletUnknown(Device):
    """

    """

    DEVICE_IDENTIFIER = -21
    DEVICE_DISPLAY_NAME = 'Unknown Bricklet'
    DEVICE_URL_PART = 'unknown' # internal

    CALLBACK_ENUMERATE = 253


    FUNCTION_GET_SPITFP_ERROR_COUNT = 234
    FUNCTION_SET_BOOTLOADER_MODE = 235
    FUNCTION_GET_BOOTLOADER_MODE = 236
    FUNCTION_SET_WRITE_FIRMWARE_POINTER = 237
    FUNCTION_WRITE_FIRMWARE = 238
    FUNCTION_SET_STATUS_LED_CONFIG = 239
    FUNCTION_GET_STATUS_LED_CONFIG = 240
    FUNCTION_GET_CHIP_TEMPERATURE = 242
    FUNCTION_RESET = 243
    FUNCTION_WRITE_UID = 248
    FUNCTION_READ_UID = 249
    FUNCTION_COMCU_ENUMERATE = 252
    FUNCTION_ENUMERATE = 254
    FUNCTION_GET_IDENTITY = 255

    ENUMERATION_TYPE_AVAILABLE = 0
    ENUMERATION_TYPE_CONNECTED = 1
    ENUMERATION_TYPE_DISCONNECTED = 2
    BOOTLOADER_MODE_BOOTLOADER = 0
    BOOTLOADER_MODE_FIRMWARE = 1
    BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT = 2
    BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT = 3
    BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT = 4
    BOOTLOADER_STATUS_OK = 0
    BOOTLOADER_STATUS_INVALID_MODE = 1
    BOOTLOADER_STATUS_NO_CHANGE = 2
    BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT = 3
    BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT = 4
    BOOTLOADER_STATUS_CRC_MISMATCH = 5
    STATUS_LED_CONFIG_OFF = 0
    STATUS_LED_CONFIG_ON = 1
    STATUS_LED_CONFIG_SHOW_HEARTBEAT = 2
    STATUS_LED_CONFIG_SHOW_STATUS = 3

    def __init__(self, uid, ipcon):
        """
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletUnknown.DEVICE_IDENTIFIER, BrickletUnknown.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletUnknown.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletUnknown.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletUnknown.FUNCTION_SET_BOOTLOADER_MODE] = BrickletUnknown.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletUnknown.FUNCTION_GET_BOOTLOADER_MODE] = BrickletUnknown.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletUnknown.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletUnknown.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletUnknown.FUNCTION_WRITE_FIRMWARE] = BrickletUnknown.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletUnknown.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletUnknown.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletUnknown.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletUnknown.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletUnknown.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletUnknown.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletUnknown.FUNCTION_RESET] = BrickletUnknown.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletUnknown.FUNCTION_WRITE_UID] = BrickletUnknown.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletUnknown.FUNCTION_READ_UID] = BrickletUnknown.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletUnknown.FUNCTION_COMCU_ENUMERATE] = BrickletUnknown.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletUnknown.FUNCTION_ENUMERATE] = BrickletUnknown.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletUnknown.FUNCTION_GET_IDENTITY] = BrickletUnknown.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletUnknown.CALLBACK_ENUMERATE] = (34, '8s 8s c 3B 3B H B')

        ipcon.add_device(self)

    def get_spitfp_error_count(self):
        """
        Returns the error count for the communication between Brick and Bricklet.

        The errors are divided into

        * ACK checksum errors,
        * message checksum errors,
        * framing errors and
        * overflow errors.

        The errors counts are for errors that occur on the Bricklet side. All
        Bricks have a similar function that returns the errors on the Brick side.
        """
        self.check_validity()

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletUnknown.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

    def set_bootloader_mode(self, mode):
        """
        Sets the bootloader mode and returns the status after the requested
        mode change was instigated.

        You can change from bootloader mode to firmware mode and vice versa. A change
        from bootloader mode to firmware mode will only take place if the entry function,
        device identifier and CRC are present and correct.

        This function is used by Brick Viewer during flashing. It should not be
        necessary to call it in a normal user program.
        """
        self.check_validity()

        mode = int(mode)

        return self.ipcon.send_request(self, BrickletUnknown.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        """
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletUnknown.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

    def set_write_firmware_pointer(self, pointer):
        """
        Sets the firmware pointer for :func:`Write Firmware`. The pointer has
        to be increased by chunks of size 64. The data is written to flash
        every 4 chunks (which equals to one page of size 256).

        This function is used by Brick Viewer during flashing. It should not be
        necessary to call it in a normal user program.
        """
        self.check_validity()

        pointer = int(pointer)

        self.ipcon.send_request(self, BrickletUnknown.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

    def write_firmware(self, data):
        """
        Writes 64 Bytes of firmware at the position as written by
        :func:`Set Write Firmware Pointer` before. The firmware is written
        to flash every 4 chunks.

        You can only write firmware in bootloader mode.

        This function is used by Brick Viewer during flashing. It should not be
        necessary to call it in a normal user program.
        """
        self.check_validity()

        data = list(map(int, data))

        return self.ipcon.send_request(self, BrickletUnknown.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

    def set_status_led_config(self, config):
        """
        Sets the status LED configuration. By default the LED shows
        communication traffic between Brick and Bricklet, it flickers once
        for every 10 received data packets.

        You can also turn the LED permanently on/off or show a heartbeat.

        If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
        """
        self.check_validity()

        config = int(config)

        self.ipcon.send_request(self, BrickletUnknown.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        """
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletUnknown.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        """
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletUnknown.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        """
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletUnknown.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        """
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletUnknown.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        """
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletUnknown.FUNCTION_READ_UID, (), '', 12, 'I')

    def comcu_enumerate(self):
        """
        This function is equivalent to the normal enumerate function.
        It is used to trigger the initial enumeration of CoMCU-Bricklets.
        See :cb:`Enumerate` callback.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletUnknown.FUNCTION_COMCU_ENUMERATE, (), '', 0, '')

    def enumerate(self):
        """
        Broadcasts an enumerate request. All devices will respond with an :cb:`Enumerate` callback.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletUnknown.FUNCTION_ENUMERATE, (), '', 0, '')

    def get_identity(self):
        """
        Returns the UID, the UID where the Bricklet is connected to,
        the position, the hardware and firmware version as well as the
        device identifier.

        The position can be 'a', 'b', 'c', 'd', 'e', 'f', 'g' or 'h' (Bricklet Port).
        A Bricklet connected to an :ref:`Isolator Bricklet <isolator_bricklet>` is always at
        position 'z'.

        The device identifier numbers can be found :ref:`here <device_identifier>`.
        |device_identifier_constant|
        """
        return GetIdentity(*self.ipcon.send_request(self, BrickletUnknown.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        """
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

Unknown = BrickletUnknown # for backward compatibility

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
