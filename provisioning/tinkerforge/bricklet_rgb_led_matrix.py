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

GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletRGBLEDMatrix(Device):
    r"""
    RGB LED Matrix with 8x8 pixel
    """

    DEVICE_IDENTIFIER = 272
    DEVICE_DISPLAY_NAME = 'RGB LED Matrix Bricklet'
    DEVICE_URL_PART = 'rgb_led_matrix' # internal

    CALLBACK_FRAME_STARTED = 11


    FUNCTION_SET_RED = 1
    FUNCTION_GET_RED = 2
    FUNCTION_SET_GREEN = 3
    FUNCTION_GET_GREEN = 4
    FUNCTION_SET_BLUE = 5
    FUNCTION_GET_BLUE = 6
    FUNCTION_SET_FRAME_DURATION = 7
    FUNCTION_GET_FRAME_DURATION = 8
    FUNCTION_DRAW_FRAME = 9
    FUNCTION_GET_SUPPLY_VOLTAGE = 10
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
        Device.__init__(self, uid, ipcon, BrickletRGBLEDMatrix.DEVICE_IDENTIFIER, BrickletRGBLEDMatrix.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletRGBLEDMatrix.FUNCTION_SET_RED] = BrickletRGBLEDMatrix.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRGBLEDMatrix.FUNCTION_GET_RED] = BrickletRGBLEDMatrix.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRGBLEDMatrix.FUNCTION_SET_GREEN] = BrickletRGBLEDMatrix.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRGBLEDMatrix.FUNCTION_GET_GREEN] = BrickletRGBLEDMatrix.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRGBLEDMatrix.FUNCTION_SET_BLUE] = BrickletRGBLEDMatrix.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRGBLEDMatrix.FUNCTION_GET_BLUE] = BrickletRGBLEDMatrix.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRGBLEDMatrix.FUNCTION_SET_FRAME_DURATION] = BrickletRGBLEDMatrix.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRGBLEDMatrix.FUNCTION_GET_FRAME_DURATION] = BrickletRGBLEDMatrix.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRGBLEDMatrix.FUNCTION_DRAW_FRAME] = BrickletRGBLEDMatrix.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRGBLEDMatrix.FUNCTION_GET_SUPPLY_VOLTAGE] = BrickletRGBLEDMatrix.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRGBLEDMatrix.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletRGBLEDMatrix.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRGBLEDMatrix.FUNCTION_SET_BOOTLOADER_MODE] = BrickletRGBLEDMatrix.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRGBLEDMatrix.FUNCTION_GET_BOOTLOADER_MODE] = BrickletRGBLEDMatrix.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRGBLEDMatrix.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletRGBLEDMatrix.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRGBLEDMatrix.FUNCTION_WRITE_FIRMWARE] = BrickletRGBLEDMatrix.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRGBLEDMatrix.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletRGBLEDMatrix.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRGBLEDMatrix.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletRGBLEDMatrix.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRGBLEDMatrix.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletRGBLEDMatrix.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRGBLEDMatrix.FUNCTION_RESET] = BrickletRGBLEDMatrix.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRGBLEDMatrix.FUNCTION_WRITE_UID] = BrickletRGBLEDMatrix.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRGBLEDMatrix.FUNCTION_READ_UID] = BrickletRGBLEDMatrix.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRGBLEDMatrix.FUNCTION_GET_IDENTITY] = BrickletRGBLEDMatrix.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletRGBLEDMatrix.CALLBACK_FRAME_STARTED] = (12, 'I')

        ipcon.add_device(self)

    def set_red(self, red):
        r"""
        Sets the 64 red LED values of the matrix.
        """
        self.check_validity()

        red = list(map(int, red))

        self.ipcon.send_request(self, BrickletRGBLEDMatrix.FUNCTION_SET_RED, (red,), '64B', 0, '')

    def get_red(self):
        r"""
        Returns the red LED values as set by :func:`Set Red`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRGBLEDMatrix.FUNCTION_GET_RED, (), '', 72, '64B')

    def set_green(self, green):
        r"""
        Sets the 64 green LED values of the matrix.
        """
        self.check_validity()

        green = list(map(int, green))

        self.ipcon.send_request(self, BrickletRGBLEDMatrix.FUNCTION_SET_GREEN, (green,), '64B', 0, '')

    def get_green(self):
        r"""
        Returns the green LED values as set by :func:`Set Green`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRGBLEDMatrix.FUNCTION_GET_GREEN, (), '', 72, '64B')

    def set_blue(self, blue):
        r"""
        Sets the 64 blue LED values of the matrix.
        """
        self.check_validity()

        blue = list(map(int, blue))

        self.ipcon.send_request(self, BrickletRGBLEDMatrix.FUNCTION_SET_BLUE, (blue,), '64B', 0, '')

    def get_blue(self):
        r"""
        Returns the blue LED values as set by :func:`Set Blue`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRGBLEDMatrix.FUNCTION_GET_BLUE, (), '', 72, '64B')

    def set_frame_duration(self, frame_duration):
        r"""
        Sets the frame duration.

        Example: If you want to achieve 20 frames per second, you should
        set the frame duration to 50ms (50ms * 20 = 1 second).

        Set this value to 0 to turn the automatic frame write mechanism off.

        Approach:

        * Call :func:`Set Frame Duration` with value > 0.
        * Set LED values for first frame with :func:`Set Red`, :func:`Set Green`, :func:`Set Blue`.
        * Wait for :cb:`Frame Started` callback.
        * Set LED values for second frame with :func:`Set Red`, :func:`Set Green`, :func:`Set Blue`.
        * Wait for :cb:`Frame Started` callback.
        * and so on.

        For frame duration of 0 see :func:`Draw Frame`.
        """
        self.check_validity()

        frame_duration = int(frame_duration)

        self.ipcon.send_request(self, BrickletRGBLEDMatrix.FUNCTION_SET_FRAME_DURATION, (frame_duration,), 'H', 0, '')

    def get_frame_duration(self):
        r"""
        Returns the frame duration as set by :func:`Set Frame Duration`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRGBLEDMatrix.FUNCTION_GET_FRAME_DURATION, (), '', 10, 'H')

    def draw_frame(self):
        r"""
        If you set the frame duration to 0 (see :func:`Set Frame Duration`), you can use this
        function to transfer the frame to the matrix.

        Approach:

        * Call :func:`Set Frame Duration` with 0.
        * Set LED values for first frame with :func:`Set Red`, :func:`Set Green`, :func:`Set Blue`.
        * Call :func:`Draw Frame`.
        * Wait for :cb:`Frame Started` callback.
        * Set LED values for second frame with :func:`Set Red`, :func:`Set Green`, :func:`Set Blue`.
        * Call :func:`Draw Frame`.
        * Wait for :cb:`Frame Started` callback.
        * and so on.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletRGBLEDMatrix.FUNCTION_DRAW_FRAME, (), '', 0, '')

    def get_supply_voltage(self):
        r"""
        Returns the current supply voltage of the Bricklet.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRGBLEDMatrix.FUNCTION_GET_SUPPLY_VOLTAGE, (), '', 10, 'H')

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletRGBLEDMatrix.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletRGBLEDMatrix.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRGBLEDMatrix.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletRGBLEDMatrix.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletRGBLEDMatrix.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletRGBLEDMatrix.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRGBLEDMatrix.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRGBLEDMatrix.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletRGBLEDMatrix.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletRGBLEDMatrix.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRGBLEDMatrix.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletRGBLEDMatrix.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

RGBLEDMatrix = BrickletRGBLEDMatrix # for backward compatibility
