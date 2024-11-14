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

GetDateTime = namedtuple('DateTime', ['year', 'month', 'day', 'hour', 'minute', 'second', 'centisecond', 'weekday'])
GetAlarm = namedtuple('Alarm', ['month', 'day', 'hour', 'minute', 'second', 'weekday', 'interval'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletRealTimeClock(Device):
    r"""
    Battery-backed real-time clock
    """

    DEVICE_IDENTIFIER = 268
    DEVICE_DISPLAY_NAME = 'Real-Time Clock Bricklet'
    DEVICE_URL_PART = 'real_time_clock' # internal

    CALLBACK_DATE_TIME = 10
    CALLBACK_ALARM = 11


    FUNCTION_SET_DATE_TIME = 1
    FUNCTION_GET_DATE_TIME = 2
    FUNCTION_GET_TIMESTAMP = 3
    FUNCTION_SET_OFFSET = 4
    FUNCTION_GET_OFFSET = 5
    FUNCTION_SET_DATE_TIME_CALLBACK_PERIOD = 6
    FUNCTION_GET_DATE_TIME_CALLBACK_PERIOD = 7
    FUNCTION_SET_ALARM = 8
    FUNCTION_GET_ALARM = 9
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

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletRealTimeClock.DEVICE_IDENTIFIER, BrickletRealTimeClock.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 1)

        self.response_expected[BrickletRealTimeClock.FUNCTION_SET_DATE_TIME] = BrickletRealTimeClock.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRealTimeClock.FUNCTION_GET_DATE_TIME] = BrickletRealTimeClock.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRealTimeClock.FUNCTION_GET_TIMESTAMP] = BrickletRealTimeClock.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRealTimeClock.FUNCTION_SET_OFFSET] = BrickletRealTimeClock.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRealTimeClock.FUNCTION_GET_OFFSET] = BrickletRealTimeClock.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRealTimeClock.FUNCTION_SET_DATE_TIME_CALLBACK_PERIOD] = BrickletRealTimeClock.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletRealTimeClock.FUNCTION_GET_DATE_TIME_CALLBACK_PERIOD] = BrickletRealTimeClock.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRealTimeClock.FUNCTION_SET_ALARM] = BrickletRealTimeClock.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletRealTimeClock.FUNCTION_GET_ALARM] = BrickletRealTimeClock.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRealTimeClock.FUNCTION_GET_IDENTITY] = BrickletRealTimeClock.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletRealTimeClock.CALLBACK_DATE_TIME] = (25, 'H B B B B B B B q')
        self.callback_formats[BrickletRealTimeClock.CALLBACK_ALARM] = (25, 'H B B B B B B B q')

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

        self.ipcon.send_request(self, BrickletRealTimeClock.FUNCTION_SET_DATE_TIME, (year, month, day, hour, minute, second, centisecond, weekday), 'H B B B B B B B', 0, '')

    def get_date_time(self):
        r"""
        Returns the current date (including weekday) and the current time of the
        real-time clock.
        """
        self.check_validity()

        return GetDateTime(*self.ipcon.send_request(self, BrickletRealTimeClock.FUNCTION_GET_DATE_TIME, (), '', 17, 'H B B B B B B B'))

    def get_timestamp(self):
        r"""
        Returns the current date and the time of the real-time clock.
        The timestamp has an effective resolution of hundredths of a
        second and is an offset to 2000-01-01 00:00:00.000.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRealTimeClock.FUNCTION_GET_TIMESTAMP, (), '', 16, 'q')

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

        self.ipcon.send_request(self, BrickletRealTimeClock.FUNCTION_SET_OFFSET, (offset,), 'b', 0, '')

    def get_offset(self):
        r"""
        Returns the offset as set by :func:`Set Offset`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRealTimeClock.FUNCTION_GET_OFFSET, (), '', 9, 'b')

    def set_date_time_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Date Time` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Date Time` Callback is only triggered if the date or time changed
        since the last triggering.

        .. versionadded:: 2.0.1$nbsp;(Plugin)
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletRealTimeClock.FUNCTION_SET_DATE_TIME_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_date_time_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Date Time Callback Period`.

        .. versionadded:: 2.0.1$nbsp;(Plugin)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRealTimeClock.FUNCTION_GET_DATE_TIME_CALLBACK_PERIOD, (), '', 12, 'I')

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

        .. versionadded:: 2.0.1$nbsp;(Plugin)
        """
        self.check_validity()

        month = int(month)
        day = int(day)
        hour = int(hour)
        minute = int(minute)
        second = int(second)
        weekday = int(weekday)
        interval = int(interval)

        self.ipcon.send_request(self, BrickletRealTimeClock.FUNCTION_SET_ALARM, (month, day, hour, minute, second, weekday, interval), 'b b b b b b i', 0, '')

    def get_alarm(self):
        r"""
        Returns the alarm configuration as set by :func:`Set Alarm`.

        .. versionadded:: 2.0.1$nbsp;(Plugin)
        """
        self.check_validity()

        return GetAlarm(*self.ipcon.send_request(self, BrickletRealTimeClock.FUNCTION_GET_ALARM, (), '', 18, 'b b b b b b i'))

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletRealTimeClock.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

RealTimeClock = BrickletRealTimeClock # for backward compatibility
