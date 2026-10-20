# -*- coding: utf-8 -*-
#############################################################
# This file was automatically generated on 2026-10-19.      #
#                                                           #
# Python Bindings Version 2.1.32                            #
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

GetDateTime = namedtuple('DateTime', ['seconds', 'minutes', 'hours', 'days', 'days_of_week', 'month', 'year'])
GetSDInformation = namedtuple('SDInformation', ['sd_status', 'lfs_status', 'sector_size', 'sector_count', 'card_type', 'product_rev', 'product_name', 'manufacturer_id'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletWARPESP32EthernetV2Co(Device):
    r"""
    Real-time clock, temperature sensor and status LED for the WARP ESP32 Ethernet 2.0
    """

    DEVICE_IDENTIFIER = 2184
    DEVICE_DISPLAY_NAME = 'WARP ESP32 Ethernet 2.0 Co Bricklet'
    DEVICE_URL_PART = 'warp_esp32_ethernet_v2_co' # internal

    CALLBACK_RMMI_INTERRUPT = 9


    FUNCTION_SET_LED = 1
    FUNCTION_GET_LED = 2
    FUNCTION_GET_TEMPERATURE = 3
    FUNCTION_SET_DATE_TIME = 4
    FUNCTION_GET_DATE_TIME = 5
    FUNCTION_GET_UPTIME = 6
    FUNCTION_FORMAT_SD = 7
    FUNCTION_GET_SD_INFORMATION = 8
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

    DATA_STATUS_OK = 0
    DATA_STATUS_SD_ERROR = 1
    DATA_STATUS_LFS_ERROR = 2
    DATA_STATUS_QUEUE_FULL = 3
    DATA_STATUS_DATE_OUT_OF_RANGE = 4
    FORMAT_STATUS_OK = 0
    FORMAT_STATUS_PASSWORD_ERROR = 1
    FORMAT_STATUS_FORMAT_ERROR = 2
    LED_STATE_OFF = 0
    LED_STATE_ON = 1
    LED_STATE_AUTO = 2
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
        Device.__init__(self, uid, ipcon, BrickletWARPESP32EthernetV2Co.DEVICE_IDENTIFIER, BrickletWARPESP32EthernetV2Co.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletWARPESP32EthernetV2Co.FUNCTION_SET_LED] = BrickletWARPESP32EthernetV2Co.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPESP32EthernetV2Co.FUNCTION_GET_LED] = BrickletWARPESP32EthernetV2Co.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPESP32EthernetV2Co.FUNCTION_GET_TEMPERATURE] = BrickletWARPESP32EthernetV2Co.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPESP32EthernetV2Co.FUNCTION_SET_DATE_TIME] = BrickletWARPESP32EthernetV2Co.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPESP32EthernetV2Co.FUNCTION_GET_DATE_TIME] = BrickletWARPESP32EthernetV2Co.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPESP32EthernetV2Co.FUNCTION_GET_UPTIME] = BrickletWARPESP32EthernetV2Co.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPESP32EthernetV2Co.FUNCTION_FORMAT_SD] = BrickletWARPESP32EthernetV2Co.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPESP32EthernetV2Co.FUNCTION_GET_SD_INFORMATION] = BrickletWARPESP32EthernetV2Co.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPESP32EthernetV2Co.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletWARPESP32EthernetV2Co.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPESP32EthernetV2Co.FUNCTION_SET_BOOTLOADER_MODE] = BrickletWARPESP32EthernetV2Co.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPESP32EthernetV2Co.FUNCTION_GET_BOOTLOADER_MODE] = BrickletWARPESP32EthernetV2Co.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPESP32EthernetV2Co.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletWARPESP32EthernetV2Co.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPESP32EthernetV2Co.FUNCTION_WRITE_FIRMWARE] = BrickletWARPESP32EthernetV2Co.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPESP32EthernetV2Co.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletWARPESP32EthernetV2Co.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPESP32EthernetV2Co.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletWARPESP32EthernetV2Co.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPESP32EthernetV2Co.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletWARPESP32EthernetV2Co.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPESP32EthernetV2Co.FUNCTION_RESET] = BrickletWARPESP32EthernetV2Co.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPESP32EthernetV2Co.FUNCTION_WRITE_UID] = BrickletWARPESP32EthernetV2Co.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPESP32EthernetV2Co.FUNCTION_READ_UID] = BrickletWARPESP32EthernetV2Co.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPESP32EthernetV2Co.FUNCTION_GET_IDENTITY] = BrickletWARPESP32EthernetV2Co.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletWARPESP32EthernetV2Co.CALLBACK_RMMI_INTERRUPT] = (8, '')

        ipcon.add_device(self)

    def set_led(self, state):
        r"""
        Sets the state of the status LED. *Off* turns the LED off, *On* turns it on and
        *Auto* lets it blink at about 1 Hz.
        """
        self.check_validity()

        state = int(state)

        self.ipcon.send_request(self, BrickletWARPESP32EthernetV2Co.FUNCTION_SET_LED, (state,), 'B', 0, '')

    def get_led(self):
        r"""
        Returns the state of the status LED as set by :func:`Set LED`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletWARPESP32EthernetV2Co.FUNCTION_GET_LED, (), '', 9, 'B')

    def get_temperature(self):
        r"""
        Returns the temperature measured by the on-board sensor in 1/100 °C.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletWARPESP32EthernetV2Co.FUNCTION_GET_TEMPERATURE, (), '', 10, 'h')

    def set_date_time(self, seconds, minutes, hours, days, days_of_week, month, year):
        r"""
        Sets the date and time of the internal real-time clock. The day of the week is
        0 for Sunday, 1 for Monday and so on.
        """
        self.check_validity()

        seconds = int(seconds)
        minutes = int(minutes)
        hours = int(hours)
        days = int(days)
        days_of_week = int(days_of_week)
        month = int(month)
        year = int(year)

        self.ipcon.send_request(self, BrickletWARPESP32EthernetV2Co.FUNCTION_SET_DATE_TIME, (seconds, minutes, hours, days, days_of_week, month, year), 'B B B B B B H', 0, '')

    def get_date_time(self):
        r"""
        Returns the date and time of the internal real-time clock as set by
        :func:`Set Date Time`.
        """
        self.check_validity()

        return GetDateTime(*self.ipcon.send_request(self, BrickletWARPESP32EthernetV2Co.FUNCTION_GET_DATE_TIME, (), '', 16, 'B B B B B B H'))

    def get_uptime(self):
        r"""
        Returns the uptime of the Bricklet in milliseconds.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletWARPESP32EthernetV2Co.FUNCTION_GET_UPTIME, (), '', 12, 'I')

    def format_sd(self, password):
        r"""
        Not supported by current hardware version.
        """
        self.check_validity()

        password = int(password)

        return self.ipcon.send_request(self, BrickletWARPESP32EthernetV2Co.FUNCTION_FORMAT_SD, (password,), 'I', 9, 'B')

    def get_sd_information(self):
        r"""
        Not supported by current hardware version.
        """
        self.check_validity()

        return GetSDInformation(*self.ipcon.send_request(self, BrickletWARPESP32EthernetV2Co.FUNCTION_GET_SD_INFORMATION, (), '', 33, 'I I H I I B 5c B'))

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletWARPESP32EthernetV2Co.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletWARPESP32EthernetV2Co.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletWARPESP32EthernetV2Co.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletWARPESP32EthernetV2Co.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletWARPESP32EthernetV2Co.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletWARPESP32EthernetV2Co.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletWARPESP32EthernetV2Co.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletWARPESP32EthernetV2Co.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletWARPESP32EthernetV2Co.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletWARPESP32EthernetV2Co.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletWARPESP32EthernetV2Co.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletWARPESP32EthernetV2Co.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

WARPESP32EthernetV2Co = BrickletWARPESP32EthernetV2Co # for backward compatibility
