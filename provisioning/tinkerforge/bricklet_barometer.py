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

GetAirPressureCallbackThreshold = namedtuple('AirPressureCallbackThreshold', ['option', 'min', 'max'])
GetAltitudeCallbackThreshold = namedtuple('AltitudeCallbackThreshold', ['option', 'min', 'max'])
GetAveraging = namedtuple('Averaging', ['moving_average_pressure', 'average_pressure', 'average_temperature'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletBarometer(Device):
    r"""
    Measures air pressure and altitude changes
    """

    DEVICE_IDENTIFIER = 221
    DEVICE_DISPLAY_NAME = 'Barometer Bricklet'
    DEVICE_URL_PART = 'barometer' # internal

    CALLBACK_AIR_PRESSURE = 15
    CALLBACK_ALTITUDE = 16
    CALLBACK_AIR_PRESSURE_REACHED = 17
    CALLBACK_ALTITUDE_REACHED = 18


    FUNCTION_GET_AIR_PRESSURE = 1
    FUNCTION_GET_ALTITUDE = 2
    FUNCTION_SET_AIR_PRESSURE_CALLBACK_PERIOD = 3
    FUNCTION_GET_AIR_PRESSURE_CALLBACK_PERIOD = 4
    FUNCTION_SET_ALTITUDE_CALLBACK_PERIOD = 5
    FUNCTION_GET_ALTITUDE_CALLBACK_PERIOD = 6
    FUNCTION_SET_AIR_PRESSURE_CALLBACK_THRESHOLD = 7
    FUNCTION_GET_AIR_PRESSURE_CALLBACK_THRESHOLD = 8
    FUNCTION_SET_ALTITUDE_CALLBACK_THRESHOLD = 9
    FUNCTION_GET_ALTITUDE_CALLBACK_THRESHOLD = 10
    FUNCTION_SET_DEBOUNCE_PERIOD = 11
    FUNCTION_GET_DEBOUNCE_PERIOD = 12
    FUNCTION_SET_REFERENCE_AIR_PRESSURE = 13
    FUNCTION_GET_CHIP_TEMPERATURE = 14
    FUNCTION_GET_REFERENCE_AIR_PRESSURE = 19
    FUNCTION_SET_AVERAGING = 20
    FUNCTION_GET_AVERAGING = 21
    FUNCTION_SET_I2C_MODE = 22
    FUNCTION_GET_I2C_MODE = 23
    FUNCTION_GET_IDENTITY = 255

    THRESHOLD_OPTION_OFF = 'x'
    THRESHOLD_OPTION_OUTSIDE = 'o'
    THRESHOLD_OPTION_INSIDE = 'i'
    THRESHOLD_OPTION_SMALLER = '<'
    THRESHOLD_OPTION_GREATER = '>'
    I2C_MODE_FAST = 0
    I2C_MODE_SLOW = 1

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletBarometer.DEVICE_IDENTIFIER, BrickletBarometer.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 2)

        self.response_expected[BrickletBarometer.FUNCTION_GET_AIR_PRESSURE] = BrickletBarometer.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometer.FUNCTION_GET_ALTITUDE] = BrickletBarometer.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometer.FUNCTION_SET_AIR_PRESSURE_CALLBACK_PERIOD] = BrickletBarometer.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletBarometer.FUNCTION_GET_AIR_PRESSURE_CALLBACK_PERIOD] = BrickletBarometer.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometer.FUNCTION_SET_ALTITUDE_CALLBACK_PERIOD] = BrickletBarometer.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletBarometer.FUNCTION_GET_ALTITUDE_CALLBACK_PERIOD] = BrickletBarometer.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometer.FUNCTION_SET_AIR_PRESSURE_CALLBACK_THRESHOLD] = BrickletBarometer.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletBarometer.FUNCTION_GET_AIR_PRESSURE_CALLBACK_THRESHOLD] = BrickletBarometer.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometer.FUNCTION_SET_ALTITUDE_CALLBACK_THRESHOLD] = BrickletBarometer.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletBarometer.FUNCTION_GET_ALTITUDE_CALLBACK_THRESHOLD] = BrickletBarometer.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometer.FUNCTION_SET_DEBOUNCE_PERIOD] = BrickletBarometer.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletBarometer.FUNCTION_GET_DEBOUNCE_PERIOD] = BrickletBarometer.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometer.FUNCTION_SET_REFERENCE_AIR_PRESSURE] = BrickletBarometer.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletBarometer.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletBarometer.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometer.FUNCTION_GET_REFERENCE_AIR_PRESSURE] = BrickletBarometer.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometer.FUNCTION_SET_AVERAGING] = BrickletBarometer.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletBarometer.FUNCTION_GET_AVERAGING] = BrickletBarometer.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometer.FUNCTION_SET_I2C_MODE] = BrickletBarometer.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletBarometer.FUNCTION_GET_I2C_MODE] = BrickletBarometer.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometer.FUNCTION_GET_IDENTITY] = BrickletBarometer.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletBarometer.CALLBACK_AIR_PRESSURE] = (12, 'i')
        self.callback_formats[BrickletBarometer.CALLBACK_ALTITUDE] = (12, 'i')
        self.callback_formats[BrickletBarometer.CALLBACK_AIR_PRESSURE_REACHED] = (12, 'i')
        self.callback_formats[BrickletBarometer.CALLBACK_ALTITUDE_REACHED] = (12, 'i')

        ipcon.add_device(self)

    def get_air_pressure(self):
        r"""
        Returns the air pressure of the air pressure sensor.

        If you want to get the air pressure periodically, it is recommended to use the
        :cb:`Air Pressure` callback and set the period with
        :func:`Set Air Pressure Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletBarometer.FUNCTION_GET_AIR_PRESSURE, (), '', 12, 'i')

    def get_altitude(self):
        r"""
        Returns the relative altitude of the air pressure sensor. The value is
        calculated based on the difference between the current air pressure
        and the reference air pressure that can be set with :func:`Set Reference Air Pressure`.

        If you want to get the altitude periodically, it is recommended to use the
        :cb:`Altitude` callback and set the period with
        :func:`Set Altitude Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletBarometer.FUNCTION_GET_ALTITUDE, (), '', 12, 'i')

    def set_air_pressure_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Air Pressure` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Air Pressure` callback is only triggered if the air pressure has
        changed since the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletBarometer.FUNCTION_SET_AIR_PRESSURE_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_air_pressure_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Air Pressure Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletBarometer.FUNCTION_GET_AIR_PRESSURE_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_altitude_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Altitude` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Altitude` callback is only triggered if the altitude has changed since
        the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletBarometer.FUNCTION_SET_ALTITUDE_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_altitude_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Altitude Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletBarometer.FUNCTION_GET_ALTITUDE_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_air_pressure_callback_threshold(self, option, min, max):
        r"""
        Sets the thresholds for the :cb:`Air Pressure Reached` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Callback is turned off"
         "'o'",    "Callback is triggered when the air pressure is *outside* the min and max values"
         "'i'",    "Callback is triggered when the air pressure is *inside* the min and max values"
         "'<'",    "Callback is triggered when the air pressure is smaller than the min value (max is ignored)"
         "'>'",    "Callback is triggered when the air pressure is greater than the min value (max is ignored)"
        """
        self.check_validity()

        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickletBarometer.FUNCTION_SET_AIR_PRESSURE_CALLBACK_THRESHOLD, (option, min, max), 'c i i', 0, '')

    def get_air_pressure_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Air Pressure Callback Threshold`.
        """
        self.check_validity()

        return GetAirPressureCallbackThreshold(*self.ipcon.send_request(self, BrickletBarometer.FUNCTION_GET_AIR_PRESSURE_CALLBACK_THRESHOLD, (), '', 17, 'c i i'))

    def set_altitude_callback_threshold(self, option, min, max):
        r"""
        Sets the thresholds for the :cb:`Altitude Reached` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Callback is turned off"
         "'o'",    "Callback is triggered when the altitude is *outside* the min and max values"
         "'i'",    "Callback is triggered when the altitude is *inside* the min and max values"
         "'<'",    "Callback is triggered when the altitude is smaller than the min value (max is ignored)"
         "'>'",    "Callback is triggered when the altitude is greater than the min value (max is ignored)"
        """
        self.check_validity()

        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickletBarometer.FUNCTION_SET_ALTITUDE_CALLBACK_THRESHOLD, (option, min, max), 'c i i', 0, '')

    def get_altitude_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Altitude Callback Threshold`.
        """
        self.check_validity()

        return GetAltitudeCallbackThreshold(*self.ipcon.send_request(self, BrickletBarometer.FUNCTION_GET_ALTITUDE_CALLBACK_THRESHOLD, (), '', 17, 'c i i'))

    def set_debounce_period(self, debounce):
        r"""
        Sets the period with which the threshold callbacks

        * :cb:`Air Pressure Reached`,
        * :cb:`Altitude Reached`

        are triggered, if the thresholds

        * :func:`Set Air Pressure Callback Threshold`,
        * :func:`Set Altitude Callback Threshold`

        keep being reached.
        """
        self.check_validity()

        debounce = int(debounce)

        self.ipcon.send_request(self, BrickletBarometer.FUNCTION_SET_DEBOUNCE_PERIOD, (debounce,), 'I', 0, '')

    def get_debounce_period(self):
        r"""
        Returns the debounce period as set by :func:`Set Debounce Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletBarometer.FUNCTION_GET_DEBOUNCE_PERIOD, (), '', 12, 'I')

    def set_reference_air_pressure(self, air_pressure):
        r"""
        Sets the reference air pressure for the altitude calculation.
        Setting the reference to the current air pressure results in a calculated
        altitude of 0cm. Passing 0 is a shortcut for passing the current air pressure as
        reference.

        Well known reference values are the Q codes
        `QNH <https://en.wikipedia.org/wiki/QNH>`__ and
        `QFE <https://en.wikipedia.org/wiki/Mean_sea_level_pressure#Mean_sea_level_pressure>`__
        used in aviation.
        """
        self.check_validity()

        air_pressure = int(air_pressure)

        self.ipcon.send_request(self, BrickletBarometer.FUNCTION_SET_REFERENCE_AIR_PRESSURE, (air_pressure,), 'i', 0, '')

    def get_chip_temperature(self):
        r"""
        Returns the temperature of the air pressure sensor.

        This temperature is used internally for temperature compensation of the air
        pressure measurement. It is not as accurate as the temperature measured by the
        :ref:`temperature_bricklet` or the :ref:`temperature_ir_bricklet`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletBarometer.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def get_reference_air_pressure(self):
        r"""
        Returns the reference air pressure as set by :func:`Set Reference Air Pressure`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletBarometer.FUNCTION_GET_REFERENCE_AIR_PRESSURE, (), '', 12, 'i')

    def set_averaging(self, moving_average_pressure, average_pressure, average_temperature):
        r"""
        Sets the different averaging parameters. It is possible to set
        the length of a normal averaging for the temperature and pressure,
        as well as an additional length of a
        `moving average <https://en.wikipedia.org/wiki/Moving_average>`__
        for the pressure. The moving average is calculated from the normal
        averages.  There is no moving average for the temperature.

        Setting the all three parameters to 0 will turn the averaging
        completely off. If the averaging is off, there is lots of noise
        on the data, but the data is without delay. Thus we recommend
        to turn the averaging off if the Barometer Bricklet data is
        to be used for sensor fusion with other sensors.

        .. versionadded:: 2.0.1$nbsp;(Plugin)
        """
        self.check_validity()

        moving_average_pressure = int(moving_average_pressure)
        average_pressure = int(average_pressure)
        average_temperature = int(average_temperature)

        self.ipcon.send_request(self, BrickletBarometer.FUNCTION_SET_AVERAGING, (moving_average_pressure, average_pressure, average_temperature), 'B B B', 0, '')

    def get_averaging(self):
        r"""
        Returns the averaging configuration as set by :func:`Set Averaging`.

        .. versionadded:: 2.0.1$nbsp;(Plugin)
        """
        self.check_validity()

        return GetAveraging(*self.ipcon.send_request(self, BrickletBarometer.FUNCTION_GET_AVERAGING, (), '', 11, 'B B B'))

    def set_i2c_mode(self, mode):
        r"""
        Sets the I2C mode. Possible modes are:

        * 0: Fast (400kHz)
        * 1: Slow (100kHz)

        If you have problems with obvious outliers in the
        Barometer Bricklet measurements, they may be caused by EMI issues.
        In this case it may be helpful to lower the I2C speed.

        It is however not recommended to lower the I2C speed in applications where
        a high throughput needs to be achieved.

        .. versionadded:: 2.0.3$nbsp;(Plugin)
        """
        self.check_validity()

        mode = int(mode)

        self.ipcon.send_request(self, BrickletBarometer.FUNCTION_SET_I2C_MODE, (mode,), 'B', 0, '')

    def get_i2c_mode(self):
        r"""
        Returns the I2C mode as set by :func:`Set I2C Mode`.

        .. versionadded:: 2.0.3$nbsp;(Plugin)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletBarometer.FUNCTION_GET_I2C_MODE, (), '', 9, 'B')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletBarometer.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

Barometer = BrickletBarometer # for backward compatibility
