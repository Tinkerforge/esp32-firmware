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

GetDateTime = namedtuple('DateTime', ['year', 'month', 'day', 'hour', 'minute', 'second', 'centisecond', 'weekday', 'timestamp'])
GetAlarm = namedtuple('Alarm', ['month', 'day', 'hour', 'minute', 'second', 'weekday', 'interval'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletRealTimeClockV2(Device):
    r"""
    Battery-backed real-time clock
    """

    DEVICE_IDENTIFIER = 2106
    DEVICE_DISPLAY_NAME = 'Real-Time Clock Bricklet 2.0'
    DEVICE_URL_PART = 'real_time_clock_v2' # internal

    CALLBACK_DATE_TIME = 10
    CALLBACK_ALARM = 11


    FUNCTION_SET_DATE_TIME = 1
    FUNCTION_GET_DATE_TIME = 2
    FUNCTION_GET_TIMESTAMP = 3
    FUNCTION_SET_OFFSET = 4
    FUNCTION_GET_OFFSET = 5
    FUNCTION_SET_DATE_TIME_CALLBACK_CONFIGURATION = 6
    FUNCTION_GET_DATE_TIME_CALLBACK_CONFIGURATION = 7
    FUNCTION_SET_ALARM = 8
    FUNCTION_GET_ALARM = 9
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

    WEEKDAY_MONDAY = 1
    WEEKDAY_TUESDAY = 2
    WEEKDAY_WEDNESDAY = 3
    WEEKDAY_THURSDAY = 4
    WEEKDAY_FRIDAY = 5
    WEEKDAY_SATURDAY = 6
    WEEKDAY_SUNDAY = 7
    ALARM_MATCH_DISABLED = -1
    ALARM_INTERVAL_DISABLED = -1
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
        Device.__init__(self, uid, ipcon, BrickletRealTimeClockV2.DEVICE_IDENTIFIER, BrickletRealTimeClockV2.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletRealTimeClockV2.FUNCTION_SET_DATE_TIME] = BrickletRealTimeClockV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRealTimeClockV2.FUNCTION_GET_DATE_TIME] = BrickletRealTimeClockV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRealTimeClockV2.FUNCTION_GET_TIMESTAMP] = BrickletRealTimeClockV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRealTimeClockV2.FUNCTION_SET_OFFSET] = BrickletRealTimeClockV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRealTimeClockV2.FUNCTION_GET_OFFSET] = BrickletRealTimeClockV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRealTimeClockV2.FUNCTION_SET_DATE_TIME_CALLBACK_CONFIGURATION] = BrickletRealTimeClockV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletRealTimeClockV2.FUNCTION_GET_DATE_TIME_CALLBACK_CONFIGURATION] = BrickletRealTimeClockV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRealTimeClockV2.FUNCTION_SET_ALARM] = BrickletRealTimeClockV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletRealTimeClockV2.FUNCTION_GET_ALARM] = BrickletRealTimeClockV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRealTimeClockV2.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletRealTimeClockV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRealTimeClockV2.FUNCTION_SET_BOOTLOADER_MODE] = BrickletRealTimeClockV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRealTimeClockV2.FUNCTION_GET_BOOTLOADER_MODE] = BrickletRealTimeClockV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRealTimeClockV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletRealTimeClockV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRealTimeClockV2.FUNCTION_WRITE_FIRMWARE] = BrickletRealTimeClockV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRealTimeClockV2.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletRealTimeClockV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRealTimeClockV2.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletRealTimeClockV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRealTimeClockV2.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletRealTimeClockV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRealTimeClockV2.FUNCTION_RESET] = BrickletRealTimeClockV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRealTimeClockV2.FUNCTION_WRITE_UID] = BrickletRealTimeClockV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRealTimeClockV2.FUNCTION_READ_UID] = BrickletRealTimeClockV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRealTimeClockV2.FUNCTION_GET_IDENTITY] = BrickletRealTimeClockV2.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletRealTimeClockV2.CALLBACK_DATE_TIME] = (25, 'H B B B B B B B q')
        self.callback_formats[BrickletRealTimeClockV2.CALLBACK_ALARM] = (25, 'H B B B B B B B q')

        ipcon.add_device(self)

    def set_date_time(self, year, month, day, hour, minute, second, centisecond, weekday):
        r"""
        Sets the current date (including weekday) and the current time.

        If the backup battery is installed then the real-time clock keeps date and
        time even if the Bricklet is not powered by a Brick.

        The real-time clock handles leap year and inserts the 29th of February
        accordingly. But leap seconds, time zones and daylight saving time are not
        handled.
        """
        self.check_validity()

        year = int(year)
        month = int(month)
        day = int(day)
        hour = int(hour)
        minute = int(minute)
        second = int(second)
        centisecond = int(centisecond)
        weekday = int(weekday)

        self.ipcon.send_request(self, BrickletRealTimeClockV2.FUNCTION_SET_DATE_TIME, (year, month, day, hour, minute, second, centisecond, weekday), 'H B B B B B B B', 0, '')

    def get_date_time(self):
        r"""
        Returns the current date (including weekday) and the current time of the
        real-time.

        The timestamp represents the current date and the the current time of the
        real-time clock converted to milliseconds and is an offset to 2000-01-01 00:00:00.0000.
        """
        self.check_validity()

        return GetDateTime(*self.ipcon.send_request(self, BrickletRealTimeClockV2.FUNCTION_GET_DATE_TIME, (), '', 25, 'H B B B B B B B q'))

    def get_timestamp(self):
        r"""
        Returns the current date and the time of the real-time clock converted to
        milliseconds. The timestamp has an effective resolution of hundredths of a
        second and is an offset to 2000-01-01 00:00:00.0000.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRealTimeClockV2.FUNCTION_GET_TIMESTAMP, (), '', 16, 'q')

    def set_offset(self, offset):
        r"""
        Sets the offset the real-time clock should compensate for in 2.17 ppm steps
        between -277.76 ppm (-128) and +275.59 ppm (127).

        The real-time clock time can deviate from the actual time due to the frequency
        deviation of its 32.768 kHz crystal. Even without compensation (factory
        default) the resulting time deviation should be at most ±20 ppm (±52.6
        seconds per month).

        This deviation can be calculated by comparing the same duration measured by the
        real-time clock (``rtc_duration``) an accurate reference clock
        (``ref_duration``).

        For best results the configured offset should be set to 0 ppm first and then a
        duration of at least 6 hours should be measured.

        The new offset (``new_offset``) can be calculated from the currently configured
        offset (``current_offset``) and the measured durations as follow::

          new_offset = current_offset - round(1000000 * (rtc_duration - ref_duration) / rtc_duration / 2.17)

        If you want to calculate the offset, then we recommend using the calibration
        dialog in Brick Viewer, instead of doing it manually.

        The offset is saved in the EEPROM of the Bricklet and only needs to be
        configured once.
        """
        self.check_validity()

        offset = int(offset)

        self.ipcon.send_request(self, BrickletRealTimeClockV2.FUNCTION_SET_OFFSET, (offset,), 'b', 0, '')

    def get_offset(self):
        r"""
        Returns the offset as set by :func:`Set Offset`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRealTimeClockV2.FUNCTION_GET_OFFSET, (), '', 9, 'b')

    def set_date_time_callback_configuration(self, period):
        r"""
        Sets the period with which the :cb:`Date Time` callback is triggered
        periodically. A value of 0 turns the callback off.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletRealTimeClockV2.FUNCTION_SET_DATE_TIME_CALLBACK_CONFIGURATION, (period,), 'I', 0, '')

    def get_date_time_callback_configuration(self):
        r"""
        Returns the period as set by :func:`Set Date Time Callback Configuration`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRealTimeClockV2.FUNCTION_GET_DATE_TIME_CALLBACK_CONFIGURATION, (), '', 12, 'I')

    def set_alarm(self, month, day, hour, minute, second, weekday, interval):
        r"""
        Configures a repeatable alarm. The :cb:`Alarm` callback is triggered if the
        current date and time matches the configured alarm.

        Setting a parameter to -1 means that it should be disabled and doesn't take part
        in the match. Setting all parameters to -1 disables the alarm completely.

        For example, to make the alarm trigger every day at 7:30 AM it can be
        configured as (-1, -1, 7, 30, -1, -1, -1). The hour is set to match 7 and the
        minute is set to match 30. The alarm is triggered if all enabled parameters
        match.

        The interval has a special role. It allows to make the alarm reconfigure itself.
        This is useful if you need a repeated alarm that cannot be expressed by matching
        the current date and time. For example, to make the alarm trigger every 23
        seconds it can be configured as (-1, -1, -1, -1, -1, -1, 23). Internally the
        Bricklet will take the current date and time, add 23 seconds to it and set the
        result as its alarm. The first alarm will be triggered 23 seconds after the
        call. Because the interval is not -1, the Bricklet will do the same again
        internally, take the current date and time, add 23 seconds to it and set that
        as its alarm. This results in a repeated alarm that triggers every 23 seconds.

        The interval can also be used in combination with the other parameters. For
        example, configuring the alarm as (-1, -1, 7, 30, -1, -1, 300) results in an
        alarm that triggers every day at 7:30 AM and is then repeated every 5 minutes.
        """
        self.check_validity()

        month = int(month)
        day = int(day)
        hour = int(hour)
        minute = int(minute)
        second = int(second)
        weekday = int(weekday)
        interval = int(interval)

        self.ipcon.send_request(self, BrickletRealTimeClockV2.FUNCTION_SET_ALARM, (month, day, hour, minute, second, weekday, interval), 'b b b b b b i', 0, '')

    def get_alarm(self):
        r"""
        Returns the alarm configuration as set by :func:`Set Alarm`.
        """
        self.check_validity()

        return GetAlarm(*self.ipcon.send_request(self, BrickletRealTimeClockV2.FUNCTION_GET_ALARM, (), '', 18, 'b b b b b b i'))

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletRealTimeClockV2.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletRealTimeClockV2.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRealTimeClockV2.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletRealTimeClockV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletRealTimeClockV2.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletRealTimeClockV2.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRealTimeClockV2.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRealTimeClockV2.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletRealTimeClockV2.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletRealTimeClockV2.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRealTimeClockV2.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletRealTimeClockV2.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

RealTimeClockV2 = BrickletRealTimeClockV2 # for backward compatibility
