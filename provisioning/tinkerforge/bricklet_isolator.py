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

GetStatistics = namedtuple('Statistics', ['messages_from_brick', 'messages_from_bricklet', 'connected_bricklet_device_identifier', 'connected_bricklet_uid'])
GetSPITFPBaudrateConfig = namedtuple('SPITFPBaudrateConfig', ['enable_dynamic_baudrate', 'minimum_dynamic_baudrate'])
GetIsolatorSPITFPErrorCount = namedtuple('IsolatorSPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetStatisticsCallbackConfiguration = namedtuple('StatisticsCallbackConfiguration', ['period', 'value_has_to_change'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletIsolator(Device):
    r"""
    Galvanically isolates any Bricklet from any Brick
    """

    DEVICE_IDENTIFIER = 2122
    DEVICE_DISPLAY_NAME = 'Isolator Bricklet'
    DEVICE_URL_PART = 'isolator' # internal

    CALLBACK_STATISTICS = 9


    FUNCTION_GET_STATISTICS = 1
    FUNCTION_SET_SPITFP_BAUDRATE_CONFIG = 2
    FUNCTION_GET_SPITFP_BAUDRATE_CONFIG = 3
    FUNCTION_SET_SPITFP_BAUDRATE = 4
    FUNCTION_GET_SPITFP_BAUDRATE = 5
    FUNCTION_GET_ISOLATOR_SPITFP_ERROR_COUNT = 6
    FUNCTION_SET_STATISTICS_CALLBACK_CONFIGURATION = 7
    FUNCTION_GET_STATISTICS_CALLBACK_CONFIGURATION = 8
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
        Device.__init__(self, uid, ipcon, BrickletIsolator.DEVICE_IDENTIFIER, BrickletIsolator.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 1)

        self.response_expected[BrickletIsolator.FUNCTION_GET_STATISTICS] = BrickletIsolator.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIsolator.FUNCTION_SET_SPITFP_BAUDRATE_CONFIG] = BrickletIsolator.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIsolator.FUNCTION_GET_SPITFP_BAUDRATE_CONFIG] = BrickletIsolator.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIsolator.FUNCTION_SET_SPITFP_BAUDRATE] = BrickletIsolator.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIsolator.FUNCTION_GET_SPITFP_BAUDRATE] = BrickletIsolator.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIsolator.FUNCTION_GET_ISOLATOR_SPITFP_ERROR_COUNT] = BrickletIsolator.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIsolator.FUNCTION_SET_STATISTICS_CALLBACK_CONFIGURATION] = BrickletIsolator.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIsolator.FUNCTION_GET_STATISTICS_CALLBACK_CONFIGURATION] = BrickletIsolator.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIsolator.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletIsolator.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIsolator.FUNCTION_SET_BOOTLOADER_MODE] = BrickletIsolator.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIsolator.FUNCTION_GET_BOOTLOADER_MODE] = BrickletIsolator.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIsolator.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletIsolator.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIsolator.FUNCTION_WRITE_FIRMWARE] = BrickletIsolator.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIsolator.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletIsolator.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIsolator.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletIsolator.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIsolator.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletIsolator.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIsolator.FUNCTION_RESET] = BrickletIsolator.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIsolator.FUNCTION_WRITE_UID] = BrickletIsolator.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIsolator.FUNCTION_READ_UID] = BrickletIsolator.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIsolator.FUNCTION_GET_IDENTITY] = BrickletIsolator.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletIsolator.CALLBACK_STATISTICS] = (26, 'I I H 8s')

        ipcon.add_device(self)

    def get_statistics(self):
        r"""
        Returns statistics for the Isolator Bricklet.
        """
        self.check_validity()

        return GetStatistics(*self.ipcon.send_request(self, BrickletIsolator.FUNCTION_GET_STATISTICS, (), '', 26, 'I I H 8s'))

    def set_spitfp_baudrate_config(self, enable_dynamic_baudrate, minimum_dynamic_baudrate):
        r"""
        The SPITF protocol can be used with a dynamic baudrate. If the dynamic baudrate is
        enabled, the Isolator Bricklet will try to adapt the baudrate for the communication
        between Bricks and Bricklets according to the amount of data that is transferred.

        The baudrate for communication config between
        Brick and Isolator Bricklet can be set through the API of the Brick.

        The baudrate will be increased exponentially if lots of data is sent/received and
        decreased linearly if little data is sent/received.

        This lowers the baudrate in applications where little data is transferred (e.g.
        a weather station) and increases the robustness. If there is lots of data to transfer
        (e.g. Thermal Imaging Bricklet) it automatically increases the baudrate as needed.

        In cases where some data has to transferred as fast as possible every few seconds
        (e.g. RS485 Bricklet with a high baudrate but small payload) you may want to turn
        the dynamic baudrate off to get the highest possible performance.

        The maximum value of the baudrate can be set per port with the function
        :func:`Set SPITFP Baudrate`. If the dynamic baudrate is disabled, the baudrate
        as set by :func:`Set SPITFP Baudrate` will be used statically.
        """
        self.check_validity()

        enable_dynamic_baudrate = bool(enable_dynamic_baudrate)
        minimum_dynamic_baudrate = int(minimum_dynamic_baudrate)

        self.ipcon.send_request(self, BrickletIsolator.FUNCTION_SET_SPITFP_BAUDRATE_CONFIG, (enable_dynamic_baudrate, minimum_dynamic_baudrate), '! I', 0, '')

    def get_spitfp_baudrate_config(self):
        r"""
        Returns the baudrate config, see :func:`Set SPITFP Baudrate Config`.
        """
        self.check_validity()

        return GetSPITFPBaudrateConfig(*self.ipcon.send_request(self, BrickletIsolator.FUNCTION_GET_SPITFP_BAUDRATE_CONFIG, (), '', 13, '! I'))

    def set_spitfp_baudrate(self, baudrate):
        r"""
        Sets the baudrate for a the communication between Isolator Bricklet
        and the connected Bricklet. The baudrate for communication between
        Brick and Isolator Bricklet can be set through the API of the Brick.

        If you want to increase the throughput of Bricklets you can increase
        the baudrate. If you get a high error count because of high
        interference (see :func:`Get SPITFP Error Count`) you can decrease the
        baudrate.

        If the dynamic baudrate feature is enabled, the baudrate set by this
        function corresponds to the maximum baudrate (see :func:`Set SPITFP Baudrate Config`).

        Regulatory testing is done with the default baudrate. If CE compatibility
        or similar is necessary in your applications we recommend to not change
        the baudrate.
        """
        self.check_validity()

        baudrate = int(baudrate)

        self.ipcon.send_request(self, BrickletIsolator.FUNCTION_SET_SPITFP_BAUDRATE, (baudrate,), 'I', 0, '')

    def get_spitfp_baudrate(self):
        r"""
        Returns the baudrate, see :func:`Set SPITFP Baudrate`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIsolator.FUNCTION_GET_SPITFP_BAUDRATE, (), '', 12, 'I')

    def get_isolator_spitfp_error_count(self):
        r"""
        Returns the error count for the communication between Isolator Bricklet and
        the connected Bricklet. Call :func:`Get SPITFP Error Count` to get the
        error count between Isolator Bricklet and Brick.

        The errors are divided into

        * ACK checksum errors,
        * message checksum errors,
        * framing errors and
        * overflow errors.
        """
        self.check_validity()

        return GetIsolatorSPITFPErrorCount(*self.ipcon.send_request(self, BrickletIsolator.FUNCTION_GET_ISOLATOR_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

    def set_statistics_callback_configuration(self, period, value_has_to_change):
        r"""
        The period is the period with which the :cb:`Statistics`
        callback is triggered periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change within the
        period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.

        .. versionadded:: 2.0.2$nbsp;(Plugin)
        """
        self.check_validity()

        period = int(period)
        value_has_to_change = bool(value_has_to_change)

        self.ipcon.send_request(self, BrickletIsolator.FUNCTION_SET_STATISTICS_CALLBACK_CONFIGURATION, (period, value_has_to_change), 'I !', 0, '')

    def get_statistics_callback_configuration(self):
        r"""
        Returns the callback configuration as set by
        :func:`Set Statistics Callback Configuration`.

        .. versionadded:: 2.0.2$nbsp;(Plugin)
        """
        self.check_validity()

        return GetStatisticsCallbackConfiguration(*self.ipcon.send_request(self, BrickletIsolator.FUNCTION_GET_STATISTICS_CALLBACK_CONFIGURATION, (), '', 13, 'I !'))

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletIsolator.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletIsolator.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIsolator.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletIsolator.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletIsolator.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletIsolator.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIsolator.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIsolator.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletIsolator.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletIsolator.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIsolator.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletIsolator.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

Isolator = BrickletIsolator # for backward compatibility
