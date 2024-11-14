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

GetAcceleration = namedtuple('Acceleration', ['x', 'y', 'z'])
GetAccelerationCallbackThreshold = namedtuple('AccelerationCallbackThreshold', ['option', 'min_x', 'max_x', 'min_y', 'max_y', 'min_z', 'max_z'])
GetConfiguration = namedtuple('Configuration', ['data_rate', 'full_scale', 'filter_bandwidth'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletAccelerometer(Device):
    r"""
    Measures acceleration in three axis
    """

    DEVICE_IDENTIFIER = 250
    DEVICE_DISPLAY_NAME = 'Accelerometer Bricklet'
    DEVICE_URL_PART = 'accelerometer' # internal

    CALLBACK_ACCELERATION = 14
    CALLBACK_ACCELERATION_REACHED = 15


    FUNCTION_GET_ACCELERATION = 1
    FUNCTION_SET_ACCELERATION_CALLBACK_PERIOD = 2
    FUNCTION_GET_ACCELERATION_CALLBACK_PERIOD = 3
    FUNCTION_SET_ACCELERATION_CALLBACK_THRESHOLD = 4
    FUNCTION_GET_ACCELERATION_CALLBACK_THRESHOLD = 5
    FUNCTION_SET_DEBOUNCE_PERIOD = 6
    FUNCTION_GET_DEBOUNCE_PERIOD = 7
    FUNCTION_GET_TEMPERATURE = 8
    FUNCTION_SET_CONFIGURATION = 9
    FUNCTION_GET_CONFIGURATION = 10
    FUNCTION_LED_ON = 11
    FUNCTION_LED_OFF = 12
    FUNCTION_IS_LED_ON = 13
    FUNCTION_GET_IDENTITY = 255

    THRESHOLD_OPTION_OFF = 'x'
    THRESHOLD_OPTION_OUTSIDE = 'o'
    THRESHOLD_OPTION_INSIDE = 'i'
    THRESHOLD_OPTION_SMALLER = '<'
    THRESHOLD_OPTION_GREATER = '>'
    DATA_RATE_OFF = 0
    DATA_RATE_3HZ = 1
    DATA_RATE_6HZ = 2
    DATA_RATE_12HZ = 3
    DATA_RATE_25HZ = 4
    DATA_RATE_50HZ = 5
    DATA_RATE_100HZ = 6
    DATA_RATE_400HZ = 7
    DATA_RATE_800HZ = 8
    DATA_RATE_1600HZ = 9
    FULL_SCALE_2G = 0
    FULL_SCALE_4G = 1
    FULL_SCALE_6G = 2
    FULL_SCALE_8G = 3
    FULL_SCALE_16G = 4
    FILTER_BANDWIDTH_800HZ = 0
    FILTER_BANDWIDTH_400HZ = 1
    FILTER_BANDWIDTH_200HZ = 2
    FILTER_BANDWIDTH_50HZ = 3

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletAccelerometer.DEVICE_IDENTIFIER, BrickletAccelerometer.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 1)

        self.response_expected[BrickletAccelerometer.FUNCTION_GET_ACCELERATION] = BrickletAccelerometer.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAccelerometer.FUNCTION_SET_ACCELERATION_CALLBACK_PERIOD] = BrickletAccelerometer.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletAccelerometer.FUNCTION_GET_ACCELERATION_CALLBACK_PERIOD] = BrickletAccelerometer.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAccelerometer.FUNCTION_SET_ACCELERATION_CALLBACK_THRESHOLD] = BrickletAccelerometer.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletAccelerometer.FUNCTION_GET_ACCELERATION_CALLBACK_THRESHOLD] = BrickletAccelerometer.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAccelerometer.FUNCTION_SET_DEBOUNCE_PERIOD] = BrickletAccelerometer.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletAccelerometer.FUNCTION_GET_DEBOUNCE_PERIOD] = BrickletAccelerometer.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAccelerometer.FUNCTION_GET_TEMPERATURE] = BrickletAccelerometer.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAccelerometer.FUNCTION_SET_CONFIGURATION] = BrickletAccelerometer.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAccelerometer.FUNCTION_GET_CONFIGURATION] = BrickletAccelerometer.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAccelerometer.FUNCTION_LED_ON] = BrickletAccelerometer.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAccelerometer.FUNCTION_LED_OFF] = BrickletAccelerometer.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAccelerometer.FUNCTION_IS_LED_ON] = BrickletAccelerometer.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAccelerometer.FUNCTION_GET_IDENTITY] = BrickletAccelerometer.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletAccelerometer.CALLBACK_ACCELERATION] = (14, 'h h h')
        self.callback_formats[BrickletAccelerometer.CALLBACK_ACCELERATION_REACHED] = (14, 'h h h')

        ipcon.add_device(self)

    def get_acceleration(self):
        r"""
        Returns the acceleration in x, y and z direction. The values
        are given in gₙ/1000 (1gₙ = 9.80665m/s²). The range is
        configured with :func:`Set Configuration`.

        If you want to get the acceleration periodically, it is recommended
        to use the :cb:`Acceleration` callback and set the period with
        :func:`Set Acceleration Callback Period`.
        """
        self.check_validity()

        return GetAcceleration(*self.ipcon.send_request(self, BrickletAccelerometer.FUNCTION_GET_ACCELERATION, (), '', 14, 'h h h'))

    def set_acceleration_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Acceleration` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Acceleration` callback is only triggered if the acceleration has
        changed since the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletAccelerometer.FUNCTION_SET_ACCELERATION_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_acceleration_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Acceleration Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAccelerometer.FUNCTION_GET_ACCELERATION_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_acceleration_callback_threshold(self, option, min_x, max_x, min_y, max_y, min_z, max_z):
        r"""
        Sets the thresholds for the :cb:`Acceleration Reached` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Callback is turned off"
         "'o'",    "Callback is triggered when the acceleration is *outside* the min and max values"
         "'i'",    "Callback is triggered when the acceleration is *inside* the min and max values"
         "'<'",    "Callback is triggered when the acceleration is smaller than the min value (max is ignored)"
         "'>'",    "Callback is triggered when the acceleration is greater than the min value (max is ignored)"
        """
        self.check_validity()

        option = create_char(option)
        min_x = int(min_x)
        max_x = int(max_x)
        min_y = int(min_y)
        max_y = int(max_y)
        min_z = int(min_z)
        max_z = int(max_z)

        self.ipcon.send_request(self, BrickletAccelerometer.FUNCTION_SET_ACCELERATION_CALLBACK_THRESHOLD, (option, min_x, max_x, min_y, max_y, min_z, max_z), 'c h h h h h h', 0, '')

    def get_acceleration_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Acceleration Callback Threshold`.
        """
        self.check_validity()

        return GetAccelerationCallbackThreshold(*self.ipcon.send_request(self, BrickletAccelerometer.FUNCTION_GET_ACCELERATION_CALLBACK_THRESHOLD, (), '', 21, 'c h h h h h h'))

    def set_debounce_period(self, debounce):
        r"""
        Sets the period with which the threshold callback

        * :cb:`Acceleration Reached`

        is triggered, if the threshold

        * :func:`Set Acceleration Callback Threshold`

        keeps being reached.
        """
        self.check_validity()

        debounce = int(debounce)

        self.ipcon.send_request(self, BrickletAccelerometer.FUNCTION_SET_DEBOUNCE_PERIOD, (debounce,), 'I', 0, '')

    def get_debounce_period(self):
        r"""
        Returns the debounce period as set by :func:`Set Debounce Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAccelerometer.FUNCTION_GET_DEBOUNCE_PERIOD, (), '', 12, 'I')

    def get_temperature(self):
        r"""
        Returns the temperature of the accelerometer.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAccelerometer.FUNCTION_GET_TEMPERATURE, (), '', 10, 'h')

    def set_configuration(self, data_rate, full_scale, filter_bandwidth):
        r"""
        Configures the data rate, full scale range and filter bandwidth.
        Possible values are:

        * Data rate of 0Hz to 1600Hz.
        * Full scale range of ±2gₙ up to ±16gₙ.
        * Filter bandwidth between 50Hz and 800Hz.

        Decreasing data rate or full scale range will also decrease the noise on
        the data.
        """
        self.check_validity()

        data_rate = int(data_rate)
        full_scale = int(full_scale)
        filter_bandwidth = int(filter_bandwidth)

        self.ipcon.send_request(self, BrickletAccelerometer.FUNCTION_SET_CONFIGURATION, (data_rate, full_scale, filter_bandwidth), 'B B B', 0, '')

    def get_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Configuration`.
        """
        self.check_validity()

        return GetConfiguration(*self.ipcon.send_request(self, BrickletAccelerometer.FUNCTION_GET_CONFIGURATION, (), '', 11, 'B B B'))

    def led_on(self):
        r"""
        Enables the LED on the Bricklet.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletAccelerometer.FUNCTION_LED_ON, (), '', 0, '')

    def led_off(self):
        r"""
        Disables the LED on the Bricklet.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletAccelerometer.FUNCTION_LED_OFF, (), '', 0, '')

    def is_led_on(self):
        r"""
        Returns *true* if the LED is enabled, *false* otherwise.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAccelerometer.FUNCTION_IS_LED_ON, (), '', 9, '!')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletAccelerometer.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

Accelerometer = BrickletAccelerometer # for backward compatibility
