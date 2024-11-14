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

GetVoltageCallbackThreshold = namedtuple('VoltageCallbackThreshold', ['option', 'min', 'max'])
GetAnalogValueCallbackThreshold = namedtuple('AnalogValueCallbackThreshold', ['option', 'min', 'max'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletAnalogIn(Device):
    r"""
    Measures DC voltage between 0V and 45V
    """

    DEVICE_IDENTIFIER = 219
    DEVICE_DISPLAY_NAME = 'Analog In Bricklet'
    DEVICE_URL_PART = 'analog_in' # internal

    CALLBACK_VOLTAGE = 13
    CALLBACK_ANALOG_VALUE = 14
    CALLBACK_VOLTAGE_REACHED = 15
    CALLBACK_ANALOG_VALUE_REACHED = 16


    FUNCTION_GET_VOLTAGE = 1
    FUNCTION_GET_ANALOG_VALUE = 2
    FUNCTION_SET_VOLTAGE_CALLBACK_PERIOD = 3
    FUNCTION_GET_VOLTAGE_CALLBACK_PERIOD = 4
    FUNCTION_SET_ANALOG_VALUE_CALLBACK_PERIOD = 5
    FUNCTION_GET_ANALOG_VALUE_CALLBACK_PERIOD = 6
    FUNCTION_SET_VOLTAGE_CALLBACK_THRESHOLD = 7
    FUNCTION_GET_VOLTAGE_CALLBACK_THRESHOLD = 8
    FUNCTION_SET_ANALOG_VALUE_CALLBACK_THRESHOLD = 9
    FUNCTION_GET_ANALOG_VALUE_CALLBACK_THRESHOLD = 10
    FUNCTION_SET_DEBOUNCE_PERIOD = 11
    FUNCTION_GET_DEBOUNCE_PERIOD = 12
    FUNCTION_SET_RANGE = 17
    FUNCTION_GET_RANGE = 18
    FUNCTION_SET_AVERAGING = 19
    FUNCTION_GET_AVERAGING = 20
    FUNCTION_GET_IDENTITY = 255

    THRESHOLD_OPTION_OFF = 'x'
    THRESHOLD_OPTION_OUTSIDE = 'o'
    THRESHOLD_OPTION_INSIDE = 'i'
    THRESHOLD_OPTION_SMALLER = '<'
    THRESHOLD_OPTION_GREATER = '>'
    RANGE_AUTOMATIC = 0
    RANGE_UP_TO_6V = 1
    RANGE_UP_TO_10V = 2
    RANGE_UP_TO_36V = 3
    RANGE_UP_TO_45V = 4
    RANGE_UP_TO_3V = 5

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletAnalogIn.DEVICE_IDENTIFIER, BrickletAnalogIn.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 3)

        self.response_expected[BrickletAnalogIn.FUNCTION_GET_VOLTAGE] = BrickletAnalogIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogIn.FUNCTION_GET_ANALOG_VALUE] = BrickletAnalogIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogIn.FUNCTION_SET_VOLTAGE_CALLBACK_PERIOD] = BrickletAnalogIn.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletAnalogIn.FUNCTION_GET_VOLTAGE_CALLBACK_PERIOD] = BrickletAnalogIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogIn.FUNCTION_SET_ANALOG_VALUE_CALLBACK_PERIOD] = BrickletAnalogIn.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletAnalogIn.FUNCTION_GET_ANALOG_VALUE_CALLBACK_PERIOD] = BrickletAnalogIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogIn.FUNCTION_SET_VOLTAGE_CALLBACK_THRESHOLD] = BrickletAnalogIn.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletAnalogIn.FUNCTION_GET_VOLTAGE_CALLBACK_THRESHOLD] = BrickletAnalogIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogIn.FUNCTION_SET_ANALOG_VALUE_CALLBACK_THRESHOLD] = BrickletAnalogIn.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletAnalogIn.FUNCTION_GET_ANALOG_VALUE_CALLBACK_THRESHOLD] = BrickletAnalogIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogIn.FUNCTION_SET_DEBOUNCE_PERIOD] = BrickletAnalogIn.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletAnalogIn.FUNCTION_GET_DEBOUNCE_PERIOD] = BrickletAnalogIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogIn.FUNCTION_SET_RANGE] = BrickletAnalogIn.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAnalogIn.FUNCTION_GET_RANGE] = BrickletAnalogIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogIn.FUNCTION_SET_AVERAGING] = BrickletAnalogIn.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAnalogIn.FUNCTION_GET_AVERAGING] = BrickletAnalogIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogIn.FUNCTION_GET_IDENTITY] = BrickletAnalogIn.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletAnalogIn.CALLBACK_VOLTAGE] = (10, 'H')
        self.callback_formats[BrickletAnalogIn.CALLBACK_ANALOG_VALUE] = (10, 'H')
        self.callback_formats[BrickletAnalogIn.CALLBACK_VOLTAGE_REACHED] = (10, 'H')
        self.callback_formats[BrickletAnalogIn.CALLBACK_ANALOG_VALUE_REACHED] = (10, 'H')

        ipcon.add_device(self)

    def get_voltage(self):
        r"""
        Returns the voltage of the sensor. The resolution between 0 and 6V is about 2mV.
        Between 6 and 45V the resolution is about 10mV.

        If you want to get the voltage periodically, it is recommended to use the
        :cb:`Voltage` callback and set the period with
        :func:`Set Voltage Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAnalogIn.FUNCTION_GET_VOLTAGE, (), '', 10, 'H')

    def get_analog_value(self):
        r"""
        Returns the value as read by a 12-bit analog-to-digital converter.

        .. note::
         The value returned by :func:`Get Voltage` is averaged over several samples
         to yield less noise, while :func:`Get Analog Value` gives back raw
         unfiltered analog values. The only reason to use :func:`Get Analog Value` is,
         if you need the full resolution of the analog-to-digital converter.

        If you want the analog value periodically, it is recommended to use the
        :cb:`Analog Value` callback and set the period with
        :func:`Set Analog Value Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAnalogIn.FUNCTION_GET_ANALOG_VALUE, (), '', 10, 'H')

    def set_voltage_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Voltage` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Voltage` callback is only triggered if the voltage has changed since
        the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletAnalogIn.FUNCTION_SET_VOLTAGE_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_voltage_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Voltage Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAnalogIn.FUNCTION_GET_VOLTAGE_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_analog_value_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Analog Value` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Analog Value` callback is only triggered if the analog value has
        changed since the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletAnalogIn.FUNCTION_SET_ANALOG_VALUE_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_analog_value_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Analog Value Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAnalogIn.FUNCTION_GET_ANALOG_VALUE_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_voltage_callback_threshold(self, option, min, max):
        r"""
        Sets the thresholds for the :cb:`Voltage Reached` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Callback is turned off"
         "'o'",    "Callback is triggered when the voltage is *outside* the min and max values"
         "'i'",    "Callback is triggered when the voltage is *inside* the min and max values"
         "'<'",    "Callback is triggered when the voltage is smaller than the min value (max is ignored)"
         "'>'",    "Callback is triggered when the voltage is greater than the min value (max is ignored)"
        """
        self.check_validity()

        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickletAnalogIn.FUNCTION_SET_VOLTAGE_CALLBACK_THRESHOLD, (option, min, max), 'c H H', 0, '')

    def get_voltage_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Voltage Callback Threshold`.
        """
        self.check_validity()

        return GetVoltageCallbackThreshold(*self.ipcon.send_request(self, BrickletAnalogIn.FUNCTION_GET_VOLTAGE_CALLBACK_THRESHOLD, (), '', 13, 'c H H'))

    def set_analog_value_callback_threshold(self, option, min, max):
        r"""
        Sets the thresholds for the :cb:`Analog Value Reached` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Callback is turned off"
         "'o'",    "Callback is triggered when the analog value is *outside* the min and max values"
         "'i'",    "Callback is triggered when the analog value is *inside* the min and max values"
         "'<'",    "Callback is triggered when the analog value is smaller than the min value (max is ignored)"
         "'>'",    "Callback is triggered when the analog value is greater than the min value (max is ignored)"
        """
        self.check_validity()

        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickletAnalogIn.FUNCTION_SET_ANALOG_VALUE_CALLBACK_THRESHOLD, (option, min, max), 'c H H', 0, '')

    def get_analog_value_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Analog Value Callback Threshold`.
        """
        self.check_validity()

        return GetAnalogValueCallbackThreshold(*self.ipcon.send_request(self, BrickletAnalogIn.FUNCTION_GET_ANALOG_VALUE_CALLBACK_THRESHOLD, (), '', 13, 'c H H'))

    def set_debounce_period(self, debounce):
        r"""
        Sets the period with which the threshold callbacks

        * :cb:`Voltage Reached`,
        * :cb:`Analog Value Reached`

        are triggered, if the thresholds

        * :func:`Set Voltage Callback Threshold`,
        * :func:`Set Analog Value Callback Threshold`

        keep being reached.
        """
        self.check_validity()

        debounce = int(debounce)

        self.ipcon.send_request(self, BrickletAnalogIn.FUNCTION_SET_DEBOUNCE_PERIOD, (debounce,), 'I', 0, '')

    def get_debounce_period(self):
        r"""
        Returns the debounce period as set by :func:`Set Debounce Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAnalogIn.FUNCTION_GET_DEBOUNCE_PERIOD, (), '', 12, 'I')

    def set_range(self, range):
        r"""
        Sets the measurement range. Possible ranges:

        * 0: Automatically switched
        * 1: 0V - 6.05V, ~1.48mV resolution
        * 2: 0V - 10.32V, ~2.52mV resolution
        * 3: 0V - 36.30V, ~8.86mV resolution
        * 4: 0V - 45.00V, ~11.25mV resolution
        * 5: 0V - 3.3V, ~0.81mV resolution, new in version 2.0.3$nbsp;(Plugin)

        .. versionadded:: 2.0.1$nbsp;(Plugin)
        """
        self.check_validity()

        range = int(range)

        self.ipcon.send_request(self, BrickletAnalogIn.FUNCTION_SET_RANGE, (range,), 'B', 0, '')

    def get_range(self):
        r"""
        Returns the measurement range as set by :func:`Set Range`.

        .. versionadded:: 2.0.1$nbsp;(Plugin)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAnalogIn.FUNCTION_GET_RANGE, (), '', 9, 'B')

    def set_averaging(self, average):
        r"""
        Set the length of a averaging for the voltage value.

        Setting the length to 0 will turn the averaging completely off. If the
        averaging is off, there is more noise on the data, but the data is without
        delay.

        .. versionadded:: 2.0.3$nbsp;(Plugin)
        """
        self.check_validity()

        average = int(average)

        self.ipcon.send_request(self, BrickletAnalogIn.FUNCTION_SET_AVERAGING, (average,), 'B', 0, '')

    def get_averaging(self):
        r"""
        Returns the averaging configuration as set by :func:`Set Averaging`.

        .. versionadded:: 2.0.3$nbsp;(Plugin)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAnalogIn.FUNCTION_GET_AVERAGING, (), '', 9, 'B')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletAnalogIn.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

AnalogIn = BrickletAnalogIn # for backward compatibility
