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

GetPositionCallbackThreshold = namedtuple('PositionCallbackThreshold', ['option', 'min', 'max'])
GetAnalogValueCallbackThreshold = namedtuple('AnalogValueCallbackThreshold', ['option', 'min', 'max'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletLinearPoti(Device):
    r"""
    59mm linear potentiometer
    """

    DEVICE_IDENTIFIER = 213
    DEVICE_DISPLAY_NAME = 'Linear Poti Bricklet'
    DEVICE_URL_PART = 'linear_poti' # internal

    CALLBACK_POSITION = 13
    CALLBACK_ANALOG_VALUE = 14
    CALLBACK_POSITION_REACHED = 15
    CALLBACK_ANALOG_VALUE_REACHED = 16


    FUNCTION_GET_POSITION = 1
    FUNCTION_GET_ANALOG_VALUE = 2
    FUNCTION_SET_POSITION_CALLBACK_PERIOD = 3
    FUNCTION_GET_POSITION_CALLBACK_PERIOD = 4
    FUNCTION_SET_ANALOG_VALUE_CALLBACK_PERIOD = 5
    FUNCTION_GET_ANALOG_VALUE_CALLBACK_PERIOD = 6
    FUNCTION_SET_POSITION_CALLBACK_THRESHOLD = 7
    FUNCTION_GET_POSITION_CALLBACK_THRESHOLD = 8
    FUNCTION_SET_ANALOG_VALUE_CALLBACK_THRESHOLD = 9
    FUNCTION_GET_ANALOG_VALUE_CALLBACK_THRESHOLD = 10
    FUNCTION_SET_DEBOUNCE_PERIOD = 11
    FUNCTION_GET_DEBOUNCE_PERIOD = 12
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
        Device.__init__(self, uid, ipcon, BrickletLinearPoti.DEVICE_IDENTIFIER, BrickletLinearPoti.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 1)

        self.response_expected[BrickletLinearPoti.FUNCTION_GET_POSITION] = BrickletLinearPoti.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLinearPoti.FUNCTION_GET_ANALOG_VALUE] = BrickletLinearPoti.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLinearPoti.FUNCTION_SET_POSITION_CALLBACK_PERIOD] = BrickletLinearPoti.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletLinearPoti.FUNCTION_GET_POSITION_CALLBACK_PERIOD] = BrickletLinearPoti.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLinearPoti.FUNCTION_SET_ANALOG_VALUE_CALLBACK_PERIOD] = BrickletLinearPoti.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletLinearPoti.FUNCTION_GET_ANALOG_VALUE_CALLBACK_PERIOD] = BrickletLinearPoti.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLinearPoti.FUNCTION_SET_POSITION_CALLBACK_THRESHOLD] = BrickletLinearPoti.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletLinearPoti.FUNCTION_GET_POSITION_CALLBACK_THRESHOLD] = BrickletLinearPoti.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLinearPoti.FUNCTION_SET_ANALOG_VALUE_CALLBACK_THRESHOLD] = BrickletLinearPoti.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletLinearPoti.FUNCTION_GET_ANALOG_VALUE_CALLBACK_THRESHOLD] = BrickletLinearPoti.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLinearPoti.FUNCTION_SET_DEBOUNCE_PERIOD] = BrickletLinearPoti.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletLinearPoti.FUNCTION_GET_DEBOUNCE_PERIOD] = BrickletLinearPoti.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLinearPoti.FUNCTION_GET_IDENTITY] = BrickletLinearPoti.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletLinearPoti.CALLBACK_POSITION] = (10, 'H')
        self.callback_formats[BrickletLinearPoti.CALLBACK_ANALOG_VALUE] = (10, 'H')
        self.callback_formats[BrickletLinearPoti.CALLBACK_POSITION_REACHED] = (10, 'H')
        self.callback_formats[BrickletLinearPoti.CALLBACK_ANALOG_VALUE_REACHED] = (10, 'H')

        ipcon.add_device(self)

    def get_position(self):
        r"""
        Returns the position of the linear potentiometer. The value is
        between 0% (slider down) and 100% (slider up).

        If you want to get the position periodically, it is recommended to use the
        :cb:`Position` callback and set the period with
        :func:`Set Position Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLinearPoti.FUNCTION_GET_POSITION, (), '', 10, 'H')

    def get_analog_value(self):
        r"""
        Returns the value as read by a 12-bit analog-to-digital converter.

        .. note::
         The value returned by :func:`Get Position` is averaged over several samples
         to yield less noise, while :func:`Get Analog Value` gives back raw
         unfiltered analog values. The only reason to use :func:`Get Analog Value` is,
         if you need the full resolution of the analog-to-digital converter.

        If you want the analog value periodically, it is recommended to use the
        :cb:`Analog Value` callback and set the period with
        :func:`Set Analog Value Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLinearPoti.FUNCTION_GET_ANALOG_VALUE, (), '', 10, 'H')

    def set_position_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Position` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Position` callback is only triggered if the position has changed
        since the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletLinearPoti.FUNCTION_SET_POSITION_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_position_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Position Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLinearPoti.FUNCTION_GET_POSITION_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_analog_value_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Analog Value` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Analog Value` callback is only triggered if the analog value has
        changed since the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletLinearPoti.FUNCTION_SET_ANALOG_VALUE_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_analog_value_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Analog Value Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLinearPoti.FUNCTION_GET_ANALOG_VALUE_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_position_callback_threshold(self, option, min, max):
        r"""
        Sets the thresholds for the :cb:`Position Reached` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Callback is turned off"
         "'o'",    "Callback is triggered when the position is *outside* the min and max values"
         "'i'",    "Callback is triggered when the position is *inside* the min and max values"
         "'<'",    "Callback is triggered when the position is smaller than the min value (max is ignored)"
         "'>'",    "Callback is triggered when the position is greater than the min value (max is ignored)"
        """
        self.check_validity()

        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickletLinearPoti.FUNCTION_SET_POSITION_CALLBACK_THRESHOLD, (option, min, max), 'c H H', 0, '')

    def get_position_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Position Callback Threshold`.
        """
        self.check_validity()

        return GetPositionCallbackThreshold(*self.ipcon.send_request(self, BrickletLinearPoti.FUNCTION_GET_POSITION_CALLBACK_THRESHOLD, (), '', 13, 'c H H'))

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

        self.ipcon.send_request(self, BrickletLinearPoti.FUNCTION_SET_ANALOG_VALUE_CALLBACK_THRESHOLD, (option, min, max), 'c H H', 0, '')

    def get_analog_value_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Analog Value Callback Threshold`.
        """
        self.check_validity()

        return GetAnalogValueCallbackThreshold(*self.ipcon.send_request(self, BrickletLinearPoti.FUNCTION_GET_ANALOG_VALUE_CALLBACK_THRESHOLD, (), '', 13, 'c H H'))

    def set_debounce_period(self, debounce):
        r"""
        Sets the period with which the threshold callbacks

        * :cb:`Position Reached`,
        * :cb:`Analog Value Reached`

        are triggered, if the thresholds

        * :func:`Set Position Callback Threshold`,
        * :func:`Set Analog Value Callback Threshold`

        keep being reached.
        """
        self.check_validity()

        debounce = int(debounce)

        self.ipcon.send_request(self, BrickletLinearPoti.FUNCTION_SET_DEBOUNCE_PERIOD, (debounce,), 'I', 0, '')

    def get_debounce_period(self):
        r"""
        Returns the debounce period as set by :func:`Set Debounce Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLinearPoti.FUNCTION_GET_DEBOUNCE_PERIOD, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletLinearPoti.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

LinearPoti = BrickletLinearPoti # for backward compatibility
