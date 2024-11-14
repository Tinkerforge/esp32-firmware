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

GetColor = namedtuple('Color', ['r', 'g', 'b', 'c'])
GetColorCallbackThreshold = namedtuple('ColorCallbackThreshold', ['option', 'min_r', 'max_r', 'min_g', 'max_g', 'min_b', 'max_b', 'min_c', 'max_c'])
GetConfig = namedtuple('Config', ['gain', 'integration_time'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletColor(Device):
    r"""
    Measures color (RGB value), illuminance and color temperature
    """

    DEVICE_IDENTIFIER = 243
    DEVICE_DISPLAY_NAME = 'Color Bricklet'
    DEVICE_URL_PART = 'color' # internal

    CALLBACK_COLOR = 8
    CALLBACK_COLOR_REACHED = 9
    CALLBACK_ILLUMINANCE = 21
    CALLBACK_COLOR_TEMPERATURE = 22


    FUNCTION_GET_COLOR = 1
    FUNCTION_SET_COLOR_CALLBACK_PERIOD = 2
    FUNCTION_GET_COLOR_CALLBACK_PERIOD = 3
    FUNCTION_SET_COLOR_CALLBACK_THRESHOLD = 4
    FUNCTION_GET_COLOR_CALLBACK_THRESHOLD = 5
    FUNCTION_SET_DEBOUNCE_PERIOD = 6
    FUNCTION_GET_DEBOUNCE_PERIOD = 7
    FUNCTION_LIGHT_ON = 10
    FUNCTION_LIGHT_OFF = 11
    FUNCTION_IS_LIGHT_ON = 12
    FUNCTION_SET_CONFIG = 13
    FUNCTION_GET_CONFIG = 14
    FUNCTION_GET_ILLUMINANCE = 15
    FUNCTION_GET_COLOR_TEMPERATURE = 16
    FUNCTION_SET_ILLUMINANCE_CALLBACK_PERIOD = 17
    FUNCTION_GET_ILLUMINANCE_CALLBACK_PERIOD = 18
    FUNCTION_SET_COLOR_TEMPERATURE_CALLBACK_PERIOD = 19
    FUNCTION_GET_COLOR_TEMPERATURE_CALLBACK_PERIOD = 20
    FUNCTION_GET_IDENTITY = 255

    THRESHOLD_OPTION_OFF = 'x'
    THRESHOLD_OPTION_OUTSIDE = 'o'
    THRESHOLD_OPTION_INSIDE = 'i'
    THRESHOLD_OPTION_SMALLER = '<'
    THRESHOLD_OPTION_GREATER = '>'
    LIGHT_ON = 0
    LIGHT_OFF = 1
    GAIN_1X = 0
    GAIN_4X = 1
    GAIN_16X = 2
    GAIN_60X = 3
    INTEGRATION_TIME_2MS = 0
    INTEGRATION_TIME_24MS = 1
    INTEGRATION_TIME_101MS = 2
    INTEGRATION_TIME_154MS = 3
    INTEGRATION_TIME_700MS = 4

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletColor.DEVICE_IDENTIFIER, BrickletColor.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletColor.FUNCTION_GET_COLOR] = BrickletColor.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletColor.FUNCTION_SET_COLOR_CALLBACK_PERIOD] = BrickletColor.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletColor.FUNCTION_GET_COLOR_CALLBACK_PERIOD] = BrickletColor.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletColor.FUNCTION_SET_COLOR_CALLBACK_THRESHOLD] = BrickletColor.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletColor.FUNCTION_GET_COLOR_CALLBACK_THRESHOLD] = BrickletColor.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletColor.FUNCTION_SET_DEBOUNCE_PERIOD] = BrickletColor.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletColor.FUNCTION_GET_DEBOUNCE_PERIOD] = BrickletColor.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletColor.FUNCTION_LIGHT_ON] = BrickletColor.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletColor.FUNCTION_LIGHT_OFF] = BrickletColor.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletColor.FUNCTION_IS_LIGHT_ON] = BrickletColor.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletColor.FUNCTION_SET_CONFIG] = BrickletColor.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletColor.FUNCTION_GET_CONFIG] = BrickletColor.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletColor.FUNCTION_GET_ILLUMINANCE] = BrickletColor.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletColor.FUNCTION_GET_COLOR_TEMPERATURE] = BrickletColor.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletColor.FUNCTION_SET_ILLUMINANCE_CALLBACK_PERIOD] = BrickletColor.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletColor.FUNCTION_GET_ILLUMINANCE_CALLBACK_PERIOD] = BrickletColor.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletColor.FUNCTION_SET_COLOR_TEMPERATURE_CALLBACK_PERIOD] = BrickletColor.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletColor.FUNCTION_GET_COLOR_TEMPERATURE_CALLBACK_PERIOD] = BrickletColor.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletColor.FUNCTION_GET_IDENTITY] = BrickletColor.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletColor.CALLBACK_COLOR] = (16, 'H H H H')
        self.callback_formats[BrickletColor.CALLBACK_COLOR_REACHED] = (16, 'H H H H')
        self.callback_formats[BrickletColor.CALLBACK_ILLUMINANCE] = (12, 'I')
        self.callback_formats[BrickletColor.CALLBACK_COLOR_TEMPERATURE] = (10, 'H')

        ipcon.add_device(self)

    def get_color(self):
        r"""
        Returns the measured color of the sensor.

        The red (r), green (g), blue (b) and clear (c) colors are measured
        with four different photodiodes that are responsive at different
        wavelengths:

        .. image:: /Images/Bricklets/bricklet_color_wavelength_chart_600.jpg
           :scale: 100 %
           :alt: Chart Responsivity / Wavelength
           :align: center
           :target: ../../_images/Bricklets/bricklet_color_wavelength_chart_600.jpg

        If you want to get the color periodically, it is recommended
        to use the :cb:`Color` callback and set the period with
        :func:`Set Color Callback Period`.
        """
        self.check_validity()

        return GetColor(*self.ipcon.send_request(self, BrickletColor.FUNCTION_GET_COLOR, (), '', 16, 'H H H H'))

    def set_color_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Color` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Color` callback is only triggered if the color has changed since the
        last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletColor.FUNCTION_SET_COLOR_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_color_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Color Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletColor.FUNCTION_GET_COLOR_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_color_callback_threshold(self, option, min_r, max_r, min_g, max_g, min_b, max_b, min_c, max_c):
        r"""
        Sets the thresholds for the :cb:`Color Reached` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Callback is turned off"
         "'o'",    "Callback is triggered when the temperature is *outside* the min and max values"
         "'i'",    "Callback is triggered when the temperature is *inside* the min and max values"
         "'<'",    "Callback is triggered when the temperature is smaller than the min value (max is ignored)"
         "'>'",    "Callback is triggered when the temperature is greater than the min value (max is ignored)"
        """
        self.check_validity()

        option = create_char(option)
        min_r = int(min_r)
        max_r = int(max_r)
        min_g = int(min_g)
        max_g = int(max_g)
        min_b = int(min_b)
        max_b = int(max_b)
        min_c = int(min_c)
        max_c = int(max_c)

        self.ipcon.send_request(self, BrickletColor.FUNCTION_SET_COLOR_CALLBACK_THRESHOLD, (option, min_r, max_r, min_g, max_g, min_b, max_b, min_c, max_c), 'c H H H H H H H H', 0, '')

    def get_color_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Color Callback Threshold`.
        """
        self.check_validity()

        return GetColorCallbackThreshold(*self.ipcon.send_request(self, BrickletColor.FUNCTION_GET_COLOR_CALLBACK_THRESHOLD, (), '', 25, 'c H H H H H H H H'))

    def set_debounce_period(self, debounce):
        r"""
        Sets the period with which the threshold callback

        * :cb:`Color Reached`

        is triggered, if the threshold

        * :func:`Set Color Callback Threshold`

        keeps being reached.
        """
        self.check_validity()

        debounce = int(debounce)

        self.ipcon.send_request(self, BrickletColor.FUNCTION_SET_DEBOUNCE_PERIOD, (debounce,), 'I', 0, '')

    def get_debounce_period(self):
        r"""
        Returns the debounce period as set by :func:`Set Debounce Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletColor.FUNCTION_GET_DEBOUNCE_PERIOD, (), '', 12, 'I')

    def light_on(self):
        r"""
        Turns the LED on.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletColor.FUNCTION_LIGHT_ON, (), '', 0, '')

    def light_off(self):
        r"""
        Turns the LED off.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletColor.FUNCTION_LIGHT_OFF, (), '', 0, '')

    def is_light_on(self):
        r"""
        Returns the state of the LED. Possible values are:

        * 0: On
        * 1: Off
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletColor.FUNCTION_IS_LIGHT_ON, (), '', 9, 'B')

    def set_config(self, gain, integration_time):
        r"""
        Sets the configuration of the sensor. Gain and integration time
        can be configured in this way.

        For configuring the gain:

        * 0: 1x Gain
        * 1: 4x Gain
        * 2: 16x Gain
        * 3: 60x Gain

        For configuring the integration time:

        * 0: 2.4ms
        * 1: 24ms
        * 2: 101ms
        * 3: 154ms
        * 4: 700ms

        Increasing the gain enables the sensor to detect a
        color from a higher distance.

        The integration time provides a trade-off between conversion time
        and accuracy. With a longer integration time the values read will
        be more accurate but it will take longer time to get the conversion
        results.
        """
        self.check_validity()

        gain = int(gain)
        integration_time = int(integration_time)

        self.ipcon.send_request(self, BrickletColor.FUNCTION_SET_CONFIG, (gain, integration_time), 'B B', 0, '')

    def get_config(self):
        r"""
        Returns the configuration as set by :func:`Set Config`.
        """
        self.check_validity()

        return GetConfig(*self.ipcon.send_request(self, BrickletColor.FUNCTION_GET_CONFIG, (), '', 10, 'B B'))

    def get_illuminance(self):
        r"""
        Returns the illuminance affected by the gain and integration time as
        set by :func:`Set Config`. To get the illuminance in Lux apply this formula::

         lux = illuminance * 700 / gain / integration_time

        To get a correct illuminance measurement make sure that the color
        values themselves are not saturated. The color value (R, G or B)
        is saturated if it is equal to the maximum value of 65535.
        In that case you have to reduce the gain, see :func:`Set Config`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletColor.FUNCTION_GET_ILLUMINANCE, (), '', 12, 'I')

    def get_color_temperature(self):
        r"""
        Returns the color temperature.

        To get a correct color temperature measurement make sure that the color
        values themselves are not saturated. The color value (R, G or B)
        is saturated if it is equal to the maximum value of 65535.
        In that case you have to reduce the gain, see :func:`Set Config`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletColor.FUNCTION_GET_COLOR_TEMPERATURE, (), '', 10, 'H')

    def set_illuminance_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Illuminance` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Illuminance` callback is only triggered if the illuminance has changed
        since the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletColor.FUNCTION_SET_ILLUMINANCE_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_illuminance_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Illuminance Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletColor.FUNCTION_GET_ILLUMINANCE_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_color_temperature_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Color Temperature` callback is
        triggered periodically. A value of 0 turns the callback off.

        The :cb:`Color Temperature` callback is only triggered if the color temperature
        has changed since the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletColor.FUNCTION_SET_COLOR_TEMPERATURE_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_color_temperature_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Color Temperature Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletColor.FUNCTION_GET_COLOR_TEMPERATURE_CALLBACK_PERIOD, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletColor.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

Color = BrickletColor # for backward compatibility
