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

GetPosition = namedtuple('Position', ['x', 'y'])
GetAnalogValue = namedtuple('AnalogValue', ['x', 'y'])
GetPositionCallbackThreshold = namedtuple('PositionCallbackThreshold', ['option', 'min_x', 'max_x', 'min_y', 'max_y'])
GetAnalogValueCallbackThreshold = namedtuple('AnalogValueCallbackThreshold', ['option', 'min_x', 'max_x', 'min_y', 'max_y'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletJoystick(Device):
    r"""
    2-axis joystick with push-button
    """

    DEVICE_IDENTIFIER = 210
    DEVICE_DISPLAY_NAME = 'Joystick Bricklet'
    DEVICE_URL_PART = 'joystick' # internal

    CALLBACK_POSITION = 15
    CALLBACK_ANALOG_VALUE = 16
    CALLBACK_POSITION_REACHED = 17
    CALLBACK_ANALOG_VALUE_REACHED = 18
    CALLBACK_PRESSED = 19
    CALLBACK_RELEASED = 20


    FUNCTION_GET_POSITION = 1
    FUNCTION_IS_PRESSED = 2
    FUNCTION_GET_ANALOG_VALUE = 3
    FUNCTION_CALIBRATE = 4
    FUNCTION_SET_POSITION_CALLBACK_PERIOD = 5
    FUNCTION_GET_POSITION_CALLBACK_PERIOD = 6
    FUNCTION_SET_ANALOG_VALUE_CALLBACK_PERIOD = 7
    FUNCTION_GET_ANALOG_VALUE_CALLBACK_PERIOD = 8
    FUNCTION_SET_POSITION_CALLBACK_THRESHOLD = 9
    FUNCTION_GET_POSITION_CALLBACK_THRESHOLD = 10
    FUNCTION_SET_ANALOG_VALUE_CALLBACK_THRESHOLD = 11
    FUNCTION_GET_ANALOG_VALUE_CALLBACK_THRESHOLD = 12
    FUNCTION_SET_DEBOUNCE_PERIOD = 13
    FUNCTION_GET_DEBOUNCE_PERIOD = 14
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
        Device.__init__(self, uid, ipcon, BrickletJoystick.DEVICE_IDENTIFIER, BrickletJoystick.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletJoystick.FUNCTION_GET_POSITION] = BrickletJoystick.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletJoystick.FUNCTION_IS_PRESSED] = BrickletJoystick.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletJoystick.FUNCTION_GET_ANALOG_VALUE] = BrickletJoystick.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletJoystick.FUNCTION_CALIBRATE] = BrickletJoystick.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletJoystick.FUNCTION_SET_POSITION_CALLBACK_PERIOD] = BrickletJoystick.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletJoystick.FUNCTION_GET_POSITION_CALLBACK_PERIOD] = BrickletJoystick.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletJoystick.FUNCTION_SET_ANALOG_VALUE_CALLBACK_PERIOD] = BrickletJoystick.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletJoystick.FUNCTION_GET_ANALOG_VALUE_CALLBACK_PERIOD] = BrickletJoystick.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletJoystick.FUNCTION_SET_POSITION_CALLBACK_THRESHOLD] = BrickletJoystick.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletJoystick.FUNCTION_GET_POSITION_CALLBACK_THRESHOLD] = BrickletJoystick.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletJoystick.FUNCTION_SET_ANALOG_VALUE_CALLBACK_THRESHOLD] = BrickletJoystick.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletJoystick.FUNCTION_GET_ANALOG_VALUE_CALLBACK_THRESHOLD] = BrickletJoystick.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletJoystick.FUNCTION_SET_DEBOUNCE_PERIOD] = BrickletJoystick.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletJoystick.FUNCTION_GET_DEBOUNCE_PERIOD] = BrickletJoystick.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletJoystick.FUNCTION_GET_IDENTITY] = BrickletJoystick.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletJoystick.CALLBACK_POSITION] = (12, 'h h')
        self.callback_formats[BrickletJoystick.CALLBACK_ANALOG_VALUE] = (12, 'H H')
        self.callback_formats[BrickletJoystick.CALLBACK_POSITION_REACHED] = (12, 'h h')
        self.callback_formats[BrickletJoystick.CALLBACK_ANALOG_VALUE_REACHED] = (12, 'H H')
        self.callback_formats[BrickletJoystick.CALLBACK_PRESSED] = (8, '')
        self.callback_formats[BrickletJoystick.CALLBACK_RELEASED] = (8, '')

        ipcon.add_device(self)

    def get_position(self):
        r"""
        Returns the position of the joystick. The middle position of the joystick is x=0, y=0.
        The returned values are averaged and calibrated (see :func:`Calibrate`).

        If you want to get the position periodically, it is recommended to use the
        :cb:`Position` callback and set the period with
        :func:`Set Position Callback Period`.
        """
        self.check_validity()

        return GetPosition(*self.ipcon.send_request(self, BrickletJoystick.FUNCTION_GET_POSITION, (), '', 12, 'h h'))

    def is_pressed(self):
        r"""
        Returns *true* if the button is pressed and *false* otherwise.

        It is recommended to use the :cb:`Pressed` and :cb:`Released` callbacks
        to handle the button.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletJoystick.FUNCTION_IS_PRESSED, (), '', 9, '!')

    def get_analog_value(self):
        r"""
        Returns the values as read by a 12-bit analog-to-digital converter.

        .. note::
         The values returned by :func:`Get Position` are averaged over several samples
         to yield less noise, while :func:`Get Analog Value` gives back raw
         unfiltered analog values. The only reason to use :func:`Get Analog Value` is,
         if you need the full resolution of the analog-to-digital converter.

        If you want the analog values periodically, it is recommended to use the
        :cb:`Analog Value` callback and set the period with
        :func:`Set Analog Value Callback Period`.
        """
        self.check_validity()

        return GetAnalogValue(*self.ipcon.send_request(self, BrickletJoystick.FUNCTION_GET_ANALOG_VALUE, (), '', 12, 'H H'))

    def calibrate(self):
        r"""
        Calibrates the middle position of the joystick. If your Joystick Bricklet
        does not return x=0 and y=0 in the middle position, call this function
        while the joystick is standing still in the middle position.

        The resulting calibration will be saved on the EEPROM of the Joystick
        Bricklet, thus you only have to calibrate it once.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletJoystick.FUNCTION_CALIBRATE, (), '', 0, '')

    def set_position_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Position` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Position` callback is only triggered if the position has changed since the
        last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletJoystick.FUNCTION_SET_POSITION_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_position_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Position Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletJoystick.FUNCTION_GET_POSITION_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_analog_value_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Analog Value` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Analog Value` callback is only triggered if the analog values have
        changed since the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletJoystick.FUNCTION_SET_ANALOG_VALUE_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_analog_value_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Analog Value Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletJoystick.FUNCTION_GET_ANALOG_VALUE_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_position_callback_threshold(self, option, min_x, max_x, min_y, max_y):
        r"""
        Sets the thresholds for the :cb:`Position Reached` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Callback is turned off"
         "'o'",    "Callback is triggered when the position is *outside* the min and max values"
         "'i'",    "Callback is triggered when the position is *inside* the min and max values"
         "'<'",    "Callback is triggered when the position is smaller than the min values (max is ignored)"
         "'>'",    "Callback is triggered when the position is greater than the min values (max is ignored)"
        """
        self.check_validity()

        option = create_char(option)
        min_x = int(min_x)
        max_x = int(max_x)
        min_y = int(min_y)
        max_y = int(max_y)

        self.ipcon.send_request(self, BrickletJoystick.FUNCTION_SET_POSITION_CALLBACK_THRESHOLD, (option, min_x, max_x, min_y, max_y), 'c h h h h', 0, '')

    def get_position_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Position Callback Threshold`.
        """
        self.check_validity()

        return GetPositionCallbackThreshold(*self.ipcon.send_request(self, BrickletJoystick.FUNCTION_GET_POSITION_CALLBACK_THRESHOLD, (), '', 17, 'c h h h h'))

    def set_analog_value_callback_threshold(self, option, min_x, max_x, min_y, max_y):
        r"""
        Sets the thresholds for the :cb:`Analog Value Reached` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Callback is turned off"
         "'o'",    "Callback is triggered when the analog values are *outside* the min and max values"
         "'i'",    "Callback is triggered when the analog values are *inside* the min and max values"
         "'<'",    "Callback is triggered when the analog values are smaller than the min values (max is ignored)"
         "'>'",    "Callback is triggered when the analog values are greater than the min values (max is ignored)"
        """
        self.check_validity()

        option = create_char(option)
        min_x = int(min_x)
        max_x = int(max_x)
        min_y = int(min_y)
        max_y = int(max_y)

        self.ipcon.send_request(self, BrickletJoystick.FUNCTION_SET_ANALOG_VALUE_CALLBACK_THRESHOLD, (option, min_x, max_x, min_y, max_y), 'c H H H H', 0, '')

    def get_analog_value_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Analog Value Callback Threshold`.
        """
        self.check_validity()

        return GetAnalogValueCallbackThreshold(*self.ipcon.send_request(self, BrickletJoystick.FUNCTION_GET_ANALOG_VALUE_CALLBACK_THRESHOLD, (), '', 17, 'c H H H H'))

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

        self.ipcon.send_request(self, BrickletJoystick.FUNCTION_SET_DEBOUNCE_PERIOD, (debounce,), 'I', 0, '')

    def get_debounce_period(self):
        r"""
        Returns the debounce period as set by :func:`Set Debounce Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletJoystick.FUNCTION_GET_DEBOUNCE_PERIOD, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletJoystick.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

Joystick = BrickletJoystick # for backward compatibility
