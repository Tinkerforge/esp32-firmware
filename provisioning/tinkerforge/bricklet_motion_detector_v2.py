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

GetIndicator = namedtuple('Indicator', ['top_left', 'top_right', 'bottom'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletMotionDetectorV2(Device):
    r"""
    Passive infrared (PIR) motion sensor with 12m range and dimmable backlight
    """

    DEVICE_IDENTIFIER = 292
    DEVICE_DISPLAY_NAME = 'Motion Detector Bricklet 2.0'
    DEVICE_URL_PART = 'motion_detector_v2' # internal

    CALLBACK_MOTION_DETECTED = 6
    CALLBACK_DETECTION_CYCLE_ENDED = 7


    FUNCTION_GET_MOTION_DETECTED = 1
    FUNCTION_SET_SENSITIVITY = 2
    FUNCTION_GET_SENSITIVITY = 3
    FUNCTION_SET_INDICATOR = 4
    FUNCTION_GET_INDICATOR = 5
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

    MOTION_NOT_DETECTED = 0
    MOTION_DETECTED = 1
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
        Device.__init__(self, uid, ipcon, BrickletMotionDetectorV2.DEVICE_IDENTIFIER, BrickletMotionDetectorV2.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletMotionDetectorV2.FUNCTION_GET_MOTION_DETECTED] = BrickletMotionDetectorV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMotionDetectorV2.FUNCTION_SET_SENSITIVITY] = BrickletMotionDetectorV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletMotionDetectorV2.FUNCTION_GET_SENSITIVITY] = BrickletMotionDetectorV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMotionDetectorV2.FUNCTION_SET_INDICATOR] = BrickletMotionDetectorV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletMotionDetectorV2.FUNCTION_GET_INDICATOR] = BrickletMotionDetectorV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMotionDetectorV2.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletMotionDetectorV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMotionDetectorV2.FUNCTION_SET_BOOTLOADER_MODE] = BrickletMotionDetectorV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMotionDetectorV2.FUNCTION_GET_BOOTLOADER_MODE] = BrickletMotionDetectorV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMotionDetectorV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletMotionDetectorV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletMotionDetectorV2.FUNCTION_WRITE_FIRMWARE] = BrickletMotionDetectorV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMotionDetectorV2.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletMotionDetectorV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletMotionDetectorV2.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletMotionDetectorV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMotionDetectorV2.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletMotionDetectorV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMotionDetectorV2.FUNCTION_RESET] = BrickletMotionDetectorV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletMotionDetectorV2.FUNCTION_WRITE_UID] = BrickletMotionDetectorV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletMotionDetectorV2.FUNCTION_READ_UID] = BrickletMotionDetectorV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMotionDetectorV2.FUNCTION_GET_IDENTITY] = BrickletMotionDetectorV2.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletMotionDetectorV2.CALLBACK_MOTION_DETECTED] = (8, '')
        self.callback_formats[BrickletMotionDetectorV2.CALLBACK_DETECTION_CYCLE_ENDED] = (8, '')

        ipcon.add_device(self)

    def get_motion_detected(self):
        r"""
        Returns 1 if a motion was detected. It returns 1 approx. for 1.8 seconds
        until the sensor checks for a new movement.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMotionDetectorV2.FUNCTION_GET_MOTION_DETECTED, (), '', 9, 'B')

    def set_sensitivity(self, sensitivity):
        r"""
        Sets the sensitivity of the PIR sensor. At full
        sensitivity (100), the Bricklet can detect motion in a range of approximately 12m.

        The actual range depends on many things in the environment (e.g. reflections) and the
        size of the object to be detected. While a big person might be detected in a range
        of 10m a cat may only be detected at 2m distance with the same setting.

        So you will have to find a good sensitivity for your application by trial and error.
        """
        self.check_validity()

        sensitivity = int(sensitivity)

        self.ipcon.send_request(self, BrickletMotionDetectorV2.FUNCTION_SET_SENSITIVITY, (sensitivity,), 'B', 0, '')

    def get_sensitivity(self):
        r"""
        Returns the sensitivity as set by :func:`Set Sensitivity`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMotionDetectorV2.FUNCTION_GET_SENSITIVITY, (), '', 9, 'B')

    def set_indicator(self, top_left, top_right, bottom):
        r"""
        Sets the blue backlight of the fresnel lens. The backlight consists of
        three LEDs. The brightness of each LED can be controlled with a 8-bit value
        (0-255). A value of 0 turns the LED off and a value of 255 turns the LED
        to full brightness.
        """
        self.check_validity()

        top_left = int(top_left)
        top_right = int(top_right)
        bottom = int(bottom)

        self.ipcon.send_request(self, BrickletMotionDetectorV2.FUNCTION_SET_INDICATOR, (top_left, top_right, bottom), 'B B B', 0, '')

    def get_indicator(self):
        r"""
        Returns the indicator configuration as set by :func:`Set Indicator`.
        """
        self.check_validity()

        return GetIndicator(*self.ipcon.send_request(self, BrickletMotionDetectorV2.FUNCTION_GET_INDICATOR, (), '', 11, 'B B B'))

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletMotionDetectorV2.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletMotionDetectorV2.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMotionDetectorV2.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletMotionDetectorV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletMotionDetectorV2.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletMotionDetectorV2.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMotionDetectorV2.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMotionDetectorV2.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletMotionDetectorV2.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletMotionDetectorV2.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMotionDetectorV2.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletMotionDetectorV2.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

MotionDetectorV2 = BrickletMotionDetectorV2 # for backward compatibility
