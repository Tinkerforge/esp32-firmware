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

GetSleepMode = namedtuple('SleepMode', ['power_off_delay', 'power_off_duration', 'raspberry_pi_off', 'bricklets_off', 'enable_sleep_indicator'])
GetVoltages = namedtuple('Voltages', ['voltage_usb', 'voltage_dc'])
GetVoltagesCallbackConfiguration = namedtuple('VoltagesCallbackConfiguration', ['period', 'value_has_to_change'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickHAT(Device):
    r"""
    HAT for Raspberry Pi with 8 Bricklets ports and real-time clock
    """

    DEVICE_IDENTIFIER = 111
    DEVICE_DISPLAY_NAME = 'HAT Brick'
    DEVICE_URL_PART = 'hat' # internal

    CALLBACK_VOLTAGES = 8


    FUNCTION_SET_SLEEP_MODE = 1
    FUNCTION_GET_SLEEP_MODE = 2
    FUNCTION_SET_BRICKLET_POWER = 3
    FUNCTION_GET_BRICKLET_POWER = 4
    FUNCTION_GET_VOLTAGES = 5
    FUNCTION_SET_VOLTAGES_CALLBACK_CONFIGURATION = 6
    FUNCTION_GET_VOLTAGES_CALLBACK_CONFIGURATION = 7
    FUNCTION_SET_RTC_DRIVER = 9
    FUNCTION_GET_RTC_DRIVER = 10
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

    RTC_DRIVER_PCF8523 = 0
    RTC_DRIVER_DS1338 = 1
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
        Device.__init__(self, uid, ipcon, BrickHAT.DEVICE_IDENTIFIER, BrickHAT.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 2)

        self.response_expected[BrickHAT.FUNCTION_SET_SLEEP_MODE] = BrickHAT.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickHAT.FUNCTION_GET_SLEEP_MODE] = BrickHAT.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickHAT.FUNCTION_SET_BRICKLET_POWER] = BrickHAT.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickHAT.FUNCTION_GET_BRICKLET_POWER] = BrickHAT.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickHAT.FUNCTION_GET_VOLTAGES] = BrickHAT.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickHAT.FUNCTION_SET_VOLTAGES_CALLBACK_CONFIGURATION] = BrickHAT.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickHAT.FUNCTION_GET_VOLTAGES_CALLBACK_CONFIGURATION] = BrickHAT.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickHAT.FUNCTION_SET_RTC_DRIVER] = BrickHAT.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickHAT.FUNCTION_GET_RTC_DRIVER] = BrickHAT.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickHAT.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickHAT.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickHAT.FUNCTION_SET_BOOTLOADER_MODE] = BrickHAT.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickHAT.FUNCTION_GET_BOOTLOADER_MODE] = BrickHAT.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickHAT.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickHAT.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickHAT.FUNCTION_WRITE_FIRMWARE] = BrickHAT.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickHAT.FUNCTION_SET_STATUS_LED_CONFIG] = BrickHAT.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickHAT.FUNCTION_GET_STATUS_LED_CONFIG] = BrickHAT.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickHAT.FUNCTION_GET_CHIP_TEMPERATURE] = BrickHAT.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickHAT.FUNCTION_RESET] = BrickHAT.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickHAT.FUNCTION_WRITE_UID] = BrickHAT.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickHAT.FUNCTION_READ_UID] = BrickHAT.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickHAT.FUNCTION_GET_IDENTITY] = BrickHAT.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickHAT.CALLBACK_VOLTAGES] = (12, 'H H')

        ipcon.add_device(self)

    def set_sleep_mode(self, power_off_delay, power_off_duration, raspberry_pi_off, bricklets_off, enable_sleep_indicator):
        r"""
        Sets the sleep mode.

        .. note::
         Calling this function will cut the Raspberry Pi's power after Power Off Delay seconds.
         You have to shut down the operating system yourself, e.g. by calling 'sudo shutdown -h now'.

        Parameters:

        * Power Off Delay: Time before the RPi/Bricklets are powered off.
        * Power Off Duration: Duration that the RPi/Bricklets stay powered off.
        * Raspberry Pi Off: RPi is powered off if set to true.
        * Bricklets Off: Bricklets are powered off if set to true.
        * Enable Sleep Indicator: If set to true, the status LED will blink in a 1s interval
          during the whole power off duration. This will draw additional 0.3mA.

        Example: To turn RPi and Bricklets off in 5 seconds for 10 minutes with sleep
        indicator enabled, call (5, 60*10, true, true, true).

        This function can also be used to implement a watchdog. To do this you can
        write a program that calls this function once per second in a loop with
        (10, 2, true, false, false). If the RPi crashes or gets stuck
        the HAT will reset the RPi after 10 seconds.
        """
        self.check_validity()

        power_off_delay = int(power_off_delay)
        power_off_duration = int(power_off_duration)
        raspberry_pi_off = bool(raspberry_pi_off)
        bricklets_off = bool(bricklets_off)
        enable_sleep_indicator = bool(enable_sleep_indicator)

        self.ipcon.send_request(self, BrickHAT.FUNCTION_SET_SLEEP_MODE, (power_off_delay, power_off_duration, raspberry_pi_off, bricklets_off, enable_sleep_indicator), 'I I ! ! !', 0, '')

    def get_sleep_mode(self):
        r"""
        Returns the sleep mode settings as set by :func:`Set Sleep Mode`.
        """
        self.check_validity()

        return GetSleepMode(*self.ipcon.send_request(self, BrickHAT.FUNCTION_GET_SLEEP_MODE, (), '', 19, 'I I ! ! !'))

    def set_bricklet_power(self, bricklet_power):
        r"""
        Set to true/false to turn the power supply of the connected Bricklets on/off.
        """
        self.check_validity()

        bricklet_power = bool(bricklet_power)

        self.ipcon.send_request(self, BrickHAT.FUNCTION_SET_BRICKLET_POWER, (bricklet_power,), '!', 0, '')

    def get_bricklet_power(self):
        r"""
        Returns the power status of the connected Bricklets as set by :func:`Set Bricklet Power`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickHAT.FUNCTION_GET_BRICKLET_POWER, (), '', 9, '!')

    def get_voltages(self):
        r"""
        Returns the USB supply voltage and the DC input supply voltage.

        There are three possible combinations:

        * Only USB connected: The USB supply voltage will be fed back to the
          DC input connector. You will read the USB voltage and a slightly lower
          voltage on the DC input.
        * Only DC input connected: The DC voltage will not be fed back to the
          USB connector. You will read the DC input voltage and the USB voltage
          will be 0.
        * USB and DC input connected: You will read both voltages. In this case
          the USB supply will be without load, but it will work as backup if you
          disconnect the DC input (or if the DC input voltage falls below the
          USB voltage).
        """
        self.check_validity()

        return GetVoltages(*self.ipcon.send_request(self, BrickHAT.FUNCTION_GET_VOLTAGES, (), '', 12, 'H H'))

    def set_voltages_callback_configuration(self, period, value_has_to_change):
        r"""
        The period is the period with which the :cb:`Voltages`
        callback is triggered periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change within the
        period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.

        .. versionadded:: 2.0.1$nbsp;(Firmware)
        """
        self.check_validity()

        period = int(period)
        value_has_to_change = bool(value_has_to_change)

        self.ipcon.send_request(self, BrickHAT.FUNCTION_SET_VOLTAGES_CALLBACK_CONFIGURATION, (period, value_has_to_change), 'I !', 0, '')

    def get_voltages_callback_configuration(self):
        r"""
        Returns the callback configuration as set by
        :func:`Set Voltages Callback Configuration`.

        .. versionadded:: 2.0.1$nbsp;(Firmware)
        """
        self.check_validity()

        return GetVoltagesCallbackConfiguration(*self.ipcon.send_request(self, BrickHAT.FUNCTION_GET_VOLTAGES_CALLBACK_CONFIGURATION, (), '', 13, 'I !'))

    def set_rtc_driver(self, rtc_driver):
        r"""
        Configures the RTC driver that is given to the Raspberry Pi to be used.
        Currently there are two different RTCs used:

        * Hardware version <= 1.5: PCF8523
        * Hardware version 1.6: DS1338

        The correct driver will be set during factory flashing by Tinkerforge.

        .. versionadded:: 2.0.3$nbsp;(Firmware)
        """
        self.check_validity()

        rtc_driver = int(rtc_driver)

        self.ipcon.send_request(self, BrickHAT.FUNCTION_SET_RTC_DRIVER, (rtc_driver,), 'B', 0, '')

    def get_rtc_driver(self):
        r"""
        Returns the RTC driver as set by :func:`Set RTC Driver`.

        .. versionadded:: 2.0.3$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickHAT.FUNCTION_GET_RTC_DRIVER, (), '', 9, 'B')

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickHAT.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickHAT.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickHAT.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickHAT.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickHAT.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickHAT.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickHAT.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickHAT.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickHAT.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickHAT.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickHAT.FUNCTION_READ_UID, (), '', 12, 'I')

    def get_identity(self):
        r"""
        Returns the UID, the UID where the HAT is connected to
        (typically '0' as the HAT is the root device in the topology),
        the position, the hardware and firmware version as well as the
        device identifier.

        The HAT (Zero) Brick is always at position 'i'.

        The device identifier numbers can be found :ref:`here <device_identifier>`.
        |device_identifier_constant|
        """
        return GetIdentity(*self.ipcon.send_request(self, BrickHAT.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

HAT = BrickHAT # for backward compatibility
