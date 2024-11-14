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
GetCalibration = namedtuple('Calibration', ['offset', 'gain'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletIndustrialDualAnalogIn(Device):
    r"""
    Measures two DC voltages between -35V and +35V with 24bit resolution each
    """

    DEVICE_IDENTIFIER = 249
    DEVICE_DISPLAY_NAME = 'Industrial Dual Analog In Bricklet'
    DEVICE_URL_PART = 'industrial_dual_analog_in' # internal

    CALLBACK_VOLTAGE = 13
    CALLBACK_VOLTAGE_REACHED = 14


    FUNCTION_GET_VOLTAGE = 1
    FUNCTION_SET_VOLTAGE_CALLBACK_PERIOD = 2
    FUNCTION_GET_VOLTAGE_CALLBACK_PERIOD = 3
    FUNCTION_SET_VOLTAGE_CALLBACK_THRESHOLD = 4
    FUNCTION_GET_VOLTAGE_CALLBACK_THRESHOLD = 5
    FUNCTION_SET_DEBOUNCE_PERIOD = 6
    FUNCTION_GET_DEBOUNCE_PERIOD = 7
    FUNCTION_SET_SAMPLE_RATE = 8
    FUNCTION_GET_SAMPLE_RATE = 9
    FUNCTION_SET_CALIBRATION = 10
    FUNCTION_GET_CALIBRATION = 11
    FUNCTION_GET_ADC_VALUES = 12
    FUNCTION_GET_IDENTITY = 255

    THRESHOLD_OPTION_OFF = 'x'
    THRESHOLD_OPTION_OUTSIDE = 'o'
    THRESHOLD_OPTION_INSIDE = 'i'
    THRESHOLD_OPTION_SMALLER = '<'
    THRESHOLD_OPTION_GREATER = '>'
    SAMPLE_RATE_976_SPS = 0
    SAMPLE_RATE_488_SPS = 1
    SAMPLE_RATE_244_SPS = 2
    SAMPLE_RATE_122_SPS = 3
    SAMPLE_RATE_61_SPS = 4
    SAMPLE_RATE_4_SPS = 5
    SAMPLE_RATE_2_SPS = 6
    SAMPLE_RATE_1_SPS = 7

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletIndustrialDualAnalogIn.DEVICE_IDENTIFIER, BrickletIndustrialDualAnalogIn.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletIndustrialDualAnalogIn.FUNCTION_GET_VOLTAGE] = BrickletIndustrialDualAnalogIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDualAnalogIn.FUNCTION_SET_VOLTAGE_CALLBACK_PERIOD] = BrickletIndustrialDualAnalogIn.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIndustrialDualAnalogIn.FUNCTION_GET_VOLTAGE_CALLBACK_PERIOD] = BrickletIndustrialDualAnalogIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDualAnalogIn.FUNCTION_SET_VOLTAGE_CALLBACK_THRESHOLD] = BrickletIndustrialDualAnalogIn.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIndustrialDualAnalogIn.FUNCTION_GET_VOLTAGE_CALLBACK_THRESHOLD] = BrickletIndustrialDualAnalogIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDualAnalogIn.FUNCTION_SET_DEBOUNCE_PERIOD] = BrickletIndustrialDualAnalogIn.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIndustrialDualAnalogIn.FUNCTION_GET_DEBOUNCE_PERIOD] = BrickletIndustrialDualAnalogIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDualAnalogIn.FUNCTION_SET_SAMPLE_RATE] = BrickletIndustrialDualAnalogIn.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDualAnalogIn.FUNCTION_GET_SAMPLE_RATE] = BrickletIndustrialDualAnalogIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDualAnalogIn.FUNCTION_SET_CALIBRATION] = BrickletIndustrialDualAnalogIn.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDualAnalogIn.FUNCTION_GET_CALIBRATION] = BrickletIndustrialDualAnalogIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDualAnalogIn.FUNCTION_GET_ADC_VALUES] = BrickletIndustrialDualAnalogIn.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDualAnalogIn.FUNCTION_GET_IDENTITY] = BrickletIndustrialDualAnalogIn.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletIndustrialDualAnalogIn.CALLBACK_VOLTAGE] = (13, 'B i')
        self.callback_formats[BrickletIndustrialDualAnalogIn.CALLBACK_VOLTAGE_REACHED] = (13, 'B i')

        ipcon.add_device(self)

    def get_voltage(self, channel):
        r"""
        Returns the voltage for the given channel.

        If you want to get the voltage periodically, it is recommended to use the
        :cb:`Voltage` callback and set the period with
        :func:`Set Voltage Callback Period`.
        """
        self.check_validity()

        channel = int(channel)

        return self.ipcon.send_request(self, BrickletIndustrialDualAnalogIn.FUNCTION_GET_VOLTAGE, (channel,), 'B', 12, 'i')

    def set_voltage_callback_period(self, channel, period):
        r"""
        Sets the period with which the :cb:`Voltage` callback is triggered
        periodically for the given channel. A value of 0 turns the callback off.

        The :cb:`Voltage` callback is only triggered if the voltage has changed since the
        last triggering.
        """
        self.check_validity()

        channel = int(channel)
        period = int(period)

        self.ipcon.send_request(self, BrickletIndustrialDualAnalogIn.FUNCTION_SET_VOLTAGE_CALLBACK_PERIOD, (channel, period), 'B I', 0, '')

    def get_voltage_callback_period(self, channel):
        r"""
        Returns the period as set by :func:`Set Voltage Callback Period`.
        """
        self.check_validity()

        channel = int(channel)

        return self.ipcon.send_request(self, BrickletIndustrialDualAnalogIn.FUNCTION_GET_VOLTAGE_CALLBACK_PERIOD, (channel,), 'B', 12, 'I')

    def set_voltage_callback_threshold(self, channel, option, min, max):
        r"""
        Sets the thresholds for the :cb:`Voltage Reached` callback for the given
        channel.

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

        channel = int(channel)
        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickletIndustrialDualAnalogIn.FUNCTION_SET_VOLTAGE_CALLBACK_THRESHOLD, (channel, option, min, max), 'B c i i', 0, '')

    def get_voltage_callback_threshold(self, channel):
        r"""
        Returns the threshold as set by :func:`Set Voltage Callback Threshold`.
        """
        self.check_validity()

        channel = int(channel)

        return GetVoltageCallbackThreshold(*self.ipcon.send_request(self, BrickletIndustrialDualAnalogIn.FUNCTION_GET_VOLTAGE_CALLBACK_THRESHOLD, (channel,), 'B', 17, 'c i i'))

    def set_debounce_period(self, debounce):
        r"""
        Sets the period with which the threshold callback

        * :cb:`Voltage Reached`

        is triggered, if the threshold

        * :func:`Set Voltage Callback Threshold`

        keeps being reached.
        """
        self.check_validity()

        debounce = int(debounce)

        self.ipcon.send_request(self, BrickletIndustrialDualAnalogIn.FUNCTION_SET_DEBOUNCE_PERIOD, (debounce,), 'I', 0, '')

    def get_debounce_period(self):
        r"""
        Returns the debounce period as set by :func:`Set Debounce Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDualAnalogIn.FUNCTION_GET_DEBOUNCE_PERIOD, (), '', 12, 'I')

    def set_sample_rate(self, rate):
        r"""
        Sets the sample rate. The sample rate can be between 1 sample per second
        and 976 samples per second. Decreasing the sample rate will also decrease the
        noise on the data.
        """
        self.check_validity()

        rate = int(rate)

        self.ipcon.send_request(self, BrickletIndustrialDualAnalogIn.FUNCTION_SET_SAMPLE_RATE, (rate,), 'B', 0, '')

    def get_sample_rate(self):
        r"""
        Returns the sample rate as set by :func:`Set Sample Rate`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDualAnalogIn.FUNCTION_GET_SAMPLE_RATE, (), '', 9, 'B')

    def set_calibration(self, offset, gain):
        r"""
        Sets offset and gain of MCP3911 internal calibration registers.

        See MCP3911 datasheet 7.7 and 7.8. The Industrial Dual Analog In Bricklet
        is already factory calibrated by Tinkerforge. It should not be necessary
        for you to use this function
        """
        self.check_validity()

        offset = list(map(int, offset))
        gain = list(map(int, gain))

        self.ipcon.send_request(self, BrickletIndustrialDualAnalogIn.FUNCTION_SET_CALIBRATION, (offset, gain), '2i 2i', 0, '')

    def get_calibration(self):
        r"""
        Returns the calibration as set by :func:`Set Calibration`.
        """
        self.check_validity()

        return GetCalibration(*self.ipcon.send_request(self, BrickletIndustrialDualAnalogIn.FUNCTION_GET_CALIBRATION, (), '', 24, '2i 2i'))

    def get_adc_values(self):
        r"""
        Returns the ADC values as given by the MCP3911 IC. This function
        is needed for proper calibration, see :func:`Set Calibration`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDualAnalogIn.FUNCTION_GET_ADC_VALUES, (), '', 16, '2i')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletIndustrialDualAnalogIn.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

IndustrialDualAnalogIn = BrickletIndustrialDualAnalogIn # for backward compatibility
