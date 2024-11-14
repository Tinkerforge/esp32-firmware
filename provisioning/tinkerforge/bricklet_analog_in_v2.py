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

class BrickletAnalogInV2(Device):
    r"""
    Measures DC voltage between 0V and 42V
    """

    DEVICE_IDENTIFIER = 251
    DEVICE_DISPLAY_NAME = 'Analog In Bricklet 2.0'
    DEVICE_URL_PART = 'analog_in_v2' # internal

    CALLBACK_VOLTAGE = 15
    CALLBACK_ANALOG_VALUE = 16
    CALLBACK_VOLTAGE_REACHED = 17
    CALLBACK_ANALOG_VALUE_REACHED = 18


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
    FUNCTION_SET_MOVING_AVERAGE = 13
    FUNCTION_GET_MOVING_AVERAGE = 14
    FUNCTION_GET_IDENTITY = 255

    THRESHOLD_OPTION_OFF = 'x'
    THRESHOLD_OPTION_OUTSIDE = 'o'
    THRESHOLD_OPTION_INSIDE = 'i'
    THRESHOLD_OPTION_SMALLER = '<'
    THRESHOLD_OPTION_GREATER = '>'

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletAnalogInV2.DEVICE_IDENTIFIER, BrickletAnalogInV2.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 1)

        self.response_expected[BrickletAnalogInV2.FUNCTION_GET_VOLTAGE] = BrickletAnalogInV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogInV2.FUNCTION_GET_ANALOG_VALUE] = BrickletAnalogInV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogInV2.FUNCTION_SET_VOLTAGE_CALLBACK_PERIOD] = BrickletAnalogInV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletAnalogInV2.FUNCTION_GET_VOLTAGE_CALLBACK_PERIOD] = BrickletAnalogInV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogInV2.FUNCTION_SET_ANALOG_VALUE_CALLBACK_PERIOD] = BrickletAnalogInV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletAnalogInV2.FUNCTION_GET_ANALOG_VALUE_CALLBACK_PERIOD] = BrickletAnalogInV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogInV2.FUNCTION_SET_VOLTAGE_CALLBACK_THRESHOLD] = BrickletAnalogInV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletAnalogInV2.FUNCTION_GET_VOLTAGE_CALLBACK_THRESHOLD] = BrickletAnalogInV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogInV2.FUNCTION_SET_ANALOG_VALUE_CALLBACK_THRESHOLD] = BrickletAnalogInV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletAnalogInV2.FUNCTION_GET_ANALOG_VALUE_CALLBACK_THRESHOLD] = BrickletAnalogInV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogInV2.FUNCTION_SET_DEBOUNCE_PERIOD] = BrickletAnalogInV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletAnalogInV2.FUNCTION_GET_DEBOUNCE_PERIOD] = BrickletAnalogInV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogInV2.FUNCTION_SET_MOVING_AVERAGE] = BrickletAnalogInV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAnalogInV2.FUNCTION_GET_MOVING_AVERAGE] = BrickletAnalogInV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogInV2.FUNCTION_GET_IDENTITY] = BrickletAnalogInV2.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletAnalogInV2.CALLBACK_VOLTAGE] = (10, 'H')
        self.callback_formats[BrickletAnalogInV2.CALLBACK_ANALOG_VALUE] = (10, 'H')
        self.callback_formats[BrickletAnalogInV2.CALLBACK_VOLTAGE_REACHED] = (10, 'H')
        self.callback_formats[BrickletAnalogInV2.CALLBACK_ANALOG_VALUE_REACHED] = (10, 'H')

        ipcon.add_device(self)

    def get_voltage(self):
        r"""
        Returns the measured voltage. The resolution is approximately 10mV.

        If you want to get the voltage periodically, it is recommended to use the
        :cb:`Voltage` callback and set the period with
        :func:`Set Voltage Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAnalogInV2.FUNCTION_GET_VOLTAGE, (), '', 10, 'H')

    def get_analog_value(self):
        r"""
        Returns the value as read by a 12-bit analog-to-digital converter.

        If you want the analog value periodically, it is recommended to use the
        :cb:`Analog Value` callback and set the period with
        :func:`Set Analog Value Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAnalogInV2.FUNCTION_GET_ANALOG_VALUE, (), '', 10, 'H')

    def set_voltage_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Voltage` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Voltage` callback is only triggered if the voltage has changed since
        the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletAnalogInV2.FUNCTION_SET_VOLTAGE_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_voltage_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Voltage Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAnalogInV2.FUNCTION_GET_VOLTAGE_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_analog_value_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Analog Value` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Analog Value` callback is only triggered if the analog value has
        changed since the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletAnalogInV2.FUNCTION_SET_ANALOG_VALUE_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_analog_value_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Analog Value Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAnalogInV2.FUNCTION_GET_ANALOG_VALUE_CALLBACK_PERIOD, (), '', 12, 'I')

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

        self.ipcon.send_request(self, BrickletAnalogInV2.FUNCTION_SET_VOLTAGE_CALLBACK_THRESHOLD, (option, min, max), 'c H H', 0, '')

    def get_voltage_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Voltage Callback Threshold`.
        """
        self.check_validity()

        return GetVoltageCallbackThreshold(*self.ipcon.send_request(self, BrickletAnalogInV2.FUNCTION_GET_VOLTAGE_CALLBACK_THRESHOLD, (), '', 13, 'c H H'))

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

        self.ipcon.send_request(self, BrickletAnalogInV2.FUNCTION_SET_ANALOG_VALUE_CALLBACK_THRESHOLD, (option, min, max), 'c H H', 0, '')

    def get_analog_value_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Analog Value Callback Threshold`.
        """
        self.check_validity()

        return GetAnalogValueCallbackThreshold(*self.ipcon.send_request(self, BrickletAnalogInV2.FUNCTION_GET_ANALOG_VALUE_CALLBACK_THRESHOLD, (), '', 13, 'c H H'))

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

        self.ipcon.send_request(self, BrickletAnalogInV2.FUNCTION_SET_DEBOUNCE_PERIOD, (debounce,), 'I', 0, '')

    def get_debounce_period(self):
        r"""
        Returns the debounce period as set by :func:`Set Debounce Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAnalogInV2.FUNCTION_GET_DEBOUNCE_PERIOD, (), '', 12, 'I')

    def set_moving_average(self, average):
        r"""
        Sets the length of a `moving averaging <https://en.wikipedia.org/wiki/Moving_average>`__
        for the voltage.

        Setting the length to 1 will turn the averaging off. With less
        averaging, there is more noise on the data.
        """
        self.check_validity()

        average = int(average)

        self.ipcon.send_request(self, BrickletAnalogInV2.FUNCTION_SET_MOVING_AVERAGE, (average,), 'B', 0, '')

    def get_moving_average(self):
        r"""
        Returns the length of the moving average as set by :func:`Set Moving Average`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAnalogInV2.FUNCTION_GET_MOVING_AVERAGE, (), '', 9, 'B')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletAnalogInV2.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

AnalogInV2 = BrickletAnalogInV2 # for backward compatibility
