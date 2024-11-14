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

GetTouchStateCallbackConfiguration = namedtuple('TouchStateCallbackConfiguration', ['period', 'value_has_to_change'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletMultiTouchV2(Device):
    r"""
    Capacitive touch sensor for 12 electrodes
    """

    DEVICE_IDENTIFIER = 2129
    DEVICE_DISPLAY_NAME = 'Multi Touch Bricklet 2.0'
    DEVICE_URL_PART = 'multi_touch_v2' # internal

    CALLBACK_TOUCH_STATE = 4


    FUNCTION_GET_TOUCH_STATE = 1
    FUNCTION_SET_TOUCH_STATE_CALLBACK_CONFIGURATION = 2
    FUNCTION_GET_TOUCH_STATE_CALLBACK_CONFIGURATION = 3
    FUNCTION_RECALIBRATE = 5
    FUNCTION_SET_ELECTRODE_CONFIG = 6
    FUNCTION_GET_ELECTRODE_CONFIG = 7
    FUNCTION_SET_ELECTRODE_SENSITIVITY = 8
    FUNCTION_GET_ELECTRODE_SENSITIVITY = 9
    FUNCTION_SET_TOUCH_LED_CONFIG = 10
    FUNCTION_GET_TOUCH_LED_CONFIG = 11
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

    TOUCH_LED_CONFIG_OFF = 0
    TOUCH_LED_CONFIG_ON = 1
    TOUCH_LED_CONFIG_SHOW_HEARTBEAT = 2
    TOUCH_LED_CONFIG_SHOW_TOUCH = 3
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
        Device.__init__(self, uid, ipcon, BrickletMultiTouchV2.DEVICE_IDENTIFIER, BrickletMultiTouchV2.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletMultiTouchV2.FUNCTION_GET_TOUCH_STATE] = BrickletMultiTouchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMultiTouchV2.FUNCTION_SET_TOUCH_STATE_CALLBACK_CONFIGURATION] = BrickletMultiTouchV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletMultiTouchV2.FUNCTION_GET_TOUCH_STATE_CALLBACK_CONFIGURATION] = BrickletMultiTouchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMultiTouchV2.FUNCTION_RECALIBRATE] = BrickletMultiTouchV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletMultiTouchV2.FUNCTION_SET_ELECTRODE_CONFIG] = BrickletMultiTouchV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletMultiTouchV2.FUNCTION_GET_ELECTRODE_CONFIG] = BrickletMultiTouchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMultiTouchV2.FUNCTION_SET_ELECTRODE_SENSITIVITY] = BrickletMultiTouchV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletMultiTouchV2.FUNCTION_GET_ELECTRODE_SENSITIVITY] = BrickletMultiTouchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMultiTouchV2.FUNCTION_SET_TOUCH_LED_CONFIG] = BrickletMultiTouchV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletMultiTouchV2.FUNCTION_GET_TOUCH_LED_CONFIG] = BrickletMultiTouchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMultiTouchV2.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletMultiTouchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMultiTouchV2.FUNCTION_SET_BOOTLOADER_MODE] = BrickletMultiTouchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMultiTouchV2.FUNCTION_GET_BOOTLOADER_MODE] = BrickletMultiTouchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMultiTouchV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletMultiTouchV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletMultiTouchV2.FUNCTION_WRITE_FIRMWARE] = BrickletMultiTouchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMultiTouchV2.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletMultiTouchV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletMultiTouchV2.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletMultiTouchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMultiTouchV2.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletMultiTouchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMultiTouchV2.FUNCTION_RESET] = BrickletMultiTouchV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletMultiTouchV2.FUNCTION_WRITE_UID] = BrickletMultiTouchV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletMultiTouchV2.FUNCTION_READ_UID] = BrickletMultiTouchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMultiTouchV2.FUNCTION_GET_IDENTITY] = BrickletMultiTouchV2.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletMultiTouchV2.CALLBACK_TOUCH_STATE] = (10, '13!')

        ipcon.add_device(self)

    def get_touch_state(self):
        r"""
        Returns the current touch state. The state is given as a array of
        bools.

        Element 0 to 11 represent the 12 electrodes and element 12 represents
        the proximity.

        If an electrode is touched, the corresponding element is *true*. If
        a hand or similar is in proximity to the electrodes, element 12 is
        *true*.

        The proximity is activated with a distance of 1-2cm. An electrode
        is already counted as touched if a finger is nearly touching the
        electrode. This means that you can put a piece of paper or foil
        or similar on top of a electrode to build a touch panel with
        a professional look.

        If you want to get the value periodically, it is recommended to use the
        :cb:`Touch State` callback. You can set the callback configuration
        with :func:`Set Touch State Callback Configuration`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMultiTouchV2.FUNCTION_GET_TOUCH_STATE, (), '', 10, '13!')

    def set_touch_state_callback_configuration(self, period, value_has_to_change):
        r"""
        The period is the period with which the :cb:`Touch State` callback
        is triggered periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change within the
        period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.
        """
        self.check_validity()

        period = int(period)
        value_has_to_change = bool(value_has_to_change)

        self.ipcon.send_request(self, BrickletMultiTouchV2.FUNCTION_SET_TOUCH_STATE_CALLBACK_CONFIGURATION, (period, value_has_to_change), 'I !', 0, '')

    def get_touch_state_callback_configuration(self):
        r"""
        Returns the callback configuration as set by
        :func:`Set Touch State Callback Configuration`.
        """
        self.check_validity()

        return GetTouchStateCallbackConfiguration(*self.ipcon.send_request(self, BrickletMultiTouchV2.FUNCTION_GET_TOUCH_STATE_CALLBACK_CONFIGURATION, (), '', 13, 'I !'))

    def recalibrate(self):
        r"""
        Recalibrates the electrodes. Call this function whenever you changed
        or moved you electrodes.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletMultiTouchV2.FUNCTION_RECALIBRATE, (), '', 0, '')

    def set_electrode_config(self, enabled_electrodes):
        r"""
        Enables/disables electrodes with a bool array (see :func:`Get Touch State`).

        *True* enables the electrode, *false* disables the electrode. A
        disabled electrode will always return *false* as its state. If you
        don't need all electrodes you can disable the electrodes that are
        not needed.

        It is recommended that you disable the proximity electrode (element 12) if
        the proximity feature is not needed. This will reduce the amount of
        traffic that is produced by the :cb:`Touch State` callback.

        Disabling electrodes will also reduce power consumption.
        """
        self.check_validity()

        enabled_electrodes = list(map(bool, enabled_electrodes))

        self.ipcon.send_request(self, BrickletMultiTouchV2.FUNCTION_SET_ELECTRODE_CONFIG, (enabled_electrodes,), '13!', 0, '')

    def get_electrode_config(self):
        r"""
        Returns the electrode configuration, as set by :func:`Set Electrode Config`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMultiTouchV2.FUNCTION_GET_ELECTRODE_CONFIG, (), '', 10, '13!')

    def set_electrode_sensitivity(self, sensitivity):
        r"""
        Sets the sensitivity of the electrodes. An electrode with a high sensitivity
        will register a touch earlier then an electrode with a low sensitivity.

        If you build a big electrode you might need to decrease the sensitivity, since
        the area that can be charged will get bigger. If you want to be able to
        activate an electrode from further away you need to increase the sensitivity.

        After a new sensitivity is set, you likely want to call :func:`Recalibrate`
        to calibrate the electrodes with the newly defined sensitivity.
        """
        self.check_validity()

        sensitivity = int(sensitivity)

        self.ipcon.send_request(self, BrickletMultiTouchV2.FUNCTION_SET_ELECTRODE_SENSITIVITY, (sensitivity,), 'B', 0, '')

    def get_electrode_sensitivity(self):
        r"""
        Returns the current sensitivity, as set by :func:`Set Electrode Sensitivity`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMultiTouchV2.FUNCTION_GET_ELECTRODE_SENSITIVITY, (), '', 9, 'B')

    def set_touch_led_config(self, config):
        r"""
        Configures the touch LED to be either turned off, turned on, blink in
        heartbeat mode or show the touch state (electrode touched = LED on).
        """
        self.check_validity()

        config = int(config)

        self.ipcon.send_request(self, BrickletMultiTouchV2.FUNCTION_SET_TOUCH_LED_CONFIG, (config,), 'B', 0, '')

    def get_touch_led_config(self):
        r"""
        Returns the LED configuration as set by :func:`Set Touch LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMultiTouchV2.FUNCTION_GET_TOUCH_LED_CONFIG, (), '', 9, 'B')

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletMultiTouchV2.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletMultiTouchV2.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMultiTouchV2.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletMultiTouchV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletMultiTouchV2.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletMultiTouchV2.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMultiTouchV2.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMultiTouchV2.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletMultiTouchV2.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletMultiTouchV2.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMultiTouchV2.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletMultiTouchV2.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

MultiTouchV2 = BrickletMultiTouchV2 # for backward compatibility
