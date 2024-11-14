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

GetConfiguration = namedtuple('Configuration', ['averaging', 'voltage_conversion_time', 'current_conversion_time'])
GetCalibration = namedtuple('Calibration', ['gain_multiplier', 'gain_divisor'])
GetCurrentCallbackThreshold = namedtuple('CurrentCallbackThreshold', ['option', 'min', 'max'])
GetVoltageCallbackThreshold = namedtuple('VoltageCallbackThreshold', ['option', 'min', 'max'])
GetPowerCallbackThreshold = namedtuple('PowerCallbackThreshold', ['option', 'min', 'max'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletVoltageCurrent(Device):
    r"""
    Measures power, DC voltage and DC current up to 720W/36V/20A
    """

    DEVICE_IDENTIFIER = 227
    DEVICE_DISPLAY_NAME = 'Voltage/Current Bricklet'
    DEVICE_URL_PART = 'voltage_current' # internal

    CALLBACK_CURRENT = 22
    CALLBACK_VOLTAGE = 23
    CALLBACK_POWER = 24
    CALLBACK_CURRENT_REACHED = 25
    CALLBACK_VOLTAGE_REACHED = 26
    CALLBACK_POWER_REACHED = 27


    FUNCTION_GET_CURRENT = 1
    FUNCTION_GET_VOLTAGE = 2
    FUNCTION_GET_POWER = 3
    FUNCTION_SET_CONFIGURATION = 4
    FUNCTION_GET_CONFIGURATION = 5
    FUNCTION_SET_CALIBRATION = 6
    FUNCTION_GET_CALIBRATION = 7
    FUNCTION_SET_CURRENT_CALLBACK_PERIOD = 8
    FUNCTION_GET_CURRENT_CALLBACK_PERIOD = 9
    FUNCTION_SET_VOLTAGE_CALLBACK_PERIOD = 10
    FUNCTION_GET_VOLTAGE_CALLBACK_PERIOD = 11
    FUNCTION_SET_POWER_CALLBACK_PERIOD = 12
    FUNCTION_GET_POWER_CALLBACK_PERIOD = 13
    FUNCTION_SET_CURRENT_CALLBACK_THRESHOLD = 14
    FUNCTION_GET_CURRENT_CALLBACK_THRESHOLD = 15
    FUNCTION_SET_VOLTAGE_CALLBACK_THRESHOLD = 16
    FUNCTION_GET_VOLTAGE_CALLBACK_THRESHOLD = 17
    FUNCTION_SET_POWER_CALLBACK_THRESHOLD = 18
    FUNCTION_GET_POWER_CALLBACK_THRESHOLD = 19
    FUNCTION_SET_DEBOUNCE_PERIOD = 20
    FUNCTION_GET_DEBOUNCE_PERIOD = 21
    FUNCTION_GET_IDENTITY = 255

    AVERAGING_1 = 0
    AVERAGING_4 = 1
    AVERAGING_16 = 2
    AVERAGING_64 = 3
    AVERAGING_128 = 4
    AVERAGING_256 = 5
    AVERAGING_512 = 6
    AVERAGING_1024 = 7
    THRESHOLD_OPTION_OFF = 'x'
    THRESHOLD_OPTION_OUTSIDE = 'o'
    THRESHOLD_OPTION_INSIDE = 'i'
    THRESHOLD_OPTION_SMALLER = '<'
    THRESHOLD_OPTION_GREATER = '>'
    CONVERSION_TIME_140US = 0
    CONVERSION_TIME_204US = 1
    CONVERSION_TIME_332US = 2
    CONVERSION_TIME_588US = 3
    CONVERSION_TIME_1_1MS = 4
    CONVERSION_TIME_2_116MS = 5
    CONVERSION_TIME_4_156MS = 6
    CONVERSION_TIME_8_244MS = 7

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletVoltageCurrent.DEVICE_IDENTIFIER, BrickletVoltageCurrent.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletVoltageCurrent.FUNCTION_GET_CURRENT] = BrickletVoltageCurrent.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletVoltageCurrent.FUNCTION_GET_VOLTAGE] = BrickletVoltageCurrent.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletVoltageCurrent.FUNCTION_GET_POWER] = BrickletVoltageCurrent.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletVoltageCurrent.FUNCTION_SET_CONFIGURATION] = BrickletVoltageCurrent.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletVoltageCurrent.FUNCTION_GET_CONFIGURATION] = BrickletVoltageCurrent.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletVoltageCurrent.FUNCTION_SET_CALIBRATION] = BrickletVoltageCurrent.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletVoltageCurrent.FUNCTION_GET_CALIBRATION] = BrickletVoltageCurrent.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletVoltageCurrent.FUNCTION_SET_CURRENT_CALLBACK_PERIOD] = BrickletVoltageCurrent.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletVoltageCurrent.FUNCTION_GET_CURRENT_CALLBACK_PERIOD] = BrickletVoltageCurrent.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletVoltageCurrent.FUNCTION_SET_VOLTAGE_CALLBACK_PERIOD] = BrickletVoltageCurrent.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletVoltageCurrent.FUNCTION_GET_VOLTAGE_CALLBACK_PERIOD] = BrickletVoltageCurrent.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletVoltageCurrent.FUNCTION_SET_POWER_CALLBACK_PERIOD] = BrickletVoltageCurrent.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletVoltageCurrent.FUNCTION_GET_POWER_CALLBACK_PERIOD] = BrickletVoltageCurrent.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletVoltageCurrent.FUNCTION_SET_CURRENT_CALLBACK_THRESHOLD] = BrickletVoltageCurrent.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletVoltageCurrent.FUNCTION_GET_CURRENT_CALLBACK_THRESHOLD] = BrickletVoltageCurrent.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletVoltageCurrent.FUNCTION_SET_VOLTAGE_CALLBACK_THRESHOLD] = BrickletVoltageCurrent.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletVoltageCurrent.FUNCTION_GET_VOLTAGE_CALLBACK_THRESHOLD] = BrickletVoltageCurrent.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletVoltageCurrent.FUNCTION_SET_POWER_CALLBACK_THRESHOLD] = BrickletVoltageCurrent.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletVoltageCurrent.FUNCTION_GET_POWER_CALLBACK_THRESHOLD] = BrickletVoltageCurrent.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletVoltageCurrent.FUNCTION_SET_DEBOUNCE_PERIOD] = BrickletVoltageCurrent.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletVoltageCurrent.FUNCTION_GET_DEBOUNCE_PERIOD] = BrickletVoltageCurrent.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletVoltageCurrent.FUNCTION_GET_IDENTITY] = BrickletVoltageCurrent.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletVoltageCurrent.CALLBACK_CURRENT] = (12, 'i')
        self.callback_formats[BrickletVoltageCurrent.CALLBACK_VOLTAGE] = (12, 'i')
        self.callback_formats[BrickletVoltageCurrent.CALLBACK_POWER] = (12, 'i')
        self.callback_formats[BrickletVoltageCurrent.CALLBACK_CURRENT_REACHED] = (12, 'i')
        self.callback_formats[BrickletVoltageCurrent.CALLBACK_VOLTAGE_REACHED] = (12, 'i')
        self.callback_formats[BrickletVoltageCurrent.CALLBACK_POWER_REACHED] = (12, 'i')

        ipcon.add_device(self)

    def get_current(self):
        r"""
        Returns the current.

        If you want to get the current periodically, it is recommended to use the
        :cb:`Current` callback and set the period with
        :func:`Set Current Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletVoltageCurrent.FUNCTION_GET_CURRENT, (), '', 12, 'i')

    def get_voltage(self):
        r"""
        Returns the voltage.

        If you want to get the voltage periodically, it is recommended to use the
        :cb:`Voltage` callback and set the period with
        :func:`Set Voltage Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletVoltageCurrent.FUNCTION_GET_VOLTAGE, (), '', 12, 'i')

    def get_power(self):
        r"""
        Returns the power.

        If you want to get the power periodically, it is recommended to use the
        :cb:`Power` callback and set the period with
        :func:`Set Power Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletVoltageCurrent.FUNCTION_GET_POWER, (), '', 12, 'i')

    def set_configuration(self, averaging, voltage_conversion_time, current_conversion_time):
        r"""
        Sets the configuration of the Voltage/Current Bricklet. It is
        possible to configure number of averages as well as
        voltage and current conversion time.
        """
        self.check_validity()

        averaging = int(averaging)
        voltage_conversion_time = int(voltage_conversion_time)
        current_conversion_time = int(current_conversion_time)

        self.ipcon.send_request(self, BrickletVoltageCurrent.FUNCTION_SET_CONFIGURATION, (averaging, voltage_conversion_time, current_conversion_time), 'B B B', 0, '')

    def get_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Configuration`.
        """
        self.check_validity()

        return GetConfiguration(*self.ipcon.send_request(self, BrickletVoltageCurrent.FUNCTION_GET_CONFIGURATION, (), '', 11, 'B B B'))

    def set_calibration(self, gain_multiplier, gain_divisor):
        r"""
        Since the shunt resistor that is used to measure the current is not
        perfectly precise, it needs to be calibrated by a multiplier and
        divisor if a very precise reading is needed.

        For example, if you are expecting a measurement of 1000mA and you
        are measuring 1023mA, you can calibrate the Voltage/Current Bricklet
        by setting the multiplier to 1000 and the divisor to 1023.
        """
        self.check_validity()

        gain_multiplier = int(gain_multiplier)
        gain_divisor = int(gain_divisor)

        self.ipcon.send_request(self, BrickletVoltageCurrent.FUNCTION_SET_CALIBRATION, (gain_multiplier, gain_divisor), 'H H', 0, '')

    def get_calibration(self):
        r"""
        Returns the calibration as set by :func:`Set Calibration`.
        """
        self.check_validity()

        return GetCalibration(*self.ipcon.send_request(self, BrickletVoltageCurrent.FUNCTION_GET_CALIBRATION, (), '', 12, 'H H'))

    def set_current_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Current` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Current` callback is only triggered if the current has changed since
        the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletVoltageCurrent.FUNCTION_SET_CURRENT_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_current_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Current Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletVoltageCurrent.FUNCTION_GET_CURRENT_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_voltage_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Voltage` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Voltage` callback is only triggered if the voltage has changed since
        the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletVoltageCurrent.FUNCTION_SET_VOLTAGE_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_voltage_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Voltage Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletVoltageCurrent.FUNCTION_GET_VOLTAGE_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_power_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Power` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Power` callback is only triggered if the power has changed since the
        last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletVoltageCurrent.FUNCTION_SET_POWER_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_power_callback_period(self):
        r"""
        Returns the period as set by :func:`Get Power Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletVoltageCurrent.FUNCTION_GET_POWER_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_current_callback_threshold(self, option, min, max):
        r"""
        Sets the thresholds for the :cb:`Current Reached` callback.

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

        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickletVoltageCurrent.FUNCTION_SET_CURRENT_CALLBACK_THRESHOLD, (option, min, max), 'c i i', 0, '')

    def get_current_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Current Callback Threshold`.
        """
        self.check_validity()

        return GetCurrentCallbackThreshold(*self.ipcon.send_request(self, BrickletVoltageCurrent.FUNCTION_GET_CURRENT_CALLBACK_THRESHOLD, (), '', 17, 'c i i'))

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

        self.ipcon.send_request(self, BrickletVoltageCurrent.FUNCTION_SET_VOLTAGE_CALLBACK_THRESHOLD, (option, min, max), 'c i i', 0, '')

    def get_voltage_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Voltage Callback Threshold`.
        """
        self.check_validity()

        return GetVoltageCallbackThreshold(*self.ipcon.send_request(self, BrickletVoltageCurrent.FUNCTION_GET_VOLTAGE_CALLBACK_THRESHOLD, (), '', 17, 'c i i'))

    def set_power_callback_threshold(self, option, min, max):
        r"""
        Sets the thresholds for the :cb:`Power Reached` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Callback is turned off"
         "'o'",    "Callback is triggered when the power is *outside* the min and max values"
         "'i'",    "Callback is triggered when the power is *inside* the min and max values"
         "'<'",    "Callback is triggered when the power is smaller than the min value (max is ignored)"
         "'>'",    "Callback is triggered when the power is greater than the min value (max is ignored)"
        """
        self.check_validity()

        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickletVoltageCurrent.FUNCTION_SET_POWER_CALLBACK_THRESHOLD, (option, min, max), 'c i i', 0, '')

    def get_power_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Power Callback Threshold`.
        """
        self.check_validity()

        return GetPowerCallbackThreshold(*self.ipcon.send_request(self, BrickletVoltageCurrent.FUNCTION_GET_POWER_CALLBACK_THRESHOLD, (), '', 17, 'c i i'))

    def set_debounce_period(self, debounce):
        r"""
        Sets the period with which the threshold callbacks

        * :cb:`Current Reached`,
        * :cb:`Voltage Reached`,
        * :cb:`Power Reached`

        are triggered, if the thresholds

        * :func:`Set Current Callback Threshold`,
        * :func:`Set Voltage Callback Threshold`,
        * :func:`Set Power Callback Threshold`

        keep being reached.
        """
        self.check_validity()

        debounce = int(debounce)

        self.ipcon.send_request(self, BrickletVoltageCurrent.FUNCTION_SET_DEBOUNCE_PERIOD, (debounce,), 'I', 0, '')

    def get_debounce_period(self):
        r"""
        Returns the debounce period as set by :func:`Set Debounce Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletVoltageCurrent.FUNCTION_GET_DEBOUNCE_PERIOD, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletVoltageCurrent.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

VoltageCurrent = BrickletVoltageCurrent # for backward compatibility
