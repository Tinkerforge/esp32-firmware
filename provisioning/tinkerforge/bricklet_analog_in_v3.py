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

GetVoltageCallbackConfiguration = namedtuple('VoltageCallbackConfiguration', ['period', 'value_has_to_change', 'option', 'min', 'max'])
GetCalibration = namedtuple('Calibration', ['offset', 'multiplier', 'divisor'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletAnalogInV3(Device):
    r"""
    Measures DC voltage between 0V and 42V
    """

    DEVICE_IDENTIFIER = 295
    DEVICE_DISPLAY_NAME = 'Analog In Bricklet 3.0'
    DEVICE_URL_PART = 'analog_in_v3' # internal

    CALLBACK_VOLTAGE = 4


    FUNCTION_GET_VOLTAGE = 1
    FUNCTION_SET_VOLTAGE_CALLBACK_CONFIGURATION = 2
    FUNCTION_GET_VOLTAGE_CALLBACK_CONFIGURATION = 3
    FUNCTION_SET_OVERSAMPLING = 5
    FUNCTION_GET_OVERSAMPLING = 6
    FUNCTION_SET_CALIBRATION = 7
    FUNCTION_GET_CALIBRATION = 8
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
    OVERSAMPLING_32 = 0
    OVERSAMPLING_64 = 1
    OVERSAMPLING_128 = 2
    OVERSAMPLING_256 = 3
    OVERSAMPLING_512 = 4
    OVERSAMPLING_1024 = 5
    OVERSAMPLING_2048 = 6
    OVERSAMPLING_4096 = 7
    OVERSAMPLING_8192 = 8
    OVERSAMPLING_16384 = 9
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
        Device.__init__(self, uid, ipcon, BrickletAnalogInV3.DEVICE_IDENTIFIER, BrickletAnalogInV3.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletAnalogInV3.FUNCTION_GET_VOLTAGE] = BrickletAnalogInV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogInV3.FUNCTION_SET_VOLTAGE_CALLBACK_CONFIGURATION] = BrickletAnalogInV3.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletAnalogInV3.FUNCTION_GET_VOLTAGE_CALLBACK_CONFIGURATION] = BrickletAnalogInV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogInV3.FUNCTION_SET_OVERSAMPLING] = BrickletAnalogInV3.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAnalogInV3.FUNCTION_GET_OVERSAMPLING] = BrickletAnalogInV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogInV3.FUNCTION_SET_CALIBRATION] = BrickletAnalogInV3.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAnalogInV3.FUNCTION_GET_CALIBRATION] = BrickletAnalogInV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogInV3.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletAnalogInV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogInV3.FUNCTION_SET_BOOTLOADER_MODE] = BrickletAnalogInV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogInV3.FUNCTION_GET_BOOTLOADER_MODE] = BrickletAnalogInV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogInV3.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletAnalogInV3.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAnalogInV3.FUNCTION_WRITE_FIRMWARE] = BrickletAnalogInV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogInV3.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletAnalogInV3.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAnalogInV3.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletAnalogInV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogInV3.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletAnalogInV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogInV3.FUNCTION_RESET] = BrickletAnalogInV3.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAnalogInV3.FUNCTION_WRITE_UID] = BrickletAnalogInV3.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAnalogInV3.FUNCTION_READ_UID] = BrickletAnalogInV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogInV3.FUNCTION_GET_IDENTITY] = BrickletAnalogInV3.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletAnalogInV3.CALLBACK_VOLTAGE] = (10, 'H')

        ipcon.add_device(self)

    def get_voltage(self):
        r"""
        Returns the measured voltage. The resolution is approximately 10mV to 1mV
        depending on the oversampling configuration (:func:`Set Oversampling`).


        If you want to get the value periodically, it is recommended to use the
        :cb:`Voltage` callback. You can set the callback configuration
        with :func:`Set Voltage Callback Configuration`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAnalogInV3.FUNCTION_GET_VOLTAGE, (), '', 10, 'H')

    def set_voltage_callback_configuration(self, period, value_has_to_change, option, min, max):
        r"""
        The period is the period with which the :cb:`Voltage` callback is triggered
        periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change
        within the period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.

        It is furthermore possible to constrain the callback with thresholds.

        The `option`-parameter together with min/max sets a threshold for the :cb:`Voltage` callback.

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

        self.ipcon.send_request(self, BrickletAnalogInV3.FUNCTION_SET_VOLTAGE_CALLBACK_CONFIGURATION, (period, value_has_to_change, option, min, max), 'I ! c H H', 0, '')

    def get_voltage_callback_configuration(self):
        r"""
        Returns the callback configuration as set by :func:`Set Voltage Callback Configuration`.
        """
        self.check_validity()

        return GetVoltageCallbackConfiguration(*self.ipcon.send_request(self, BrickletAnalogInV3.FUNCTION_GET_VOLTAGE_CALLBACK_CONFIGURATION, (), '', 18, 'I ! c H H'))

    def set_oversampling(self, oversampling):
        r"""
        Sets the oversampling between 32x and 16384x. The Bricklet
        takes one 12bit sample every 17.5µs. Thus an oversampling
        of 32x is equivalent to an integration time of 0.56ms and
        a oversampling of 16384x is equivalent to an integration
        time of 286ms.

        The oversampling uses the moving average principle. A
        new value is always calculated once per millisecond.

        With increased oversampling the noise decreases. With decreased
        oversampling the reaction time increases (changes in voltage will be
        measured faster).
        """
        self.check_validity()

        oversampling = int(oversampling)

        self.ipcon.send_request(self, BrickletAnalogInV3.FUNCTION_SET_OVERSAMPLING, (oversampling,), 'B', 0, '')

    def get_oversampling(self):
        r"""
        Returns the oversampling value as set by :func:`Set Oversampling`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAnalogInV3.FUNCTION_GET_OVERSAMPLING, (), '', 9, 'B')

    def set_calibration(self, offset, multiplier, divisor):
        r"""
        Sets a calibration for the measured voltage value.
        The formula for the calibration is as follows::

         Calibrated Value = (Value + Offset) * Multiplier / Divisor

        We recommend that you use the Brick Viewer to calibrate
        the Bricklet. The calibration will be saved internally and only
        has to be done once.
        """
        self.check_validity()

        offset = int(offset)
        multiplier = int(multiplier)
        divisor = int(divisor)

        self.ipcon.send_request(self, BrickletAnalogInV3.FUNCTION_SET_CALIBRATION, (offset, multiplier, divisor), 'h H H', 0, '')

    def get_calibration(self):
        r"""
        Returns the calibration as set by :func:`Set Calibration`.
        """
        self.check_validity()

        return GetCalibration(*self.ipcon.send_request(self, BrickletAnalogInV3.FUNCTION_GET_CALIBRATION, (), '', 14, 'h H H'))

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletAnalogInV3.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletAnalogInV3.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAnalogInV3.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletAnalogInV3.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletAnalogInV3.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletAnalogInV3.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAnalogInV3.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAnalogInV3.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletAnalogInV3.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletAnalogInV3.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAnalogInV3.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletAnalogInV3.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

AnalogInV3 = BrickletAnalogInV3 # for backward compatibility
