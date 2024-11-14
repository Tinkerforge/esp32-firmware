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

GetTemperatureCallbackThreshold = namedtuple('TemperatureCallbackThreshold', ['option', 'min', 'max'])
GetResistanceCallbackThreshold = namedtuple('ResistanceCallbackThreshold', ['option', 'min', 'max'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletPTC(Device):
    r"""
    Reads temperatures from Pt100 und Pt1000 sensors
    """

    DEVICE_IDENTIFIER = 226
    DEVICE_DISPLAY_NAME = 'PTC Bricklet'
    DEVICE_URL_PART = 'ptc' # internal

    CALLBACK_TEMPERATURE = 13
    CALLBACK_TEMPERATURE_REACHED = 14
    CALLBACK_RESISTANCE = 15
    CALLBACK_RESISTANCE_REACHED = 16
    CALLBACK_SENSOR_CONNECTED = 24


    FUNCTION_GET_TEMPERATURE = 1
    FUNCTION_GET_RESISTANCE = 2
    FUNCTION_SET_TEMPERATURE_CALLBACK_PERIOD = 3
    FUNCTION_GET_TEMPERATURE_CALLBACK_PERIOD = 4
    FUNCTION_SET_RESISTANCE_CALLBACK_PERIOD = 5
    FUNCTION_GET_RESISTANCE_CALLBACK_PERIOD = 6
    FUNCTION_SET_TEMPERATURE_CALLBACK_THRESHOLD = 7
    FUNCTION_GET_TEMPERATURE_CALLBACK_THRESHOLD = 8
    FUNCTION_SET_RESISTANCE_CALLBACK_THRESHOLD = 9
    FUNCTION_GET_RESISTANCE_CALLBACK_THRESHOLD = 10
    FUNCTION_SET_DEBOUNCE_PERIOD = 11
    FUNCTION_GET_DEBOUNCE_PERIOD = 12
    FUNCTION_SET_NOISE_REJECTION_FILTER = 17
    FUNCTION_GET_NOISE_REJECTION_FILTER = 18
    FUNCTION_IS_SENSOR_CONNECTED = 19
    FUNCTION_SET_WIRE_MODE = 20
    FUNCTION_GET_WIRE_MODE = 21
    FUNCTION_SET_SENSOR_CONNECTED_CALLBACK_CONFIGURATION = 22
    FUNCTION_GET_SENSOR_CONNECTED_CALLBACK_CONFIGURATION = 23
    FUNCTION_GET_IDENTITY = 255

    THRESHOLD_OPTION_OFF = 'x'
    THRESHOLD_OPTION_OUTSIDE = 'o'
    THRESHOLD_OPTION_INSIDE = 'i'
    THRESHOLD_OPTION_SMALLER = '<'
    THRESHOLD_OPTION_GREATER = '>'
    FILTER_OPTION_50HZ = 0
    FILTER_OPTION_60HZ = 1
    WIRE_MODE_2 = 2
    WIRE_MODE_3 = 3
    WIRE_MODE_4 = 4

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletPTC.DEVICE_IDENTIFIER, BrickletPTC.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 1)

        self.response_expected[BrickletPTC.FUNCTION_GET_TEMPERATURE] = BrickletPTC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPTC.FUNCTION_GET_RESISTANCE] = BrickletPTC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPTC.FUNCTION_SET_TEMPERATURE_CALLBACK_PERIOD] = BrickletPTC.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletPTC.FUNCTION_GET_TEMPERATURE_CALLBACK_PERIOD] = BrickletPTC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPTC.FUNCTION_SET_RESISTANCE_CALLBACK_PERIOD] = BrickletPTC.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletPTC.FUNCTION_GET_RESISTANCE_CALLBACK_PERIOD] = BrickletPTC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPTC.FUNCTION_SET_TEMPERATURE_CALLBACK_THRESHOLD] = BrickletPTC.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletPTC.FUNCTION_GET_TEMPERATURE_CALLBACK_THRESHOLD] = BrickletPTC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPTC.FUNCTION_SET_RESISTANCE_CALLBACK_THRESHOLD] = BrickletPTC.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletPTC.FUNCTION_GET_RESISTANCE_CALLBACK_THRESHOLD] = BrickletPTC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPTC.FUNCTION_SET_DEBOUNCE_PERIOD] = BrickletPTC.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletPTC.FUNCTION_GET_DEBOUNCE_PERIOD] = BrickletPTC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPTC.FUNCTION_SET_NOISE_REJECTION_FILTER] = BrickletPTC.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPTC.FUNCTION_GET_NOISE_REJECTION_FILTER] = BrickletPTC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPTC.FUNCTION_IS_SENSOR_CONNECTED] = BrickletPTC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPTC.FUNCTION_SET_WIRE_MODE] = BrickletPTC.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPTC.FUNCTION_GET_WIRE_MODE] = BrickletPTC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPTC.FUNCTION_SET_SENSOR_CONNECTED_CALLBACK_CONFIGURATION] = BrickletPTC.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletPTC.FUNCTION_GET_SENSOR_CONNECTED_CALLBACK_CONFIGURATION] = BrickletPTC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPTC.FUNCTION_GET_IDENTITY] = BrickletPTC.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletPTC.CALLBACK_TEMPERATURE] = (12, 'i')
        self.callback_formats[BrickletPTC.CALLBACK_TEMPERATURE_REACHED] = (12, 'i')
        self.callback_formats[BrickletPTC.CALLBACK_RESISTANCE] = (12, 'i')
        self.callback_formats[BrickletPTC.CALLBACK_RESISTANCE_REACHED] = (12, 'i')
        self.callback_formats[BrickletPTC.CALLBACK_SENSOR_CONNECTED] = (9, '!')

        ipcon.add_device(self)

    def get_temperature(self):
        r"""
        Returns the temperature of connected sensor.

        If you want to get the temperature periodically, it is recommended
        to use the :cb:`Temperature` callback and set the period with
        :func:`Set Temperature Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPTC.FUNCTION_GET_TEMPERATURE, (), '', 12, 'i')

    def get_resistance(self):
        r"""
        Returns the value as measured by the MAX31865 precision delta-sigma ADC.

        The value can be converted with the following formulas:

        * Pt100:  resistance = (value * 390) / 32768
        * Pt1000: resistance = (value * 3900) / 32768

        If you want to get the resistance periodically, it is recommended
        to use the :cb:`Resistance` callback and set the period with
        :func:`Set Resistance Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPTC.FUNCTION_GET_RESISTANCE, (), '', 12, 'i')

    def set_temperature_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Temperature` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Temperature` callback is only triggered if the temperature has
        changed since the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletPTC.FUNCTION_SET_TEMPERATURE_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_temperature_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Temperature Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPTC.FUNCTION_GET_TEMPERATURE_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_resistance_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Resistance` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Resistance` callback is only triggered if the resistance has changed
        since the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletPTC.FUNCTION_SET_RESISTANCE_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_resistance_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Resistance Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPTC.FUNCTION_GET_RESISTANCE_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_temperature_callback_threshold(self, option, min, max):
        r"""
        Sets the thresholds for the :cb:`Temperature Reached` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Callback is turned off"
         "'o'",    "Callback is triggered when the temperature is *outside* the min and max values"
         "'i'",    "Callback is triggered when the temperature is *inside* the min and max values"
         "'<'",    "Callback is triggered when the temperature is smaller than the min value (max is ignored)"
         "'>'",    "Callback is triggered when the temperature is greater than the min value (max is ignored)"
        """
        self.check_validity()

        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickletPTC.FUNCTION_SET_TEMPERATURE_CALLBACK_THRESHOLD, (option, min, max), 'c i i', 0, '')

    def get_temperature_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Temperature Callback Threshold`.
        """
        self.check_validity()

        return GetTemperatureCallbackThreshold(*self.ipcon.send_request(self, BrickletPTC.FUNCTION_GET_TEMPERATURE_CALLBACK_THRESHOLD, (), '', 17, 'c i i'))

    def set_resistance_callback_threshold(self, option, min, max):
        r"""
        Sets the thresholds for the :cb:`Resistance Reached` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Callback is turned off"
         "'o'",    "Callback is triggered when the temperature is *outside* the min and max values"
         "'i'",    "Callback is triggered when the temperature is *inside* the min and max values"
         "'<'",    "Callback is triggered when the temperature is smaller than the min value (max is ignored)"
         "'>'",    "Callback is triggered when the temperature is greater than the min value (max is ignored)"
        """
        self.check_validity()

        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickletPTC.FUNCTION_SET_RESISTANCE_CALLBACK_THRESHOLD, (option, min, max), 'c i i', 0, '')

    def get_resistance_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Resistance Callback Threshold`.
        """
        self.check_validity()

        return GetResistanceCallbackThreshold(*self.ipcon.send_request(self, BrickletPTC.FUNCTION_GET_RESISTANCE_CALLBACK_THRESHOLD, (), '', 17, 'c i i'))

    def set_debounce_period(self, debounce):
        r"""
        Sets the period with which the threshold callback

        * :cb:`Temperature Reached`,
        * :cb:`Resistance Reached`

        is triggered, if the threshold

        * :func:`Set Temperature Callback Threshold`,
        * :func:`Set Resistance Callback Threshold`

        keeps being reached.
        """
        self.check_validity()

        debounce = int(debounce)

        self.ipcon.send_request(self, BrickletPTC.FUNCTION_SET_DEBOUNCE_PERIOD, (debounce,), 'I', 0, '')

    def get_debounce_period(self):
        r"""
        Returns the debounce period as set by :func:`Set Debounce Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPTC.FUNCTION_GET_DEBOUNCE_PERIOD, (), '', 12, 'I')

    def set_noise_rejection_filter(self, filter):
        r"""
        Sets the noise rejection filter to either 50Hz (0) or 60Hz (1).
        Noise from 50Hz or 60Hz power sources (including
        harmonics of the AC power's fundamental frequency) is
        attenuated by 82dB.
        """
        self.check_validity()

        filter = int(filter)

        self.ipcon.send_request(self, BrickletPTC.FUNCTION_SET_NOISE_REJECTION_FILTER, (filter,), 'B', 0, '')

    def get_noise_rejection_filter(self):
        r"""
        Returns the noise rejection filter option as set by
        :func:`Set Noise Rejection Filter`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPTC.FUNCTION_GET_NOISE_REJECTION_FILTER, (), '', 9, 'B')

    def is_sensor_connected(self):
        r"""
        Returns *true* if the sensor is connected correctly.

        If this function
        returns *false*, there is either no Pt100 or Pt1000 sensor connected,
        the sensor is connected incorrectly or the sensor itself is faulty.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPTC.FUNCTION_IS_SENSOR_CONNECTED, (), '', 9, '!')

    def set_wire_mode(self, mode):
        r"""
        Sets the wire mode of the sensor. Possible values are 2, 3 and 4 which
        correspond to 2-, 3- and 4-wire sensors. The value has to match the jumper
        configuration on the Bricklet.
        """
        self.check_validity()

        mode = int(mode)

        self.ipcon.send_request(self, BrickletPTC.FUNCTION_SET_WIRE_MODE, (mode,), 'B', 0, '')

    def get_wire_mode(self):
        r"""
        Returns the wire mode as set by :func:`Set Wire Mode`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPTC.FUNCTION_GET_WIRE_MODE, (), '', 9, 'B')

    def set_sensor_connected_callback_configuration(self, enabled):
        r"""
        If you enable this callback, the :cb:`Sensor Connected` callback is triggered
        every time a Pt sensor is connected/disconnected.

        .. versionadded:: 2.0.2$nbsp;(Plugin)
        """
        self.check_validity()

        enabled = bool(enabled)

        self.ipcon.send_request(self, BrickletPTC.FUNCTION_SET_SENSOR_CONNECTED_CALLBACK_CONFIGURATION, (enabled,), '!', 0, '')

    def get_sensor_connected_callback_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Sensor Connected Callback Configuration`.

        .. versionadded:: 2.0.2$nbsp;(Plugin)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPTC.FUNCTION_GET_SENSOR_CONNECTED_CALLBACK_CONFIGURATION, (), '', 9, '!')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletPTC.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

PTC = BrickletPTC # for backward compatibility
