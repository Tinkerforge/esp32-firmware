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

GetConfiguration = namedtuple('Configuration', ['voltage_range', 'current_range'])
GetOutLEDStatusConfig = namedtuple('OutLEDStatusConfig', ['min', 'max', 'config'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletIndustrialAnalogOutV2(Device):
    r"""
    Generates configurable DC voltage and current, 0V to 10V and 4mA to 20mA
    """

    DEVICE_IDENTIFIER = 2116
    DEVICE_DISPLAY_NAME = 'Industrial Analog Out Bricklet 2.0'
    DEVICE_URL_PART = 'industrial_analog_out_v2' # internal



    FUNCTION_SET_ENABLED = 1
    FUNCTION_GET_ENABLED = 2
    FUNCTION_SET_VOLTAGE = 3
    FUNCTION_GET_VOLTAGE = 4
    FUNCTION_SET_CURRENT = 5
    FUNCTION_GET_CURRENT = 6
    FUNCTION_SET_CONFIGURATION = 7
    FUNCTION_GET_CONFIGURATION = 8
    FUNCTION_SET_OUT_LED_CONFIG = 9
    FUNCTION_GET_OUT_LED_CONFIG = 10
    FUNCTION_SET_OUT_LED_STATUS_CONFIG = 11
    FUNCTION_GET_OUT_LED_STATUS_CONFIG = 12
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

    VOLTAGE_RANGE_0_TO_5V = 0
    VOLTAGE_RANGE_0_TO_10V = 1
    CURRENT_RANGE_4_TO_20MA = 0
    CURRENT_RANGE_0_TO_20MA = 1
    CURRENT_RANGE_0_TO_24MA = 2
    OUT_LED_CONFIG_OFF = 0
    OUT_LED_CONFIG_ON = 1
    OUT_LED_CONFIG_SHOW_HEARTBEAT = 2
    OUT_LED_CONFIG_SHOW_OUT_STATUS = 3
    OUT_LED_STATUS_CONFIG_THRESHOLD = 0
    OUT_LED_STATUS_CONFIG_INTENSITY = 1
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
        Device.__init__(self, uid, ipcon, BrickletIndustrialAnalogOutV2.DEVICE_IDENTIFIER, BrickletIndustrialAnalogOutV2.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletIndustrialAnalogOutV2.FUNCTION_SET_ENABLED] = BrickletIndustrialAnalogOutV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialAnalogOutV2.FUNCTION_GET_ENABLED] = BrickletIndustrialAnalogOutV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialAnalogOutV2.FUNCTION_SET_VOLTAGE] = BrickletIndustrialAnalogOutV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialAnalogOutV2.FUNCTION_GET_VOLTAGE] = BrickletIndustrialAnalogOutV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialAnalogOutV2.FUNCTION_SET_CURRENT] = BrickletIndustrialAnalogOutV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialAnalogOutV2.FUNCTION_GET_CURRENT] = BrickletIndustrialAnalogOutV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialAnalogOutV2.FUNCTION_SET_CONFIGURATION] = BrickletIndustrialAnalogOutV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialAnalogOutV2.FUNCTION_GET_CONFIGURATION] = BrickletIndustrialAnalogOutV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialAnalogOutV2.FUNCTION_SET_OUT_LED_CONFIG] = BrickletIndustrialAnalogOutV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialAnalogOutV2.FUNCTION_GET_OUT_LED_CONFIG] = BrickletIndustrialAnalogOutV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialAnalogOutV2.FUNCTION_SET_OUT_LED_STATUS_CONFIG] = BrickletIndustrialAnalogOutV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialAnalogOutV2.FUNCTION_GET_OUT_LED_STATUS_CONFIG] = BrickletIndustrialAnalogOutV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialAnalogOutV2.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletIndustrialAnalogOutV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialAnalogOutV2.FUNCTION_SET_BOOTLOADER_MODE] = BrickletIndustrialAnalogOutV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialAnalogOutV2.FUNCTION_GET_BOOTLOADER_MODE] = BrickletIndustrialAnalogOutV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialAnalogOutV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletIndustrialAnalogOutV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialAnalogOutV2.FUNCTION_WRITE_FIRMWARE] = BrickletIndustrialAnalogOutV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialAnalogOutV2.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletIndustrialAnalogOutV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialAnalogOutV2.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletIndustrialAnalogOutV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialAnalogOutV2.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletIndustrialAnalogOutV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialAnalogOutV2.FUNCTION_RESET] = BrickletIndustrialAnalogOutV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialAnalogOutV2.FUNCTION_WRITE_UID] = BrickletIndustrialAnalogOutV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialAnalogOutV2.FUNCTION_READ_UID] = BrickletIndustrialAnalogOutV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialAnalogOutV2.FUNCTION_GET_IDENTITY] = BrickletIndustrialAnalogOutV2.RESPONSE_EXPECTED_ALWAYS_TRUE


        ipcon.add_device(self)

    def set_enabled(self, enabled):
        r"""
        Enables/disables the output of voltage and current.
        """
        self.check_validity()

        enabled = bool(enabled)

        self.ipcon.send_request(self, BrickletIndustrialAnalogOutV2.FUNCTION_SET_ENABLED, (enabled,), '!', 0, '')

    def get_enabled(self):
        r"""
        Returns *true* if output of voltage and current is enabled, *false* otherwise.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialAnalogOutV2.FUNCTION_GET_ENABLED, (), '', 9, '!')

    def set_voltage(self, voltage):
        r"""
        Sets the output voltage.

        The output voltage and output current are linked. Changing the output voltage
        also changes the output current.
        """
        self.check_validity()

        voltage = int(voltage)

        self.ipcon.send_request(self, BrickletIndustrialAnalogOutV2.FUNCTION_SET_VOLTAGE, (voltage,), 'H', 0, '')

    def get_voltage(self):
        r"""
        Returns the voltage as set by :func:`Set Voltage`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialAnalogOutV2.FUNCTION_GET_VOLTAGE, (), '', 10, 'H')

    def set_current(self, current):
        r"""
        Sets the output current.

        The output current and output voltage are linked. Changing the output current
        also changes the output voltage.
        """
        self.check_validity()

        current = int(current)

        self.ipcon.send_request(self, BrickletIndustrialAnalogOutV2.FUNCTION_SET_CURRENT, (current,), 'H', 0, '')

    def get_current(self):
        r"""
        Returns the current as set by :func:`Set Current`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialAnalogOutV2.FUNCTION_GET_CURRENT, (), '', 10, 'H')

    def set_configuration(self, voltage_range, current_range):
        r"""
        Configures the voltage and current range.

        Possible voltage ranges are:

        * 0V to 5V
        * 0V to 10V

        Possible current ranges are:

        * 4mA to 20mA
        * 0mA to 20mA
        * 0mA to 24mA

        The resolution will always be 12 bit. This means, that the
        precision is higher with a smaller range.
        """
        self.check_validity()

        voltage_range = int(voltage_range)
        current_range = int(current_range)

        self.ipcon.send_request(self, BrickletIndustrialAnalogOutV2.FUNCTION_SET_CONFIGURATION, (voltage_range, current_range), 'B B', 0, '')

    def get_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Configuration`.
        """
        self.check_validity()

        return GetConfiguration(*self.ipcon.send_request(self, BrickletIndustrialAnalogOutV2.FUNCTION_GET_CONFIGURATION, (), '', 10, 'B B'))

    def set_out_led_config(self, config):
        r"""
        You can turn the Out LED off, on or show a
        heartbeat. You can also set the LED to "Out Status". In this mode the
        LED can either be turned on with a pre-defined threshold or the intensity
        of the LED can change with the output value (voltage or current).

        You can configure the channel status behavior with :func:`Set Out LED Status Config`.
        """
        self.check_validity()

        config = int(config)

        self.ipcon.send_request(self, BrickletIndustrialAnalogOutV2.FUNCTION_SET_OUT_LED_CONFIG, (config,), 'B', 0, '')

    def get_out_led_config(self):
        r"""
        Returns the Out LED configuration as set by :func:`Set Out LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialAnalogOutV2.FUNCTION_GET_OUT_LED_CONFIG, (), '', 9, 'B')

    def set_out_led_status_config(self, min, max, config):
        r"""
        Sets the Out LED status config. This config is used if the Out LED is
        configured as "Out Status", see :func:`Set Out LED Config`.

        For each channel you can choose between threshold and intensity mode.

        In threshold mode you can define a positive or a negative threshold.
        For a positive threshold set the "min" parameter to the threshold value in mV or
        µA above which the LED should turn on and set the "max" parameter to 0. Example:
        If you set a positive threshold of 5V, the LED will turn on as soon as the
        output value exceeds 5V and turn off again if it goes below 5V.
        For a negative threshold set the "max" parameter to the threshold value in mV or
        µA below which the LED should turn on and set the "min" parameter to 0. Example:
        If you set a negative threshold of 5V, the LED will turn on as soon as the
        output value goes below 5V and the LED will turn off when the output value
        exceeds 5V.

        In intensity mode you can define a range mV or µA that is used to scale the brightness
        of the LED. Example with min=2V, max=8V: The LED is off at 2V and below, on at
        8V and above and the brightness is linearly scaled between the values 2V and 8V.
        If the min value is greater than the max value, the LED brightness is scaled the
        other way around.
        """
        self.check_validity()

        min = int(min)
        max = int(max)
        config = int(config)

        self.ipcon.send_request(self, BrickletIndustrialAnalogOutV2.FUNCTION_SET_OUT_LED_STATUS_CONFIG, (min, max, config), 'H H B', 0, '')

    def get_out_led_status_config(self):
        r"""
        Returns the Out LED status configuration as set by :func:`Set Out LED Status Config`.
        """
        self.check_validity()

        return GetOutLEDStatusConfig(*self.ipcon.send_request(self, BrickletIndustrialAnalogOutV2.FUNCTION_GET_OUT_LED_STATUS_CONFIG, (), '', 13, 'H H B'))

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletIndustrialAnalogOutV2.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletIndustrialAnalogOutV2.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialAnalogOutV2.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletIndustrialAnalogOutV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletIndustrialAnalogOutV2.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletIndustrialAnalogOutV2.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialAnalogOutV2.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialAnalogOutV2.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletIndustrialAnalogOutV2.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletIndustrialAnalogOutV2.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialAnalogOutV2.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletIndustrialAnalogOutV2.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

IndustrialAnalogOutV2 = BrickletIndustrialAnalogOutV2 # for backward compatibility
