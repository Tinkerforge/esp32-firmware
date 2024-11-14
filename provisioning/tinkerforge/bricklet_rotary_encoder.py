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

GetCountCallbackThreshold = namedtuple('CountCallbackThreshold', ['option', 'min', 'max'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletRotaryEncoder(Device):
    r"""
    360° rotary encoder with push-button
    """

    DEVICE_IDENTIFIER = 236
    DEVICE_DISPLAY_NAME = 'Rotary Encoder Bricklet'
    DEVICE_URL_PART = 'rotary_encoder' # internal

    CALLBACK_COUNT = 8
    CALLBACK_COUNT_REACHED = 9
    CALLBACK_PRESSED = 11
    CALLBACK_RELEASED = 12


    FUNCTION_GET_COUNT = 1
    FUNCTION_SET_COUNT_CALLBACK_PERIOD = 2
    FUNCTION_GET_COUNT_CALLBACK_PERIOD = 3
    FUNCTION_SET_COUNT_CALLBACK_THRESHOLD = 4
    FUNCTION_GET_COUNT_CALLBACK_THRESHOLD = 5
    FUNCTION_SET_DEBOUNCE_PERIOD = 6
    FUNCTION_GET_DEBOUNCE_PERIOD = 7
    FUNCTION_IS_PRESSED = 10
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
        Device.__init__(self, uid, ipcon, BrickletRotaryEncoder.DEVICE_IDENTIFIER, BrickletRotaryEncoder.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletRotaryEncoder.FUNCTION_GET_COUNT] = BrickletRotaryEncoder.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRotaryEncoder.FUNCTION_SET_COUNT_CALLBACK_PERIOD] = BrickletRotaryEncoder.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletRotaryEncoder.FUNCTION_GET_COUNT_CALLBACK_PERIOD] = BrickletRotaryEncoder.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRotaryEncoder.FUNCTION_SET_COUNT_CALLBACK_THRESHOLD] = BrickletRotaryEncoder.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletRotaryEncoder.FUNCTION_GET_COUNT_CALLBACK_THRESHOLD] = BrickletRotaryEncoder.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRotaryEncoder.FUNCTION_SET_DEBOUNCE_PERIOD] = BrickletRotaryEncoder.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletRotaryEncoder.FUNCTION_GET_DEBOUNCE_PERIOD] = BrickletRotaryEncoder.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRotaryEncoder.FUNCTION_IS_PRESSED] = BrickletRotaryEncoder.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRotaryEncoder.FUNCTION_GET_IDENTITY] = BrickletRotaryEncoder.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletRotaryEncoder.CALLBACK_COUNT] = (12, 'i')
        self.callback_formats[BrickletRotaryEncoder.CALLBACK_COUNT_REACHED] = (12, 'i')
        self.callback_formats[BrickletRotaryEncoder.CALLBACK_PRESSED] = (8, '')
        self.callback_formats[BrickletRotaryEncoder.CALLBACK_RELEASED] = (8, '')

        ipcon.add_device(self)

    def get_count(self, reset):
        r"""
        Returns the current count of the encoder. If you set reset
        to true, the count is set back to 0 directly after the
        current count is read.

        The encoder has 24 steps per rotation

        Turning the encoder to the left decrements the counter,
        so a negative count is possible.
        """
        self.check_validity()

        reset = bool(reset)

        return self.ipcon.send_request(self, BrickletRotaryEncoder.FUNCTION_GET_COUNT, (reset,), '!', 12, 'i')

    def set_count_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Count` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Count` callback is only triggered if the count has changed since the
        last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletRotaryEncoder.FUNCTION_SET_COUNT_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_count_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Count Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRotaryEncoder.FUNCTION_GET_COUNT_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_count_callback_threshold(self, option, min, max):
        r"""
        Sets the thresholds for the :cb:`Count Reached` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Callback is turned off"
         "'o'",    "Callback is triggered when the count is *outside* the min and max values"
         "'i'",    "Callback is triggered when the count is *inside* the min and max values"
         "'<'",    "Callback is triggered when the count is smaller than the min value (max is ignored)"
         "'>'",    "Callback is triggered when the count is greater than the min value (max is ignored)"
        """
        self.check_validity()

        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickletRotaryEncoder.FUNCTION_SET_COUNT_CALLBACK_THRESHOLD, (option, min, max), 'c i i', 0, '')

    def get_count_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Count Callback Threshold`.
        """
        self.check_validity()

        return GetCountCallbackThreshold(*self.ipcon.send_request(self, BrickletRotaryEncoder.FUNCTION_GET_COUNT_CALLBACK_THRESHOLD, (), '', 17, 'c i i'))

    def set_debounce_period(self, debounce):
        r"""
        Sets the period with which the threshold callback

        * :cb:`Count Reached`

        is triggered, if the thresholds

        * :func:`Set Count Callback Threshold`

        keeps being reached.
        """
        self.check_validity()

        debounce = int(debounce)

        self.ipcon.send_request(self, BrickletRotaryEncoder.FUNCTION_SET_DEBOUNCE_PERIOD, (debounce,), 'I', 0, '')

    def get_debounce_period(self):
        r"""
        Returns the debounce period as set by :func:`Set Debounce Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRotaryEncoder.FUNCTION_GET_DEBOUNCE_PERIOD, (), '', 12, 'I')

    def is_pressed(self):
        r"""
        Returns *true* if the button is pressed and *false* otherwise.

        It is recommended to use the :cb:`Pressed` and :cb:`Released` callbacks
        to handle the button.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRotaryEncoder.FUNCTION_IS_PRESSED, (), '', 9, '!')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletRotaryEncoder.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

RotaryEncoder = BrickletRotaryEncoder # for backward compatibility
