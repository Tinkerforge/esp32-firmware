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

GetPositionCallbackConfiguration = namedtuple('PositionCallbackConfiguration', ['period', 'value_has_to_change', 'option', 'min', 'max'])
GetMotorPosition = namedtuple('MotorPosition', ['position', 'drive_mode', 'hold_position', 'position_reached'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletMotorizedLinearPoti(Device):
    r"""
    Motorized Linear Potentiometer
    """

    DEVICE_IDENTIFIER = 267
    DEVICE_DISPLAY_NAME = 'Motorized Linear Poti Bricklet'
    DEVICE_URL_PART = 'motorized_linear_poti' # internal

    CALLBACK_POSITION = 4
    CALLBACK_POSITION_REACHED = 10


    FUNCTION_GET_POSITION = 1
    FUNCTION_SET_POSITION_CALLBACK_CONFIGURATION = 2
    FUNCTION_GET_POSITION_CALLBACK_CONFIGURATION = 3
    FUNCTION_SET_MOTOR_POSITION = 5
    FUNCTION_GET_MOTOR_POSITION = 6
    FUNCTION_CALIBRATE = 7
    FUNCTION_SET_POSITION_REACHED_CALLBACK_CONFIGURATION = 8
    FUNCTION_GET_POSITION_REACHED_CALLBACK_CONFIGURATION = 9
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

    THRESHOLD_OPTION_OFF = 'x'
    THRESHOLD_OPTION_OUTSIDE = 'o'
    THRESHOLD_OPTION_INSIDE = 'i'
    THRESHOLD_OPTION_SMALLER = '<'
    THRESHOLD_OPTION_GREATER = '>'
    DRIVE_MODE_FAST = 0
    DRIVE_MODE_SMOOTH = 1
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
        Device.__init__(self, uid, ipcon, BrickletMotorizedLinearPoti.DEVICE_IDENTIFIER, BrickletMotorizedLinearPoti.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletMotorizedLinearPoti.FUNCTION_GET_POSITION] = BrickletMotorizedLinearPoti.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMotorizedLinearPoti.FUNCTION_SET_POSITION_CALLBACK_CONFIGURATION] = BrickletMotorizedLinearPoti.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletMotorizedLinearPoti.FUNCTION_GET_POSITION_CALLBACK_CONFIGURATION] = BrickletMotorizedLinearPoti.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMotorizedLinearPoti.FUNCTION_SET_MOTOR_POSITION] = BrickletMotorizedLinearPoti.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletMotorizedLinearPoti.FUNCTION_GET_MOTOR_POSITION] = BrickletMotorizedLinearPoti.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMotorizedLinearPoti.FUNCTION_CALIBRATE] = BrickletMotorizedLinearPoti.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletMotorizedLinearPoti.FUNCTION_SET_POSITION_REACHED_CALLBACK_CONFIGURATION] = BrickletMotorizedLinearPoti.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletMotorizedLinearPoti.FUNCTION_GET_POSITION_REACHED_CALLBACK_CONFIGURATION] = BrickletMotorizedLinearPoti.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMotorizedLinearPoti.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletMotorizedLinearPoti.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMotorizedLinearPoti.FUNCTION_SET_BOOTLOADER_MODE] = BrickletMotorizedLinearPoti.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMotorizedLinearPoti.FUNCTION_GET_BOOTLOADER_MODE] = BrickletMotorizedLinearPoti.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMotorizedLinearPoti.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletMotorizedLinearPoti.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletMotorizedLinearPoti.FUNCTION_WRITE_FIRMWARE] = BrickletMotorizedLinearPoti.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMotorizedLinearPoti.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletMotorizedLinearPoti.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletMotorizedLinearPoti.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletMotorizedLinearPoti.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMotorizedLinearPoti.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletMotorizedLinearPoti.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMotorizedLinearPoti.FUNCTION_RESET] = BrickletMotorizedLinearPoti.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletMotorizedLinearPoti.FUNCTION_WRITE_UID] = BrickletMotorizedLinearPoti.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletMotorizedLinearPoti.FUNCTION_READ_UID] = BrickletMotorizedLinearPoti.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMotorizedLinearPoti.FUNCTION_GET_IDENTITY] = BrickletMotorizedLinearPoti.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletMotorizedLinearPoti.CALLBACK_POSITION] = (10, 'H')
        self.callback_formats[BrickletMotorizedLinearPoti.CALLBACK_POSITION_REACHED] = (10, 'H')

        ipcon.add_device(self)

    def get_position(self):
        r"""
        Returns the position of the linear potentiometer. The value is
        between 0 (slider down) and 100 (slider up).


        If you want to get the value periodically, it is recommended to use the
        :cb:`Position` callback. You can set the callback configuration
        with :func:`Set Position Callback Configuration`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMotorizedLinearPoti.FUNCTION_GET_POSITION, (), '', 10, 'H')

    def set_position_callback_configuration(self, period, value_has_to_change, option, min, max):
        r"""
        The period is the period with which the :cb:`Position` callback is triggered
        periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change
        within the period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.

        It is furthermore possible to constrain the callback with thresholds.

        The `option`-parameter together with min/max sets a threshold for the :cb:`Position` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Threshold is turned off"
         "'o'",    "Threshold is triggered when the value is *outside* the min and max values"
         "'i'",    "Threshold is triggered when the value is *inside* or equal to the min and max values"
         "'<'",    "Threshold is triggered when the value is smaller than the min value (max is ignored)"
         "'>'",    "Threshold is triggered when the value is greater than the min value (max is ignored)"

        If the option is set to 'x' (threshold turned off) the callback is triggered with the fixed period.
        """
        self.check_validity()

        period = int(period)
        value_has_to_change = bool(value_has_to_change)
        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickletMotorizedLinearPoti.FUNCTION_SET_POSITION_CALLBACK_CONFIGURATION, (period, value_has_to_change, option, min, max), 'I ! c H H', 0, '')

    def get_position_callback_configuration(self):
        r"""
        Returns the callback configuration as set by :func:`Set Position Callback Configuration`.
        """
        self.check_validity()

        return GetPositionCallbackConfiguration(*self.ipcon.send_request(self, BrickletMotorizedLinearPoti.FUNCTION_GET_POSITION_CALLBACK_CONFIGURATION, (), '', 18, 'I ! c H H'))

    def set_motor_position(self, position, drive_mode, hold_position):
        r"""
        Sets the position of the potentiometer. The motorized potentiometer will
        immediately start to approach the position. Depending on the chosen drive mode,
        the position will either be reached as fast as possible or in a slow but smooth
        motion.

        The position has to be between 0 (slider down) and 100 (slider up).

        If you set the hold position parameter to true, the position will automatically
        be retained. If a user changes the position of the potentiometer, it will
        automatically drive back to the original set point.

        If the hold position parameter is set to false, the potentiometer can be changed
        again by the user as soon as the set point was reached once.
        """
        self.check_validity()

        position = int(position)
        drive_mode = int(drive_mode)
        hold_position = bool(hold_position)

        self.ipcon.send_request(self, BrickletMotorizedLinearPoti.FUNCTION_SET_MOTOR_POSITION, (position, drive_mode, hold_position), 'H B !', 0, '')

    def get_motor_position(self):
        r"""
        Returns the last motor position as set by :func:`Set Motor Position`. This is not
        the current position (use :func:`Get Position` to get the current position). This
        is the last used set point and configuration.

        The position reached parameter is true if the position has been reached at one point.
        The position may have been changed again in the meantime by the user.
        """
        self.check_validity()

        return GetMotorPosition(*self.ipcon.send_request(self, BrickletMotorizedLinearPoti.FUNCTION_GET_MOTOR_POSITION, (), '', 13, 'H B ! !'))

    def calibrate(self):
        r"""
        Starts a calibration procedure. The potentiometer will be driven to the extreme
        points to calibrate the potentiometer.

        The calibration is saved in flash, it does not have to be called on every start up.

        The Motorized Linear Poti Bricklet is already factory-calibrated during
        testing at Tinkerforge.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletMotorizedLinearPoti.FUNCTION_CALIBRATE, (), '', 0, '')

    def set_position_reached_callback_configuration(self, enabled):
        r"""
        Enables/Disables :cb:`Position Reached` callback.
        """
        self.check_validity()

        enabled = bool(enabled)

        self.ipcon.send_request(self, BrickletMotorizedLinearPoti.FUNCTION_SET_POSITION_REACHED_CALLBACK_CONFIGURATION, (enabled,), '!', 0, '')

    def get_position_reached_callback_configuration(self):
        r"""
        Returns the :cb:`Position Reached` callback configuration
        as set by :func:`Set Position Reached Callback Configuration`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMotorizedLinearPoti.FUNCTION_GET_POSITION_REACHED_CALLBACK_CONFIGURATION, (), '', 9, '!')

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletMotorizedLinearPoti.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletMotorizedLinearPoti.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMotorizedLinearPoti.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletMotorizedLinearPoti.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletMotorizedLinearPoti.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletMotorizedLinearPoti.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMotorizedLinearPoti.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMotorizedLinearPoti.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletMotorizedLinearPoti.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletMotorizedLinearPoti.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMotorizedLinearPoti.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletMotorizedLinearPoti.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

MotorizedLinearPoti = BrickletMotorizedLinearPoti # for backward compatibility
