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

GetWeightCallbackThreshold = namedtuple('WeightCallbackThreshold', ['option', 'min', 'max'])
GetConfiguration = namedtuple('Configuration', ['rate', 'gain'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletLoadCell(Device):
    r"""
    Measures weight with a load cell
    """

    DEVICE_IDENTIFIER = 253
    DEVICE_DISPLAY_NAME = 'Load Cell Bricklet'
    DEVICE_URL_PART = 'load_cell' # internal

    CALLBACK_WEIGHT = 17
    CALLBACK_WEIGHT_REACHED = 18


    FUNCTION_GET_WEIGHT = 1
    FUNCTION_SET_WEIGHT_CALLBACK_PERIOD = 2
    FUNCTION_GET_WEIGHT_CALLBACK_PERIOD = 3
    FUNCTION_SET_WEIGHT_CALLBACK_THRESHOLD = 4
    FUNCTION_GET_WEIGHT_CALLBACK_THRESHOLD = 5
    FUNCTION_SET_DEBOUNCE_PERIOD = 6
    FUNCTION_GET_DEBOUNCE_PERIOD = 7
    FUNCTION_SET_MOVING_AVERAGE = 8
    FUNCTION_GET_MOVING_AVERAGE = 9
    FUNCTION_LED_ON = 10
    FUNCTION_LED_OFF = 11
    FUNCTION_IS_LED_ON = 12
    FUNCTION_CALIBRATE = 13
    FUNCTION_TARE = 14
    FUNCTION_SET_CONFIGURATION = 15
    FUNCTION_GET_CONFIGURATION = 16
    FUNCTION_GET_IDENTITY = 255

    THRESHOLD_OPTION_OFF = 'x'
    THRESHOLD_OPTION_OUTSIDE = 'o'
    THRESHOLD_OPTION_INSIDE = 'i'
    THRESHOLD_OPTION_SMALLER = '<'
    THRESHOLD_OPTION_GREATER = '>'
    RATE_10HZ = 0
    RATE_80HZ = 1
    GAIN_128X = 0
    GAIN_64X = 1
    GAIN_32X = 2

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletLoadCell.DEVICE_IDENTIFIER, BrickletLoadCell.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletLoadCell.FUNCTION_GET_WEIGHT] = BrickletLoadCell.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLoadCell.FUNCTION_SET_WEIGHT_CALLBACK_PERIOD] = BrickletLoadCell.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletLoadCell.FUNCTION_GET_WEIGHT_CALLBACK_PERIOD] = BrickletLoadCell.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLoadCell.FUNCTION_SET_WEIGHT_CALLBACK_THRESHOLD] = BrickletLoadCell.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletLoadCell.FUNCTION_GET_WEIGHT_CALLBACK_THRESHOLD] = BrickletLoadCell.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLoadCell.FUNCTION_SET_DEBOUNCE_PERIOD] = BrickletLoadCell.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletLoadCell.FUNCTION_GET_DEBOUNCE_PERIOD] = BrickletLoadCell.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLoadCell.FUNCTION_SET_MOVING_AVERAGE] = BrickletLoadCell.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLoadCell.FUNCTION_GET_MOVING_AVERAGE] = BrickletLoadCell.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLoadCell.FUNCTION_LED_ON] = BrickletLoadCell.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLoadCell.FUNCTION_LED_OFF] = BrickletLoadCell.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLoadCell.FUNCTION_IS_LED_ON] = BrickletLoadCell.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLoadCell.FUNCTION_CALIBRATE] = BrickletLoadCell.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLoadCell.FUNCTION_TARE] = BrickletLoadCell.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLoadCell.FUNCTION_SET_CONFIGURATION] = BrickletLoadCell.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLoadCell.FUNCTION_GET_CONFIGURATION] = BrickletLoadCell.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLoadCell.FUNCTION_GET_IDENTITY] = BrickletLoadCell.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletLoadCell.CALLBACK_WEIGHT] = (12, 'i')
        self.callback_formats[BrickletLoadCell.CALLBACK_WEIGHT_REACHED] = (12, 'i')

        ipcon.add_device(self)

    def get_weight(self):
        r"""
        Returns the currently measured weight.

        If you want to get the weight periodically, it is recommended
        to use the :cb:`Weight` callback and set the period with
        :func:`Set Weight Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLoadCell.FUNCTION_GET_WEIGHT, (), '', 12, 'i')

    def set_weight_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Weight` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Weight` callback is only triggered if the weight has changed since the
        last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletLoadCell.FUNCTION_SET_WEIGHT_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_weight_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Weight Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLoadCell.FUNCTION_GET_WEIGHT_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_weight_callback_threshold(self, option, min, max):
        r"""
        Sets the thresholds for the :cb:`Weight Reached` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Callback is turned off"
         "'o'",    "Callback is triggered when the weight is *outside* the min and max values"
         "'i'",    "Callback is triggered when the weight is *inside* the min and max values"
         "'<'",    "Callback is triggered when the weight is smaller than the min value (max is ignored)"
         "'>'",    "Callback is triggered when the weight is greater than the min value (max is ignored)"
        """
        self.check_validity()

        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickletLoadCell.FUNCTION_SET_WEIGHT_CALLBACK_THRESHOLD, (option, min, max), 'c i i', 0, '')

    def get_weight_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Weight Callback Threshold`.
        """
        self.check_validity()

        return GetWeightCallbackThreshold(*self.ipcon.send_request(self, BrickletLoadCell.FUNCTION_GET_WEIGHT_CALLBACK_THRESHOLD, (), '', 17, 'c i i'))

    def set_debounce_period(self, debounce):
        r"""
        Sets the period with which the threshold callback

        * :cb:`Weight Reached`

        is triggered, if the threshold

        * :func:`Set Weight Callback Threshold`

        keeps being reached.
        """
        self.check_validity()

        debounce = int(debounce)

        self.ipcon.send_request(self, BrickletLoadCell.FUNCTION_SET_DEBOUNCE_PERIOD, (debounce,), 'I', 0, '')

    def get_debounce_period(self):
        r"""
        Returns the debounce period as set by :func:`Set Debounce Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLoadCell.FUNCTION_GET_DEBOUNCE_PERIOD, (), '', 12, 'I')

    def set_moving_average(self, average):
        r"""
        Sets the length of a `moving averaging <https://en.wikipedia.org/wiki/Moving_average>`__
        for the weight value.

        Setting the length to 1 will turn the averaging off. With less
        averaging, there is more noise on the data.
        """
        self.check_validity()

        average = int(average)

        self.ipcon.send_request(self, BrickletLoadCell.FUNCTION_SET_MOVING_AVERAGE, (average,), 'B', 0, '')

    def get_moving_average(self):
        r"""
        Returns the length moving average as set by :func:`Set Moving Average`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLoadCell.FUNCTION_GET_MOVING_AVERAGE, (), '', 9, 'B')

    def led_on(self):
        r"""
        Turns the LED on.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletLoadCell.FUNCTION_LED_ON, (), '', 0, '')

    def led_off(self):
        r"""
        Turns the LED off.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletLoadCell.FUNCTION_LED_OFF, (), '', 0, '')

    def is_led_on(self):
        r"""
        Returns *true* if the led is on, *false* otherwise.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLoadCell.FUNCTION_IS_LED_ON, (), '', 9, '!')

    def calibrate(self, weight):
        r"""
        To calibrate your Load Cell Bricklet you have to

        * empty the scale and call this function with 0 and
        * add a known weight to the scale and call this function with the weight.

        The calibration is saved in the EEPROM of the Bricklet and only
        needs to be done once.

        We recommend to use the Brick Viewer for calibration, you don't need
        to call this function in your source code.
        """
        self.check_validity()

        weight = int(weight)

        self.ipcon.send_request(self, BrickletLoadCell.FUNCTION_CALIBRATE, (weight,), 'I', 0, '')

    def tare(self):
        r"""
        Sets the currently measured weight as tare weight.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletLoadCell.FUNCTION_TARE, (), '', 0, '')

    def set_configuration(self, rate, gain):
        r"""
        The measurement rate and gain are configurable.

        The rate can be either 10Hz or 80Hz. A faster rate will produce more noise.
        It is additionally possible to add a moving average
        (see :func:`Set Moving Average`) to the measurements.

        The gain can be 128x, 64x or 32x. It represents a measurement range of
        ±20mV, ±40mV and ±80mV respectively. The Load Cell Bricklet uses an
        excitation voltage of 5V and most load cells use an output of 2mV/V. That
        means the voltage range is ±15mV for most load cells (i.e. gain of 128x
        is best). If you don't know what all of this means you should keep it at
        128x, it will most likely be correct.

        The configuration is saved in the EEPROM of the Bricklet and only
        needs to be done once.

        We recommend to use the Brick Viewer for configuration, you don't need
        to call this function in your source code.
        """
        self.check_validity()

        rate = int(rate)
        gain = int(gain)

        self.ipcon.send_request(self, BrickletLoadCell.FUNCTION_SET_CONFIGURATION, (rate, gain), 'B B', 0, '')

    def get_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Configuration`.
        """
        self.check_validity()

        return GetConfiguration(*self.ipcon.send_request(self, BrickletLoadCell.FUNCTION_GET_CONFIGURATION, (), '', 10, 'B B'))

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletLoadCell.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

LoadCell = BrickletLoadCell # for backward compatibility
