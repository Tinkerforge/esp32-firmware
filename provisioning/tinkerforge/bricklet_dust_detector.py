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

GetDustDensityCallbackThreshold = namedtuple('DustDensityCallbackThreshold', ['option', 'min', 'max'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletDustDetector(Device):
    r"""
    Measures dust density
    """

    DEVICE_IDENTIFIER = 260
    DEVICE_DISPLAY_NAME = 'Dust Detector Bricklet'
    DEVICE_URL_PART = 'dust_detector' # internal

    CALLBACK_DUST_DENSITY = 8
    CALLBACK_DUST_DENSITY_REACHED = 9


    FUNCTION_GET_DUST_DENSITY = 1
    FUNCTION_SET_DUST_DENSITY_CALLBACK_PERIOD = 2
    FUNCTION_GET_DUST_DENSITY_CALLBACK_PERIOD = 3
    FUNCTION_SET_DUST_DENSITY_CALLBACK_THRESHOLD = 4
    FUNCTION_GET_DUST_DENSITY_CALLBACK_THRESHOLD = 5
    FUNCTION_SET_DEBOUNCE_PERIOD = 6
    FUNCTION_GET_DEBOUNCE_PERIOD = 7
    FUNCTION_SET_MOVING_AVERAGE = 10
    FUNCTION_GET_MOVING_AVERAGE = 11
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
        Device.__init__(self, uid, ipcon, BrickletDustDetector.DEVICE_IDENTIFIER, BrickletDustDetector.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletDustDetector.FUNCTION_GET_DUST_DENSITY] = BrickletDustDetector.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDustDetector.FUNCTION_SET_DUST_DENSITY_CALLBACK_PERIOD] = BrickletDustDetector.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletDustDetector.FUNCTION_GET_DUST_DENSITY_CALLBACK_PERIOD] = BrickletDustDetector.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDustDetector.FUNCTION_SET_DUST_DENSITY_CALLBACK_THRESHOLD] = BrickletDustDetector.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletDustDetector.FUNCTION_GET_DUST_DENSITY_CALLBACK_THRESHOLD] = BrickletDustDetector.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDustDetector.FUNCTION_SET_DEBOUNCE_PERIOD] = BrickletDustDetector.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletDustDetector.FUNCTION_GET_DEBOUNCE_PERIOD] = BrickletDustDetector.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDustDetector.FUNCTION_SET_MOVING_AVERAGE] = BrickletDustDetector.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletDustDetector.FUNCTION_GET_MOVING_AVERAGE] = BrickletDustDetector.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDustDetector.FUNCTION_GET_IDENTITY] = BrickletDustDetector.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletDustDetector.CALLBACK_DUST_DENSITY] = (10, 'H')
        self.callback_formats[BrickletDustDetector.CALLBACK_DUST_DENSITY_REACHED] = (10, 'H')

        ipcon.add_device(self)

    def get_dust_density(self):
        r"""
        Returns the dust density.

        If you want to get the dust density periodically, it is recommended
        to use the :cb:`Dust Density` callback and set the period with
        :func:`Set Dust Density Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletDustDetector.FUNCTION_GET_DUST_DENSITY, (), '', 10, 'H')

    def set_dust_density_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Dust Density` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Dust Density` callback is only triggered if the dust density has
        changed since the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletDustDetector.FUNCTION_SET_DUST_DENSITY_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_dust_density_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Dust Density Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletDustDetector.FUNCTION_GET_DUST_DENSITY_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_dust_density_callback_threshold(self, option, min, max):
        r"""
        Sets the thresholds for the :cb:`Dust Density Reached` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Callback is turned off"
         "'o'",    "Callback is triggered when the dust density value is *outside* the min and max values"
         "'i'",    "Callback is triggered when the dust density value is *inside* the min and max values"
         "'<'",    "Callback is triggered when the dust density value is smaller than the min value (max is ignored)"
         "'>'",    "Callback is triggered when the dust density value is greater than the min value (max is ignored)"
        """
        self.check_validity()

        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickletDustDetector.FUNCTION_SET_DUST_DENSITY_CALLBACK_THRESHOLD, (option, min, max), 'c H H', 0, '')

    def get_dust_density_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Dust Density Callback Threshold`.
        """
        self.check_validity()

        return GetDustDensityCallbackThreshold(*self.ipcon.send_request(self, BrickletDustDetector.FUNCTION_GET_DUST_DENSITY_CALLBACK_THRESHOLD, (), '', 13, 'c H H'))

    def set_debounce_period(self, debounce):
        r"""
        Sets the period with which the threshold callback

        * :cb:`Dust Density Reached`

        is triggered, if the threshold

        * :func:`Set Dust Density Callback Threshold`

        keeps being reached.
        """
        self.check_validity()

        debounce = int(debounce)

        self.ipcon.send_request(self, BrickletDustDetector.FUNCTION_SET_DEBOUNCE_PERIOD, (debounce,), 'I', 0, '')

    def get_debounce_period(self):
        r"""
        Returns the debounce period as set by :func:`Set Debounce Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletDustDetector.FUNCTION_GET_DEBOUNCE_PERIOD, (), '', 12, 'I')

    def set_moving_average(self, average):
        r"""
        Sets the length of a `moving averaging <https://en.wikipedia.org/wiki/Moving_average>`__
        for the dust density.

        Setting the length to 0 will turn the averaging completely off. With less
        averaging, there is more noise on the data.
        """
        self.check_validity()

        average = int(average)

        self.ipcon.send_request(self, BrickletDustDetector.FUNCTION_SET_MOVING_AVERAGE, (average,), 'B', 0, '')

    def get_moving_average(self):
        r"""
        Returns the length moving average as set by :func:`Set Moving Average`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletDustDetector.FUNCTION_GET_MOVING_AVERAGE, (), '', 9, 'B')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletDustDetector.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

DustDetector = BrickletDustDetector # for backward compatibility
