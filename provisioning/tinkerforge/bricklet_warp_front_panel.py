# -*- coding: utf-8 -*-
#############################################################
# This file was automatically generated on 2024-11-14.      #
#                                                           #
# Python Bindings Version 2.1.31                            #
#                                                           #
# If you have a bugfix for this file and want to commit it, #
# please fix the bug in the generator. You can find a link  #
# to the generators git repository on tinkerforge.com       #
#############################################################

#### __DEVICE_IS_NOT_RELEASED__ ####

from collections import namedtuple

try:
    from .ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data
except (ValueError, ImportError):
    try:
        from ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data
    except (ValueError, ImportError):
        from tinkerforge.ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data

GetFlashIndex = namedtuple('FlashIndex', ['page_index', 'sub_page_index'])
SetFlashData = namedtuple('SetFlashData', ['next_page_index', 'next_sub_page_index', 'status'])
GetStatusBar = namedtuple('StatusBar', ['ethernet_status', 'wifi_status', 'hours', 'minutes', 'seconds'])
GetDisplayFrontPageIcon = namedtuple('DisplayFrontPageIcon', ['active', 'sprite_index', 'text_1', 'font_index_1', 'text_2', 'font_index_2'])
GetDisplayWifiSetup1 = namedtuple('DisplayWifiSetup1', ['ip_address', 'ssid'])
GetLEDState = namedtuple('LEDState', ['pattern', 'color'])
GetDisplay = namedtuple('Display', ['display', 'countdown'])
GetFlashMetadata = namedtuple('FlashMetadata', ['version_flash', 'version_expected', 'length_flash', 'length_expected', 'checksum_flash', 'checksum_expected'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletWARPFrontPanel(Device):
    r"""
    TBD
    """

    DEVICE_IDENTIFIER = 2179
    DEVICE_DISPLAY_NAME = 'WARP Front Panel Bricklet'
    DEVICE_URL_PART = 'warp_front_panel' # internal



    FUNCTION_SET_FLASH_INDEX = 1
    FUNCTION_GET_FLASH_INDEX = 2
    FUNCTION_SET_FLASH_DATA = 3
    FUNCTION_ERASE_FLASH_SECTOR = 4
    FUNCTION_ERASE_FLASH = 5
    FUNCTION_SET_STATUS_BAR = 6
    FUNCTION_GET_STATUS_BAR = 7
    FUNCTION_SET_DISPLAY_PAGE_INDEX = 8
    FUNCTION_GET_DISPLAY_PAGE_INDEX = 9
    FUNCTION_SET_DISPLAY_FRONT_PAGE_ICON = 10
    FUNCTION_GET_DISPLAY_FRONT_PAGE_ICON = 11
    FUNCTION_SET_DISPLAY_WIFI_SETUP_1 = 12
    FUNCTION_GET_DISPLAY_WIFI_SETUP_1 = 13
    FUNCTION_SET_DISPLAY_WIFI_SETUP_2 = 14
    FUNCTION_GET_DISPLAY_WIFI_SETUP_2 = 15
    FUNCTION_SET_LED_STATE = 16
    FUNCTION_GET_LED_STATE = 17
    FUNCTION_SET_DISPLAY = 18
    FUNCTION_GET_DISPLAY = 19
    FUNCTION_GET_FLASH_METADATA = 20
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

    FLASH_STATUS_OK = 0
    FLASH_STATUS_BUSY = 1
    LED_PATTERN_OFF = 0
    LED_PATTERN_ON = 1
    LED_PATTERN_BLINKING = 2
    LED_PATTERN_BREATHING = 3
    LED_COLOR_GREEN = 0
    LED_COLOR_RED = 1
    LED_COLOR_YELLOW = 2
    DISPLAY_OFF = 0
    DISPLAY_AUTOMATIC = 1
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
        Device.__init__(self, uid, ipcon, BrickletWARPFrontPanel.DEVICE_IDENTIFIER, BrickletWARPFrontPanel.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletWARPFrontPanel.FUNCTION_SET_FLASH_INDEX] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_GET_FLASH_INDEX] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_SET_FLASH_DATA] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_ERASE_FLASH_SECTOR] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_ERASE_FLASH] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_SET_STATUS_BAR] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_GET_STATUS_BAR] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_SET_DISPLAY_PAGE_INDEX] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_GET_DISPLAY_PAGE_INDEX] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_SET_DISPLAY_FRONT_PAGE_ICON] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_GET_DISPLAY_FRONT_PAGE_ICON] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_SET_DISPLAY_WIFI_SETUP_1] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_GET_DISPLAY_WIFI_SETUP_1] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_SET_DISPLAY_WIFI_SETUP_2] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_GET_DISPLAY_WIFI_SETUP_2] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_SET_LED_STATE] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_GET_LED_STATE] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_SET_DISPLAY] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_GET_DISPLAY] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_GET_FLASH_METADATA] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_SET_BOOTLOADER_MODE] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_GET_BOOTLOADER_MODE] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_WRITE_FIRMWARE] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_RESET] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_WRITE_UID] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_READ_UID] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPFrontPanel.FUNCTION_GET_IDENTITY] = BrickletWARPFrontPanel.RESPONSE_EXPECTED_ALWAYS_TRUE


        ipcon.add_device(self)

    def set_flash_index(self, page_index, sub_page_index):
        r"""
        TODO
        """
        self.check_validity()

        page_index = int(page_index)
        sub_page_index = int(sub_page_index)

        self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_SET_FLASH_INDEX, (page_index, sub_page_index), 'I B', 0, '')

    def get_flash_index(self):
        r"""
        TODO
        """
        self.check_validity()

        return GetFlashIndex(*self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_GET_FLASH_INDEX, (), '', 13, 'I B'))

    def set_flash_data(self, data):
        r"""
        TODO
        """
        self.check_validity()

        data = list(map(int, data))

        return SetFlashData(*self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_SET_FLASH_DATA, (data,), '64B', 14, 'I B B'))

    def erase_flash_sector(self, sector_index):
        r"""
        TODO
        """
        self.check_validity()

        sector_index = int(sector_index)

        return self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_ERASE_FLASH_SECTOR, (sector_index,), 'H', 9, 'B')

    def erase_flash(self):
        r"""
        TODO
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_ERASE_FLASH, (), '', 9, 'B')

    def set_status_bar(self, ethernet_status, wifi_status, hours, minutes, seconds):
        r"""
        TODO
        """
        self.check_validity()

        ethernet_status = int(ethernet_status)
        wifi_status = int(wifi_status)
        hours = int(hours)
        minutes = int(minutes)
        seconds = int(seconds)

        self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_SET_STATUS_BAR, (ethernet_status, wifi_status, hours, minutes, seconds), 'I I B B B', 0, '')

    def get_status_bar(self):
        r"""
        TODO
        """
        self.check_validity()

        return GetStatusBar(*self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_GET_STATUS_BAR, (), '', 19, 'I I B B B'))

    def set_display_page_index(self, page_index):
        r"""
        TODO
        """
        self.check_validity()

        page_index = int(page_index)

        self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_SET_DISPLAY_PAGE_INDEX, (page_index,), 'I', 0, '')

    def get_display_page_index(self):
        r"""
        TODO
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_GET_DISPLAY_PAGE_INDEX, (), '', 12, 'I')

    def set_display_front_page_icon(self, icon_index, active, sprite_index, text_1, font_index_1, text_2, font_index_2):
        r"""
        TODO
        """
        self.check_validity()

        icon_index = int(icon_index)
        active = bool(active)
        sprite_index = int(sprite_index)
        text_1 = create_char_list(text_1)
        font_index_1 = int(font_index_1)
        text_2 = create_char_list(text_2)
        font_index_2 = int(font_index_2)

        self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_SET_DISPLAY_FRONT_PAGE_ICON, (icon_index, active, sprite_index, text_1, font_index_1, text_2, font_index_2), 'I ! I 6c B 6c B', 0, '')

    def get_display_front_page_icon(self, icon_index):
        r"""
        TODO
        """
        self.check_validity()

        icon_index = int(icon_index)

        return GetDisplayFrontPageIcon(*self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_GET_DISPLAY_FRONT_PAGE_ICON, (icon_index,), 'I', 35, '! I 10c B 10c B'))

    def set_display_wifi_setup_1(self, ip_address, ssid):
        r"""
        TODO
        """
        self.check_validity()

        ip_address = create_char_list(ip_address)
        ssid = create_char_list(ssid)

        self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_SET_DISPLAY_WIFI_SETUP_1, (ip_address, ssid), '15c 49c', 0, '')

    def get_display_wifi_setup_1(self):
        r"""
        TODO
        """
        self.check_validity()

        return GetDisplayWifiSetup1(*self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_GET_DISPLAY_WIFI_SETUP_1, (), '', 72, '15c 49c'))

    def set_display_wifi_setup_2(self, password):
        r"""
        TODO
        """
        self.check_validity()

        password = create_char_list(password)

        self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_SET_DISPLAY_WIFI_SETUP_2, (password,), '64c', 0, '')

    def get_display_wifi_setup_2(self):
        r"""
        TODO
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_GET_DISPLAY_WIFI_SETUP_2, (), '', 72, '64c')

    def set_led_state(self, pattern, color):
        r"""

        """
        self.check_validity()

        pattern = int(pattern)
        color = int(color)

        self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_SET_LED_STATE, (pattern, color), 'B B', 0, '')

    def get_led_state(self):
        r"""

        """
        self.check_validity()

        return GetLEDState(*self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_GET_LED_STATE, (), '', 10, 'B B'))

    def set_display(self, display):
        r"""

        """
        self.check_validity()

        display = int(display)

        self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_SET_DISPLAY, (display,), 'B', 0, '')

    def get_display(self):
        r"""

        """
        self.check_validity()

        return GetDisplay(*self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_GET_DISPLAY, (), '', 13, 'B I'))

    def get_flash_metadata(self):
        r"""

        """
        self.check_validity()

        return GetFlashMetadata(*self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_GET_FLASH_METADATA, (), '', 32, 'I I I I I I'))

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletWARPFrontPanel.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

WARPFrontPanel = BrickletWARPFrontPanel # for backward compatibility
