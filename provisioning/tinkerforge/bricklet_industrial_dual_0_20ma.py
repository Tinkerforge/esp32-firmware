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

GetCurrentCallbackThreshold = namedtuple('CurrentCallbackThreshold', ['option', 'min', 'max'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletIndustrialDual020mA(Device):
    r"""
    Measures two DC currents between 0mA and 20mA (IEC 60381-1)
    """

    DEVICE_IDENTIFIER = 228
    DEVICE_DISPLAY_NAME = 'Industrial Dual 0-20mA Bricklet'
    DEVICE_URL_PART = 'industrial_dual_0_20ma' # internal

    CALLBACK_CURRENT = 10
    CALLBACK_CURRENT_REACHED = 11


    FUNCTION_GET_CURRENT = 1
    FUNCTION_SET_CURRENT_CALLBACK_PERIOD = 2
    FUNCTION_GET_CURRENT_CALLBACK_PERIOD = 3
    FUNCTION_SET_CURRENT_CALLBACK_THRESHOLD = 4
    FUNCTION_GET_CURRENT_CALLBACK_THRESHOLD = 5
    FUNCTION_SET_DEBOUNCE_PERIOD = 6
    FUNCTION_GET_DEBOUNCE_PERIOD = 7
    FUNCTION_SET_SAMPLE_RATE = 8
    FUNCTION_GET_SAMPLE_RATE = 9
    FUNCTION_GET_IDENTITY = 255

    THRESHOLD_OPTION_OFF = 'x'
    THRESHOLD_OPTION_OUTSIDE = 'o'
    THRESHOLD_OPTION_INSIDE = 'i'
    THRESHOLD_OPTION_SMALLER = '<'
    THRESHOLD_OPTION_GREATER = '>'
    SAMPLE_RATE_240_SPS = 0
    SAMPLE_RATE_60_SPS = 1
    SAMPLE_RATE_15_SPS = 2
    SAMPLE_RATE_4_SPS = 3

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletIndustrialDual020mA.DEVICE_IDENTIFIER, BrickletIndustrialDual020mA.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletIndustrialDual020mA.FUNCTION_GET_CURRENT] = BrickletIndustrialDual020mA.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDual020mA.FUNCTION_SET_CURRENT_CALLBACK_PERIOD] = BrickletIndustrialDual020mA.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIndustrialDual020mA.FUNCTION_GET_CURRENT_CALLBACK_PERIOD] = BrickletIndustrialDual020mA.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDual020mA.FUNCTION_SET_CURRENT_CALLBACK_THRESHOLD] = BrickletIndustrialDual020mA.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIndustrialDual020mA.FUNCTION_GET_CURRENT_CALLBACK_THRESHOLD] = BrickletIndustrialDual020mA.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDual020mA.FUNCTION_SET_DEBOUNCE_PERIOD] = BrickletIndustrialDual020mA.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIndustrialDual020mA.FUNCTION_GET_DEBOUNCE_PERIOD] = BrickletIndustrialDual020mA.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDual020mA.FUNCTION_SET_SAMPLE_RATE] = BrickletIndustrialDual020mA.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDual020mA.FUNCTION_GET_SAMPLE_RATE] = BrickletIndustrialDual020mA.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDual020mA.FUNCTION_GET_IDENTITY] = BrickletIndustrialDual020mA.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletIndustrialDual020mA.CALLBACK_CURRENT] = (13, 'B i')
        self.callback_formats[BrickletIndustrialDual020mA.CALLBACK_CURRENT_REACHED] = (13, 'B i')

        ipcon.add_device(self)

    def get_current(self, sensor):
        r"""
        Returns the current of the specified sensor.

        It is possible to detect if an IEC 60381-1 compatible sensor is connected
        and if it works properly.

        If the returned current is below 4mA, there is likely no sensor connected
        or the sensor may be defect. If the returned current is over 20mA, there might
        be a short circuit or the sensor may be defect.

        If you want to get the current periodically, it is recommended to use the
        :cb:`Current` callback and set the period with
        :func:`Set Current Callback Period`.
        """
        self.check_validity()

        sensor = int(sensor)

        return self.ipcon.send_request(self, BrickletIndustrialDual020mA.FUNCTION_GET_CURRENT, (sensor,), 'B', 12, 'i')

    def set_current_callback_period(self, sensor, period):
        r"""
        Sets the period with which the :cb:`Current` callback is triggered
        periodically for the given sensor. A value of 0 turns the callback off.

        The :cb:`Current` callback is only triggered if the current has changed since the
        last triggering.
        """
        self.check_validity()

        sensor = int(sensor)
        period = int(period)

        self.ipcon.send_request(self, BrickletIndustrialDual020mA.FUNCTION_SET_CURRENT_CALLBACK_PERIOD, (sensor, period), 'B I', 0, '')

    def get_current_callback_period(self, sensor):
        r"""
        Returns the period as set by :func:`Set Current Callback Period`.
        """
        self.check_validity()

        sensor = int(sensor)

        return self.ipcon.send_request(self, BrickletIndustrialDual020mA.FUNCTION_GET_CURRENT_CALLBACK_PERIOD, (sensor,), 'B', 12, 'I')

    def set_current_callback_threshold(self, sensor, option, min, max):
        r"""
        Sets the thresholds for the :cb:`Current Reached` callback for the given
        sensor.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Callback is turned off"
         "'o'",    "Callback is triggered when the current is *outside* the min and max values"
         "'i'",    "Callback is triggered when the current is *inside* the min and max values"
         "'<'",    "Callback is triggered when the current is smaller than the min value (max is ignored)"
         "'>'",    "Callback is triggered when the current is greater than the min value (max is ignored)"
        """
        self.check_validity()

        sensor = int(sensor)
        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickletIndustrialDual020mA.FUNCTION_SET_CURRENT_CALLBACK_THRESHOLD, (sensor, option, min, max), 'B c i i', 0, '')

    def get_current_callback_threshold(self, sensor):
        r"""
        Returns the threshold as set by :func:`Set Current Callback Threshold`.
        """
        self.check_validity()

        sensor = int(sensor)

        return GetCurrentCallbackThreshold(*self.ipcon.send_request(self, BrickletIndustrialDual020mA.FUNCTION_GET_CURRENT_CALLBACK_THRESHOLD, (sensor,), 'B', 17, 'c i i'))

    def set_debounce_period(self, debounce):
        r"""
        Sets the period with which the threshold callback

        * :cb:`Current Reached`

        is triggered, if the threshold

        * :func:`Set Current Callback Threshold`

        keeps being reached.
        """
        self.check_validity()

        debounce = int(debounce)

        self.ipcon.send_request(self, BrickletIndustrialDual020mA.FUNCTION_SET_DEBOUNCE_PERIOD, (debounce,), 'I', 0, '')

    def get_debounce_period(self):
        r"""
        Returns the debounce period as set by :func:`Set Debounce Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDual020mA.FUNCTION_GET_DEBOUNCE_PERIOD, (), '', 12, 'I')

    def set_sample_rate(self, rate):
        r"""
        Sets the sample rate to either 240, 60, 15 or 4 samples per second.
        The resolution for the rates is 12, 14, 16 and 18 bit respectively.

        .. csv-table::
         :header: "Value", "Description"
         :widths: 10, 100

         "0",    "240 samples per second, 12 bit resolution"
         "1",    "60 samples per second, 14 bit resolution"
         "2",    "15 samples per second, 16 bit resolution"
         "3",    "4 samples per second, 18 bit resolution"
        """
        self.check_validity()

        rate = int(rate)

        self.ipcon.send_request(self, BrickletIndustrialDual020mA.FUNCTION_SET_SAMPLE_RATE, (rate,), 'B', 0, '')

    def get_sample_rate(self):
        r"""
        Returns the sample rate as set by :func:`Set Sample Rate`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDual020mA.FUNCTION_GET_SAMPLE_RATE, (), '', 9, 'B')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletIndustrialDual020mA.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

IndustrialDual020mA = BrickletIndustrialDual020mA # for backward compatibility
