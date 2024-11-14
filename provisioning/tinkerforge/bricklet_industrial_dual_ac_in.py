# -*- coding: utf-8 -*-
#############################################################
# This file was automatically generated on 2024-02-27.      #
#                                                           #
# Python Bindings Version 2.1.31                            #
#                                                           #
# If you have a bugfix for this file and want to commit it, #
# please fix the bug in the generator. You can find a link  #
# to the generators git repository on tinkerforge.com       #
#############################################################

from collections import namedtuple

try:
    from .ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data
except (ValueError, ImportError):
    try:
        from ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data
    except (ValueError, ImportError):
        from tinkerforge.ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data

GetValueCallbackConfiguration = namedtuple('ValueCallbackConfiguration', ['period', 'value_has_to_change'])
GetAllValueCallbackConfiguration = namedtuple('AllValueCallbackConfiguration', ['period', 'value_has_to_change'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletIndustrialDualACIn(Device):
    r"""
    2 inputs that can detect AC voltages of up to 230V
    """

    DEVICE_IDENTIFIER = 2174
    DEVICE_DISPLAY_NAME = 'Industrial Dual AC In Bricklet'
    DEVICE_URL_PART = 'industrial_dual_ac_in' # internal

    CALLBACK_VALUE = 8
    CALLBACK_ALL_VALUE = 9


    FUNCTION_GET_VALUE = 1
    FUNCTION_SET_VALUE_CALLBACK_CONFIGURATION = 2
    FUNCTION_GET_VALUE_CALLBACK_CONFIGURATION = 3
    FUNCTION_SET_ALL_VALUE_CALLBACK_CONFIGURATION = 4
    FUNCTION_GET_ALL_VALUE_CALLBACK_CONFIGURATION = 5
    FUNCTION_SET_CHANNEL_LED_CONFIG = 6
    FUNCTION_GET_CHANNEL_LED_CONFIG = 7
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
    FUNCTION_GET_IDENTITY = 255

    CHANNEL_0 = 0
    CHANNEL_1 = 1
    CHANNEL_LED_CONFIG_OFF = 0
    CHANNEL_LED_CONFIG_ON = 1
    CHANNEL_LED_CONFIG_SHOW_HEARTBEAT = 2
    CHANNEL_LED_CONFIG_SHOW_CHANNEL_STATUS = 3
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
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletIndustrialDualACIn.DEVICE_IDENTIFIER, BrickletIndustrialDualACIn.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletIndustrialDualACIn.FUNCTION_GET_VALUE] = BrickletIndustrialDualACIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDualACIn.FUNCTION_SET_VALUE_CALLBACK_CONFIGURATION] = BrickletIndustrialDualACIn.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIndustrialDualACIn.FUNCTION_GET_VALUE_CALLBACK_CONFIGURATION] = BrickletIndustrialDualACIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDualACIn.FUNCTION_SET_ALL_VALUE_CALLBACK_CONFIGURATION] = BrickletIndustrialDualACIn.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIndustrialDualACIn.FUNCTION_GET_ALL_VALUE_CALLBACK_CONFIGURATION] = BrickletIndustrialDualACIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDualACIn.FUNCTION_SET_CHANNEL_LED_CONFIG] = BrickletIndustrialDualACIn.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDualACIn.FUNCTION_GET_CHANNEL_LED_CONFIG] = BrickletIndustrialDualACIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDualACIn.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletIndustrialDualACIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDualACIn.FUNCTION_SET_BOOTLOADER_MODE] = BrickletIndustrialDualACIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDualACIn.FUNCTION_GET_BOOTLOADER_MODE] = BrickletIndustrialDualACIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDualACIn.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletIndustrialDualACIn.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDualACIn.FUNCTION_WRITE_FIRMWARE] = BrickletIndustrialDualACIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDualACIn.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletIndustrialDualACIn.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDualACIn.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletIndustrialDualACIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDualACIn.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletIndustrialDualACIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDualACIn.FUNCTION_RESET] = BrickletIndustrialDualACIn.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDualACIn.FUNCTION_WRITE_UID] = BrickletIndustrialDualACIn.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDualACIn.FUNCTION_READ_UID] = BrickletIndustrialDualACIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDualACIn.FUNCTION_GET_IDENTITY] = BrickletIndustrialDualACIn.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletIndustrialDualACIn.CALLBACK_VALUE] = (11, 'B ! !')
        self.callback_formats[BrickletIndustrialDualACIn.CALLBACK_ALL_VALUE] = (10, '2! 2!')

        ipcon.add_device(self)

    def get_value(self):
        r"""
        Returns the input values as bools, *true* refers to "AC voltage detected" and *false* refers to no AC "voltage detected".
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDualACIn.FUNCTION_GET_VALUE, (), '', 9, '2!')

    def set_value_callback_configuration(self, channel, period, value_has_to_change):
        r"""
        This callback can be configured per channel.

        The period is the period with which the :cb:`Value`
        callback is triggered periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change within the
        period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.
        """
        self.check_validity()

        channel = int(channel)
        period = int(period)
        value_has_to_change = bool(value_has_to_change)

        self.ipcon.send_request(self, BrickletIndustrialDualACIn.FUNCTION_SET_VALUE_CALLBACK_CONFIGURATION, (channel, period, value_has_to_change), 'B I !', 0, '')

    def get_value_callback_configuration(self, channel):
        r"""
        Returns the callback configuration for the given channel as set by
        :func:`Set Value Callback Configuration`.
        """
        self.check_validity()

        channel = int(channel)

        return GetValueCallbackConfiguration(*self.ipcon.send_request(self, BrickletIndustrialDualACIn.FUNCTION_GET_VALUE_CALLBACK_CONFIGURATION, (channel,), 'B', 13, 'I !'))

    def set_all_value_callback_configuration(self, period, value_has_to_change):
        r"""
        The period is the period with which the :cb:`All Value`
        callback is triggered periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change within the
        period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.
        """
        self.check_validity()

        period = int(period)
        value_has_to_change = bool(value_has_to_change)

        self.ipcon.send_request(self, BrickletIndustrialDualACIn.FUNCTION_SET_ALL_VALUE_CALLBACK_CONFIGURATION, (period, value_has_to_change), 'I !', 0, '')

    def get_all_value_callback_configuration(self):
        r"""
        Returns the callback configuration as set by
        :func:`Set All Value Callback Configuration`.
        """
        self.check_validity()

        return GetAllValueCallbackConfiguration(*self.ipcon.send_request(self, BrickletIndustrialDualACIn.FUNCTION_GET_ALL_VALUE_CALLBACK_CONFIGURATION, (), '', 13, 'I !'))

    def set_channel_led_config(self, channel, config):
        r"""
        Each channel has a corresponding LED. You can turn the LED off, on or show a
        heartbeat. You can also set the LED to "Channel Status". In this mode the
        LED is on if the channel is high and off otherwise.

        By default all channel LEDs are configured as "Channel Status".
        """
        self.check_validity()

        channel = int(channel)
        config = int(config)

        self.ipcon.send_request(self, BrickletIndustrialDualACIn.FUNCTION_SET_CHANNEL_LED_CONFIG, (channel, config), 'B B', 0, '')

    def get_channel_led_config(self, channel):
        r"""
        Returns the channel LED configuration as set by :func:`Set Channel LED Config`
        """
        self.check_validity()

        channel = int(channel)

        return self.ipcon.send_request(self, BrickletIndustrialDualACIn.FUNCTION_GET_CHANNEL_LED_CONFIG, (channel,), 'B', 9, 'B')

    def get_spitfp_error_count(self):
        r"""
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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletIndustrialDualACIn.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

    def set_bootloader_mode(self, mode):
        r"""
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

        return self.ipcon.send_request(self, BrickletIndustrialDualACIn.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDualACIn.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

    def set_write_firmware_pointer(self, pointer):
        r"""
        Sets the firmware pointer for :func:`Write Firmware`. The pointer has
        to be increased by chunks of size 64. The data is written to flash
        every 4 chunks (which equals to one page of size 256).

        This function is used by Brick Viewer during flashing. It should not be
        necessary to call it in a normal user program.
        """
        self.check_validity()

        pointer = int(pointer)

        self.ipcon.send_request(self, BrickletIndustrialDualACIn.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

    def write_firmware(self, data):
        r"""
        Writes 64 Bytes of firmware at the position as written by
        :func:`Set Write Firmware Pointer` before. The firmware is written
        to flash every 4 chunks.

        You can only write firmware in bootloader mode.

        This function is used by Brick Viewer during flashing. It should not be
        necessary to call it in a normal user program.
        """
        self.check_validity()

        data = list(map(int, data))

        return self.ipcon.send_request(self, BrickletIndustrialDualACIn.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

    def set_status_led_config(self, config):
        r"""
        Sets the status LED configuration. By default the LED shows
        communication traffic between Brick and Bricklet, it flickers once
        for every 10 received data packets.

        You can also turn the LED permanently on/off or show a heartbeat.

        If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
        """
        self.check_validity()

        config = int(config)

        self.ipcon.send_request(self, BrickletIndustrialDualACIn.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDualACIn.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDualACIn.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletIndustrialDualACIn.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletIndustrialDualACIn.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDualACIn.FUNCTION_READ_UID, (), '', 12, 'I')

    def get_identity(self):
        r"""
        Returns the UID, the UID where the Bricklet is connected to,
        the position, the hardware and firmware version as well as the
        device identifier.

        The position can be 'a', 'b', 'c', 'd', 'e', 'f', 'g' or 'h' (Bricklet Port).
        A Bricklet connected to an :ref:`Isolator Bricklet <isolator_bricklet>` is always at
        position 'z'.

        The device identifier numbers can be found :ref:`here <device_identifier>`.
        |device_identifier_constant|
        """
        return GetIdentity(*self.ipcon.send_request(self, BrickletIndustrialDualACIn.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

IndustrialDualACIn = BrickletIndustrialDualACIn # for backward compatibility
