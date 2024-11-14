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

GetLEDState = namedtuple('LEDState', ['led_l', 'led_r'])
GetButtonState = namedtuple('ButtonState', ['button_l', 'button_r'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletDualButtonV2(Device):
    r"""
    Two tactile buttons with built-in blue LEDs
    """

    DEVICE_IDENTIFIER = 2119
    DEVICE_DISPLAY_NAME = 'Dual Button Bricklet 2.0'
    DEVICE_URL_PART = 'dual_button_v2' # internal

    CALLBACK_STATE_CHANGED = 4


    FUNCTION_SET_LED_STATE = 1
    FUNCTION_GET_LED_STATE = 2
    FUNCTION_GET_BUTTON_STATE = 3
    FUNCTION_SET_SELECTED_LED_STATE = 5
    FUNCTION_SET_STATE_CHANGED_CALLBACK_CONFIGURATION = 6
    FUNCTION_GET_STATE_CHANGED_CALLBACK_CONFIGURATION = 7
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

    LED_STATE_AUTO_TOGGLE_ON = 0
    LED_STATE_AUTO_TOGGLE_OFF = 1
    LED_STATE_ON = 2
    LED_STATE_OFF = 3
    BUTTON_STATE_PRESSED = 0
    BUTTON_STATE_RELEASED = 1
    LED_LEFT = 0
    LED_RIGHT = 1
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
        Device.__init__(self, uid, ipcon, BrickletDualButtonV2.DEVICE_IDENTIFIER, BrickletDualButtonV2.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletDualButtonV2.FUNCTION_SET_LED_STATE] = BrickletDualButtonV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletDualButtonV2.FUNCTION_GET_LED_STATE] = BrickletDualButtonV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDualButtonV2.FUNCTION_GET_BUTTON_STATE] = BrickletDualButtonV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDualButtonV2.FUNCTION_SET_SELECTED_LED_STATE] = BrickletDualButtonV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletDualButtonV2.FUNCTION_SET_STATE_CHANGED_CALLBACK_CONFIGURATION] = BrickletDualButtonV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletDualButtonV2.FUNCTION_GET_STATE_CHANGED_CALLBACK_CONFIGURATION] = BrickletDualButtonV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDualButtonV2.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletDualButtonV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDualButtonV2.FUNCTION_SET_BOOTLOADER_MODE] = BrickletDualButtonV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDualButtonV2.FUNCTION_GET_BOOTLOADER_MODE] = BrickletDualButtonV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDualButtonV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletDualButtonV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletDualButtonV2.FUNCTION_WRITE_FIRMWARE] = BrickletDualButtonV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDualButtonV2.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletDualButtonV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletDualButtonV2.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletDualButtonV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDualButtonV2.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletDualButtonV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDualButtonV2.FUNCTION_RESET] = BrickletDualButtonV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletDualButtonV2.FUNCTION_WRITE_UID] = BrickletDualButtonV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletDualButtonV2.FUNCTION_READ_UID] = BrickletDualButtonV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDualButtonV2.FUNCTION_GET_IDENTITY] = BrickletDualButtonV2.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletDualButtonV2.CALLBACK_STATE_CHANGED] = (12, 'B B B B')

        ipcon.add_device(self)

    def set_led_state(self, led_l, led_r):
        r"""
        Sets the state of the LEDs. Possible states are:

        * 0 = AutoToggleOn: Enables auto toggle with initially enabled LED.
        * 1 = AutoToggleOff: Activates auto toggle with initially disabled LED.
        * 2 = On: Enables LED (auto toggle is disabled).
        * 3 = Off: Disables LED (auto toggle is disabled).

        In auto toggle mode the LED is toggled automatically at each press of a button.

        If you just want to set one of the LEDs and don't know the current state
        of the other LED, you can get the state with :func:`Get LED State` or you
        can use :func:`Set Selected LED State`.
        """
        self.check_validity()

        led_l = int(led_l)
        led_r = int(led_r)

        self.ipcon.send_request(self, BrickletDualButtonV2.FUNCTION_SET_LED_STATE, (led_l, led_r), 'B B', 0, '')

    def get_led_state(self):
        r"""
        Returns the current state of the LEDs, as set by :func:`Set LED State`.
        """
        self.check_validity()

        return GetLEDState(*self.ipcon.send_request(self, BrickletDualButtonV2.FUNCTION_GET_LED_STATE, (), '', 10, 'B B'))

    def get_button_state(self):
        r"""
        Returns the current state for both buttons. Possible states are:

        * 0 = pressed
        * 1 = released
        """
        self.check_validity()

        return GetButtonState(*self.ipcon.send_request(self, BrickletDualButtonV2.FUNCTION_GET_BUTTON_STATE, (), '', 10, 'B B'))

    def set_selected_led_state(self, led, state):
        r"""
        Sets the state of the selected LED.

        The other LED remains untouched.
        """
        self.check_validity()

        led = int(led)
        state = int(state)

        self.ipcon.send_request(self, BrickletDualButtonV2.FUNCTION_SET_SELECTED_LED_STATE, (led, state), 'B B', 0, '')

    def set_state_changed_callback_configuration(self, enabled):
        r"""
        If you enable this callback, the :cb:`State Changed` callback is triggered
        every time a button is pressed/released
        """
        self.check_validity()

        enabled = bool(enabled)

        self.ipcon.send_request(self, BrickletDualButtonV2.FUNCTION_SET_STATE_CHANGED_CALLBACK_CONFIGURATION, (enabled,), '!', 0, '')

    def get_state_changed_callback_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set State Changed Callback Configuration`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletDualButtonV2.FUNCTION_GET_STATE_CHANGED_CALLBACK_CONFIGURATION, (), '', 9, '!')

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletDualButtonV2.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletDualButtonV2.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletDualButtonV2.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletDualButtonV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletDualButtonV2.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletDualButtonV2.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletDualButtonV2.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletDualButtonV2.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletDualButtonV2.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletDualButtonV2.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletDualButtonV2.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletDualButtonV2.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

DualButtonV2 = BrickletDualButtonV2 # for backward compatibility
