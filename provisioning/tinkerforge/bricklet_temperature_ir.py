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

GetAmbientTemperatureCallbackThreshold = namedtuple('AmbientTemperatureCallbackThreshold', ['option', 'min', 'max'])
GetObjectTemperatureCallbackThreshold = namedtuple('ObjectTemperatureCallbackThreshold', ['option', 'min', 'max'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletTemperatureIR(Device):
    r"""
    Measures contactless object temperature between -70°C and +380°C
    """

    DEVICE_IDENTIFIER = 217
    DEVICE_DISPLAY_NAME = 'Temperature IR Bricklet'
    DEVICE_URL_PART = 'temperature_ir' # internal

    CALLBACK_AMBIENT_TEMPERATURE = 15
    CALLBACK_OBJECT_TEMPERATURE = 16
    CALLBACK_AMBIENT_TEMPERATURE_REACHED = 17
    CALLBACK_OBJECT_TEMPERATURE_REACHED = 18


    FUNCTION_GET_AMBIENT_TEMPERATURE = 1
    FUNCTION_GET_OBJECT_TEMPERATURE = 2
    FUNCTION_SET_EMISSIVITY = 3
    FUNCTION_GET_EMISSIVITY = 4
    FUNCTION_SET_AMBIENT_TEMPERATURE_CALLBACK_PERIOD = 5
    FUNCTION_GET_AMBIENT_TEMPERATURE_CALLBACK_PERIOD = 6
    FUNCTION_SET_OBJECT_TEMPERATURE_CALLBACK_PERIOD = 7
    FUNCTION_GET_OBJECT_TEMPERATURE_CALLBACK_PERIOD = 8
    FUNCTION_SET_AMBIENT_TEMPERATURE_CALLBACK_THRESHOLD = 9
    FUNCTION_GET_AMBIENT_TEMPERATURE_CALLBACK_THRESHOLD = 10
    FUNCTION_SET_OBJECT_TEMPERATURE_CALLBACK_THRESHOLD = 11
    FUNCTION_GET_OBJECT_TEMPERATURE_CALLBACK_THRESHOLD = 12
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
        Device.__init__(self, uid, ipcon, BrickletTemperatureIR.DEVICE_IDENTIFIER, BrickletTemperatureIR.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletTemperatureIR.FUNCTION_GET_AMBIENT_TEMPERATURE] = BrickletTemperatureIR.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletTemperatureIR.FUNCTION_GET_OBJECT_TEMPERATURE] = BrickletTemperatureIR.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletTemperatureIR.FUNCTION_SET_EMISSIVITY] = BrickletTemperatureIR.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletTemperatureIR.FUNCTION_GET_EMISSIVITY] = BrickletTemperatureIR.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletTemperatureIR.FUNCTION_SET_AMBIENT_TEMPERATURE_CALLBACK_PERIOD] = BrickletTemperatureIR.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletTemperatureIR.FUNCTION_GET_AMBIENT_TEMPERATURE_CALLBACK_PERIOD] = BrickletTemperatureIR.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletTemperatureIR.FUNCTION_SET_OBJECT_TEMPERATURE_CALLBACK_PERIOD] = BrickletTemperatureIR.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletTemperatureIR.FUNCTION_GET_OBJECT_TEMPERATURE_CALLBACK_PERIOD] = BrickletTemperatureIR.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletTemperatureIR.FUNCTION_SET_AMBIENT_TEMPERATURE_CALLBACK_THRESHOLD] = BrickletTemperatureIR.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletTemperatureIR.FUNCTION_GET_AMBIENT_TEMPERATURE_CALLBACK_THRESHOLD] = BrickletTemperatureIR.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletTemperatureIR.FUNCTION_SET_OBJECT_TEMPERATURE_CALLBACK_THRESHOLD] = BrickletTemperatureIR.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletTemperatureIR.FUNCTION_GET_OBJECT_TEMPERATURE_CALLBACK_THRESHOLD] = BrickletTemperatureIR.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletTemperatureIR.FUNCTION_SET_DEBOUNCE_PERIOD] = BrickletTemperatureIR.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletTemperatureIR.FUNCTION_GET_DEBOUNCE_PERIOD] = BrickletTemperatureIR.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletTemperatureIR.FUNCTION_GET_IDENTITY] = BrickletTemperatureIR.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletTemperatureIR.CALLBACK_AMBIENT_TEMPERATURE] = (10, 'h')
        self.callback_formats[BrickletTemperatureIR.CALLBACK_OBJECT_TEMPERATURE] = (10, 'h')
        self.callback_formats[BrickletTemperatureIR.CALLBACK_AMBIENT_TEMPERATURE_REACHED] = (10, 'h')
        self.callback_formats[BrickletTemperatureIR.CALLBACK_OBJECT_TEMPERATURE_REACHED] = (10, 'h')

        ipcon.add_device(self)

    def get_ambient_temperature(self):
        r"""
        Returns the ambient temperature of the sensor.

        If you want to get the ambient temperature periodically, it is recommended
        to use the :cb:`Ambient Temperature` callback and set the period with
        :func:`Set Ambient Temperature Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletTemperatureIR.FUNCTION_GET_AMBIENT_TEMPERATURE, (), '', 10, 'h')

    def get_object_temperature(self):
        r"""
        Returns the object temperature of the sensor, i.e. the temperature
        of the surface of the object the sensor is aimed at.

        The temperature of different materials is dependent on their `emissivity
        <https://en.wikipedia.org/wiki/Emissivity>`__. The emissivity of the material
        can be set with :func:`Set Emissivity`.

        If you want to get the object temperature periodically, it is recommended
        to use the :cb:`Object Temperature` callback and set the period with
        :func:`Set Object Temperature Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletTemperatureIR.FUNCTION_GET_OBJECT_TEMPERATURE, (), '', 10, 'h')

    def set_emissivity(self, emissivity):
        r"""
        Sets the `emissivity <https://en.wikipedia.org/wiki/Emissivity>`__ that is
        used to calculate the surface temperature as returned by
        :func:`Get Object Temperature`.

        The emissivity is usually given as a value between 0.0 and 1.0. A list of
        emissivities of different materials can be found
        `here <https://www.infrared-thermography.com/material.htm>`__.

        The parameter of :func:`Set Emissivity` has to be given with a factor of
        65535 (16-bit). For example: An emissivity of 0.1 can be set with the
        value 6553, an emissivity of 0.5 with the value 32767 and so on.

        .. note::
         If you need a precise measurement for the object temperature, it is
         absolutely crucial that you also provide a precise emissivity.

        The emissivity is stored in non-volatile memory and will still be used after a restart or power cycle of the Bricklet.
        """
        self.check_validity()

        emissivity = int(emissivity)

        self.ipcon.send_request(self, BrickletTemperatureIR.FUNCTION_SET_EMISSIVITY, (emissivity,), 'H', 0, '')

    def get_emissivity(self):
        r"""
        Returns the emissivity as set by :func:`Set Emissivity`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletTemperatureIR.FUNCTION_GET_EMISSIVITY, (), '', 10, 'H')

    def set_ambient_temperature_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Ambient Temperature` callback is
        triggered periodically. A value of 0 turns the callback off.

        The :cb:`Ambient Temperature` callback is only triggered if the temperature has
        changed since the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletTemperatureIR.FUNCTION_SET_AMBIENT_TEMPERATURE_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_ambient_temperature_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Ambient Temperature Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletTemperatureIR.FUNCTION_GET_AMBIENT_TEMPERATURE_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_object_temperature_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Object Temperature` callback is
        triggered periodically. A value of 0 turns the callback off.

        The :cb:`Object Temperature` callback is only triggered if the temperature
        has changed since the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletTemperatureIR.FUNCTION_SET_OBJECT_TEMPERATURE_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_object_temperature_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Object Temperature Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletTemperatureIR.FUNCTION_GET_OBJECT_TEMPERATURE_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_ambient_temperature_callback_threshold(self, option, min, max):
        r"""
        Sets the thresholds for the :cb:`Ambient Temperature Reached` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Callback is turned off"
         "'o'",    "Callback is triggered when the ambient temperature is *outside* the min and max values"
         "'i'",    "Callback is triggered when the ambient temperature is *inside* the min and max values"
         "'<'",    "Callback is triggered when the ambient temperature is smaller than the min value (max is ignored)"
         "'>'",    "Callback is triggered when the ambient temperature is greater than the min value (max is ignored)"
        """
        self.check_validity()

        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickletTemperatureIR.FUNCTION_SET_AMBIENT_TEMPERATURE_CALLBACK_THRESHOLD, (option, min, max), 'c h h', 0, '')

    def get_ambient_temperature_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Ambient Temperature Callback Threshold`.
        """
        self.check_validity()

        return GetAmbientTemperatureCallbackThreshold(*self.ipcon.send_request(self, BrickletTemperatureIR.FUNCTION_GET_AMBIENT_TEMPERATURE_CALLBACK_THRESHOLD, (), '', 13, 'c h h'))

    def set_object_temperature_callback_threshold(self, option, min, max):
        r"""
        Sets the thresholds for the :cb:`Object Temperature Reached` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Callback is turned off"
         "'o'",    "Callback is triggered when the object temperature is *outside* the min and max values"
         "'i'",    "Callback is triggered when the object temperature is *inside* the min and max values"
         "'<'",    "Callback is triggered when the object temperature is smaller than the min value (max is ignored)"
         "'>'",    "Callback is triggered when the object temperature is greater than the min value (max is ignored)"
        """
        self.check_validity()

        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickletTemperatureIR.FUNCTION_SET_OBJECT_TEMPERATURE_CALLBACK_THRESHOLD, (option, min, max), 'c h h', 0, '')

    def get_object_temperature_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Object Temperature Callback Threshold`.
        """
        self.check_validity()

        return GetObjectTemperatureCallbackThreshold(*self.ipcon.send_request(self, BrickletTemperatureIR.FUNCTION_GET_OBJECT_TEMPERATURE_CALLBACK_THRESHOLD, (), '', 13, 'c h h'))

    def set_debounce_period(self, debounce):
        r"""
        Sets the period with which the threshold callbacks

        * :cb:`Ambient Temperature Reached`,
        * :cb:`Object Temperature Reached`

        are triggered, if the thresholds

        * :func:`Set Ambient Temperature Callback Threshold`,
        * :func:`Set Object Temperature Callback Threshold`

        keep being reached.
        """
        self.check_validity()

        debounce = int(debounce)

        self.ipcon.send_request(self, BrickletTemperatureIR.FUNCTION_SET_DEBOUNCE_PERIOD, (debounce,), 'I', 0, '')

    def get_debounce_period(self):
        r"""
        Returns the debounce period as set by :func:`Set Debounce Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletTemperatureIR.FUNCTION_GET_DEBOUNCE_PERIOD, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletTemperatureIR.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

TemperatureIR = BrickletTemperatureIR # for backward compatibility
