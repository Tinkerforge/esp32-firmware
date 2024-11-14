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

GetIlluminanceCallbackThreshold = namedtuple('IlluminanceCallbackThreshold', ['option', 'min', 'max'])
GetConfiguration = namedtuple('Configuration', ['illuminance_range', 'integration_time'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletAmbientLightV2(Device):
    r"""
    Measures ambient light up to 64000lux
    """

    DEVICE_IDENTIFIER = 259
    DEVICE_DISPLAY_NAME = 'Ambient Light Bricklet 2.0'
    DEVICE_URL_PART = 'ambient_light_v2' # internal

    CALLBACK_ILLUMINANCE = 10
    CALLBACK_ILLUMINANCE_REACHED = 11


    FUNCTION_GET_ILLUMINANCE = 1
    FUNCTION_SET_ILLUMINANCE_CALLBACK_PERIOD = 2
    FUNCTION_GET_ILLUMINANCE_CALLBACK_PERIOD = 3
    FUNCTION_SET_ILLUMINANCE_CALLBACK_THRESHOLD = 4
    FUNCTION_GET_ILLUMINANCE_CALLBACK_THRESHOLD = 5
    FUNCTION_SET_DEBOUNCE_PERIOD = 6
    FUNCTION_GET_DEBOUNCE_PERIOD = 7
    FUNCTION_SET_CONFIGURATION = 8
    FUNCTION_GET_CONFIGURATION = 9
    FUNCTION_GET_IDENTITY = 255

    THRESHOLD_OPTION_OFF = 'x'
    THRESHOLD_OPTION_OUTSIDE = 'o'
    THRESHOLD_OPTION_INSIDE = 'i'
    THRESHOLD_OPTION_SMALLER = '<'
    THRESHOLD_OPTION_GREATER = '>'
    ILLUMINANCE_RANGE_UNLIMITED = 6
    ILLUMINANCE_RANGE_64000LUX = 0
    ILLUMINANCE_RANGE_32000LUX = 1
    ILLUMINANCE_RANGE_16000LUX = 2
    ILLUMINANCE_RANGE_8000LUX = 3
    ILLUMINANCE_RANGE_1300LUX = 4
    ILLUMINANCE_RANGE_600LUX = 5
    INTEGRATION_TIME_50MS = 0
    INTEGRATION_TIME_100MS = 1
    INTEGRATION_TIME_150MS = 2
    INTEGRATION_TIME_200MS = 3
    INTEGRATION_TIME_250MS = 4
    INTEGRATION_TIME_300MS = 5
    INTEGRATION_TIME_350MS = 6
    INTEGRATION_TIME_400MS = 7

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletAmbientLightV2.DEVICE_IDENTIFIER, BrickletAmbientLightV2.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 1)

        self.response_expected[BrickletAmbientLightV2.FUNCTION_GET_ILLUMINANCE] = BrickletAmbientLightV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAmbientLightV2.FUNCTION_SET_ILLUMINANCE_CALLBACK_PERIOD] = BrickletAmbientLightV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletAmbientLightV2.FUNCTION_GET_ILLUMINANCE_CALLBACK_PERIOD] = BrickletAmbientLightV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAmbientLightV2.FUNCTION_SET_ILLUMINANCE_CALLBACK_THRESHOLD] = BrickletAmbientLightV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletAmbientLightV2.FUNCTION_GET_ILLUMINANCE_CALLBACK_THRESHOLD] = BrickletAmbientLightV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAmbientLightV2.FUNCTION_SET_DEBOUNCE_PERIOD] = BrickletAmbientLightV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletAmbientLightV2.FUNCTION_GET_DEBOUNCE_PERIOD] = BrickletAmbientLightV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAmbientLightV2.FUNCTION_SET_CONFIGURATION] = BrickletAmbientLightV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAmbientLightV2.FUNCTION_GET_CONFIGURATION] = BrickletAmbientLightV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAmbientLightV2.FUNCTION_GET_IDENTITY] = BrickletAmbientLightV2.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletAmbientLightV2.CALLBACK_ILLUMINANCE] = (12, 'I')
        self.callback_formats[BrickletAmbientLightV2.CALLBACK_ILLUMINANCE_REACHED] = (12, 'I')

        ipcon.add_device(self)

    def get_illuminance(self):
        r"""
        Returns the illuminance of the ambient light sensor. The measurement range goes
        up to about 100000lux, but above 64000lux the precision starts to drop.

        .. versionchanged:: 2.0.2$nbsp;(Plugin)
          An illuminance of 0lux indicates an error condition where the sensor cannot
          perform a reasonable measurement. This can happen with very dim or very bright
          light conditions. In bright light conditions this might indicate that the sensor
          is saturated and the configuration should be modified (:func:`Set Configuration`)
          to better match the light conditions.

        If you want to get the illuminance periodically, it is recommended to use the
        :cb:`Illuminance` callback and set the period with
        :func:`Set Illuminance Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAmbientLightV2.FUNCTION_GET_ILLUMINANCE, (), '', 12, 'I')

    def set_illuminance_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Illuminance` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Illuminance` callback is only triggered if the illuminance has changed
        since the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletAmbientLightV2.FUNCTION_SET_ILLUMINANCE_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_illuminance_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Illuminance Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAmbientLightV2.FUNCTION_GET_ILLUMINANCE_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_illuminance_callback_threshold(self, option, min, max):
        r"""
        Sets the thresholds for the :cb:`Illuminance Reached` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Callback is turned off"
         "'o'",    "Callback is triggered when the illuminance is *outside* the min and max values"
         "'i'",    "Callback is triggered when the illuminance is *inside* the min and max values"
         "'<'",    "Callback is triggered when the illuminance is smaller than the min value (max is ignored)"
         "'>'",    "Callback is triggered when the illuminance is greater than the min value (max is ignored)"
        """
        self.check_validity()

        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickletAmbientLightV2.FUNCTION_SET_ILLUMINANCE_CALLBACK_THRESHOLD, (option, min, max), 'c I I', 0, '')

    def get_illuminance_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Illuminance Callback Threshold`.
        """
        self.check_validity()

        return GetIlluminanceCallbackThreshold(*self.ipcon.send_request(self, BrickletAmbientLightV2.FUNCTION_GET_ILLUMINANCE_CALLBACK_THRESHOLD, (), '', 17, 'c I I'))

    def set_debounce_period(self, debounce):
        r"""
        Sets the period with which the threshold callbacks

        * :cb:`Illuminance Reached`,

        are triggered, if the thresholds

        * :func:`Set Illuminance Callback Threshold`,

        keep being reached.
        """
        self.check_validity()

        debounce = int(debounce)

        self.ipcon.send_request(self, BrickletAmbientLightV2.FUNCTION_SET_DEBOUNCE_PERIOD, (debounce,), 'I', 0, '')

    def get_debounce_period(self):
        r"""
        Returns the debounce period as set by :func:`Set Debounce Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAmbientLightV2.FUNCTION_GET_DEBOUNCE_PERIOD, (), '', 12, 'I')

    def set_configuration(self, illuminance_range, integration_time):
        r"""
        Sets the configuration. It is possible to configure an illuminance range
        between 0-600lux and 0-64000lux and an integration time between 50ms and 400ms.

        .. versionadded:: 2.0.2$nbsp;(Plugin)
          The unlimited illuminance range allows to measure up to about 100000lux, but
          above 64000lux the precision starts to drop.

        A smaller illuminance range increases the resolution of the data. A longer
        integration time will result in less noise on the data.

        .. versionchanged:: 2.0.2$nbsp;(Plugin)
          If the actual measure illuminance is out-of-range then the current illuminance
          range maximum +0.01lux is reported by :func:`Get Illuminance` and the
          :cb:`Illuminance` callback. For example, 800001 for the 0-8000lux range.

        .. versionchanged:: 2.0.2$nbsp;(Plugin)
          With a long integration time the sensor might be saturated before the measured
          value reaches the maximum of the selected illuminance range. In this case 0lux
          is reported by :func:`Get Illuminance` and the :cb:`Illuminance` callback.

        If the measurement is out-of-range or the sensor is saturated then you should
        configure the next higher illuminance range. If the highest range is already
        in use, then start to reduce the integration time.
        """
        self.check_validity()

        illuminance_range = int(illuminance_range)
        integration_time = int(integration_time)

        self.ipcon.send_request(self, BrickletAmbientLightV2.FUNCTION_SET_CONFIGURATION, (illuminance_range, integration_time), 'B B', 0, '')

    def get_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Configuration`.
        """
        self.check_validity()

        return GetConfiguration(*self.ipcon.send_request(self, BrickletAmbientLightV2.FUNCTION_GET_CONFIGURATION, (), '', 10, 'B B'))

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletAmbientLightV2.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

AmbientLightV2 = BrickletAmbientLightV2 # for backward compatibility
