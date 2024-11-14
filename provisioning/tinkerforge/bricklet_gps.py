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

GetCoordinates = namedtuple('Coordinates', ['latitude', 'ns', 'longitude', 'ew', 'pdop', 'hdop', 'vdop', 'epe'])
GetStatus = namedtuple('Status', ['fix', 'satellites_view', 'satellites_used'])
GetAltitude = namedtuple('Altitude', ['altitude', 'geoidal_separation'])
GetMotion = namedtuple('Motion', ['course', 'speed'])
GetDateTime = namedtuple('DateTime', ['date', 'time'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletGPS(Device):
    r"""
    Determine position, velocity and altitude using GPS
    """

    DEVICE_IDENTIFIER = 222
    DEVICE_DISPLAY_NAME = 'GPS Bricklet'
    DEVICE_URL_PART = 'gps' # internal

    CALLBACK_COORDINATES = 17
    CALLBACK_STATUS = 18
    CALLBACK_ALTITUDE = 19
    CALLBACK_MOTION = 20
    CALLBACK_DATE_TIME = 21


    FUNCTION_GET_COORDINATES = 1
    FUNCTION_GET_STATUS = 2
    FUNCTION_GET_ALTITUDE = 3
    FUNCTION_GET_MOTION = 4
    FUNCTION_GET_DATE_TIME = 5
    FUNCTION_RESTART = 6
    FUNCTION_SET_COORDINATES_CALLBACK_PERIOD = 7
    FUNCTION_GET_COORDINATES_CALLBACK_PERIOD = 8
    FUNCTION_SET_STATUS_CALLBACK_PERIOD = 9
    FUNCTION_GET_STATUS_CALLBACK_PERIOD = 10
    FUNCTION_SET_ALTITUDE_CALLBACK_PERIOD = 11
    FUNCTION_GET_ALTITUDE_CALLBACK_PERIOD = 12
    FUNCTION_SET_MOTION_CALLBACK_PERIOD = 13
    FUNCTION_GET_MOTION_CALLBACK_PERIOD = 14
    FUNCTION_SET_DATE_TIME_CALLBACK_PERIOD = 15
    FUNCTION_GET_DATE_TIME_CALLBACK_PERIOD = 16
    FUNCTION_GET_IDENTITY = 255

    FIX_NO_FIX = 1
    FIX_2D_FIX = 2
    FIX_3D_FIX = 3
    RESTART_TYPE_HOT_START = 0
    RESTART_TYPE_WARM_START = 1
    RESTART_TYPE_COLD_START = 2
    RESTART_TYPE_FACTORY_RESET = 3

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletGPS.DEVICE_IDENTIFIER, BrickletGPS.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 1)

        self.response_expected[BrickletGPS.FUNCTION_GET_COORDINATES] = BrickletGPS.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPS.FUNCTION_GET_STATUS] = BrickletGPS.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPS.FUNCTION_GET_ALTITUDE] = BrickletGPS.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPS.FUNCTION_GET_MOTION] = BrickletGPS.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPS.FUNCTION_GET_DATE_TIME] = BrickletGPS.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPS.FUNCTION_RESTART] = BrickletGPS.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletGPS.FUNCTION_SET_COORDINATES_CALLBACK_PERIOD] = BrickletGPS.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletGPS.FUNCTION_GET_COORDINATES_CALLBACK_PERIOD] = BrickletGPS.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPS.FUNCTION_SET_STATUS_CALLBACK_PERIOD] = BrickletGPS.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletGPS.FUNCTION_GET_STATUS_CALLBACK_PERIOD] = BrickletGPS.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPS.FUNCTION_SET_ALTITUDE_CALLBACK_PERIOD] = BrickletGPS.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletGPS.FUNCTION_GET_ALTITUDE_CALLBACK_PERIOD] = BrickletGPS.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPS.FUNCTION_SET_MOTION_CALLBACK_PERIOD] = BrickletGPS.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletGPS.FUNCTION_GET_MOTION_CALLBACK_PERIOD] = BrickletGPS.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPS.FUNCTION_SET_DATE_TIME_CALLBACK_PERIOD] = BrickletGPS.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletGPS.FUNCTION_GET_DATE_TIME_CALLBACK_PERIOD] = BrickletGPS.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPS.FUNCTION_GET_IDENTITY] = BrickletGPS.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletGPS.CALLBACK_COORDINATES] = (26, 'I c I c H H H H')
        self.callback_formats[BrickletGPS.CALLBACK_STATUS] = (11, 'B B B')
        self.callback_formats[BrickletGPS.CALLBACK_ALTITUDE] = (16, 'i i')
        self.callback_formats[BrickletGPS.CALLBACK_MOTION] = (16, 'I I')
        self.callback_formats[BrickletGPS.CALLBACK_DATE_TIME] = (16, 'I I')

        ipcon.add_device(self)

    def get_coordinates(self):
        r"""
        Returns the GPS coordinates. Latitude and longitude are given in the
        ``DD.dddddd°`` format, the value 57123468 means 57.123468°.
        The parameter ``ns`` and ``ew`` are the cardinal directions for
        latitude and longitude. Possible values for ``ns`` and ``ew`` are 'N', 'S', 'E'
        and 'W' (north, south, east and west).

        PDOP, HDOP and VDOP are the dilution of precision (DOP) values. They specify
        the additional multiplicative effect of GPS satellite geometry on GPS
        precision. See
        `here <https://en.wikipedia.org/wiki/Dilution_of_precision_(GPS)>`__
        for more information.

        EPE is the "Estimated Position Error". This is not the
        absolute maximum error, it is the error with a specific confidence. See
        `here <https://www.nps.gov/gis/gps/WhatisEPE.html>`__ for more information.

        This data is only valid if there is currently a fix as indicated by
        :func:`Get Status`.
        """
        self.check_validity()

        return GetCoordinates(*self.ipcon.send_request(self, BrickletGPS.FUNCTION_GET_COORDINATES, (), '', 26, 'I c I c H H H H'))

    def get_status(self):
        r"""
        Returns the current fix status, the number of satellites that are in view and
        the number of satellites that are currently used.

        Possible fix status values can be:

        .. csv-table::
         :header: "Value", "Description"
         :widths: 10, 100

         "1", "No Fix, :func:`Get Coordinates`, :func:`Get Altitude` and :func:`Get Motion` return invalid data"
         "2", "2D Fix, only :func:`Get Coordinates` and :func:`Get Motion` return valid data"
         "3", "3D Fix, :func:`Get Coordinates`, :func:`Get Altitude` and :func:`Get Motion` return valid data"

        There is also a :ref:`blue LED <gps_bricklet_fix_led>` on the Bricklet that
        indicates the fix status.
        """
        self.check_validity()

        return GetStatus(*self.ipcon.send_request(self, BrickletGPS.FUNCTION_GET_STATUS, (), '', 11, 'B B B'))

    def get_altitude(self):
        r"""
        Returns the current altitude and corresponding geoidal separation.

        This data is only valid if there is currently a fix as indicated by
        :func:`Get Status`.
        """
        self.check_validity()

        return GetAltitude(*self.ipcon.send_request(self, BrickletGPS.FUNCTION_GET_ALTITUDE, (), '', 16, 'i i'))

    def get_motion(self):
        r"""
        Returns the current course and speed. A course of 0° means the Bricklet is
        traveling north bound and 90° means it is traveling east bound.

        Please note that this only returns useful values if an actual movement
        is present.

        This data is only valid if there is currently a fix as indicated by
        :func:`Get Status`.
        """
        self.check_validity()

        return GetMotion(*self.ipcon.send_request(self, BrickletGPS.FUNCTION_GET_MOTION, (), '', 16, 'I I'))

    def get_date_time(self):
        r"""
        Returns the current date and time. The date is
        given in the format ``ddmmyy`` and the time is given
        in the format ``hhmmss.sss``. For example, 140713 means
        14.07.13 as date and 195923568 means 19:59:23.568 as time.
        """
        self.check_validity()

        return GetDateTime(*self.ipcon.send_request(self, BrickletGPS.FUNCTION_GET_DATE_TIME, (), '', 16, 'I I'))

    def restart(self, restart_type):
        r"""
        Restarts the GPS Bricklet, the following restart types are available:

        .. csv-table::
         :header: "Value", "Description"
         :widths: 10, 100

         "0", "Hot start (use all available data in the NV store)"
         "1", "Warm start (don't use ephemeris at restart)"
         "2", "Cold start (don't use time, position, almanacs and ephemeris at restart)"
         "3", "Factory reset (clear all system/user configurations at restart)"
        """
        self.check_validity()

        restart_type = int(restart_type)

        self.ipcon.send_request(self, BrickletGPS.FUNCTION_RESTART, (restart_type,), 'B', 0, '')

    def set_coordinates_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Coordinates` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Coordinates` callback is only triggered if the coordinates changed
        since the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletGPS.FUNCTION_SET_COORDINATES_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_coordinates_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Coordinates Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletGPS.FUNCTION_GET_COORDINATES_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_status_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Status` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Status` callback is only triggered if the status changed since the
        last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletGPS.FUNCTION_SET_STATUS_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_status_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Status Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletGPS.FUNCTION_GET_STATUS_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_altitude_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Altitude` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Altitude` callback is only triggered if the altitude changed since
        the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletGPS.FUNCTION_SET_ALTITUDE_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_altitude_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Altitude Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletGPS.FUNCTION_GET_ALTITUDE_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_motion_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Motion` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Motion` callback is only triggered if the motion changed since the
        last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletGPS.FUNCTION_SET_MOTION_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_motion_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Motion Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletGPS.FUNCTION_GET_MOTION_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_date_time_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Date Time` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Date Time` callback is only triggered if the date or time changed
        since the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletGPS.FUNCTION_SET_DATE_TIME_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_date_time_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Date Time Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletGPS.FUNCTION_GET_DATE_TIME_CALLBACK_PERIOD, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletGPS.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

GPS = BrickletGPS # for backward compatibility
