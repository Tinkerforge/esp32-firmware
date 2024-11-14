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

GetRGBValue = namedtuple('RGBValue', ['r', 'g', 'b'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletRGBLED(Device):
    r"""
    Controls one RGB LED
    """

    DEVICE_IDENTIFIER = 271
    DEVICE_DISPLAY_NAME = 'RGB LED Bricklet'
    DEVICE_URL_PART = 'rgb_led' # internal



    FUNCTION_SET_RGB_VALUE = 1
    FUNCTION_GET_RGB_VALUE = 2
    FUNCTION_GET_IDENTITY = 255


    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletRGBLED.DEVICE_IDENTIFIER, BrickletRGBLED.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletRGBLED.FUNCTION_SET_RGB_VALUE] = BrickletRGBLED.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRGBLED.FUNCTION_GET_RGB_VALUE] = BrickletRGBLED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRGBLED.FUNCTION_GET_IDENTITY] = BrickletRGBLED.RESPONSE_EXPECTED_ALWAYS_TRUE


        ipcon.add_device(self)

    def set_rgb_value(self, r, g, b):
        r"""
        Sets the *r*, *g* and *b* values for the LED.
        """
        self.check_validity()

        r = int(r)
        g = int(g)
        b = int(b)

        self.ipcon.send_request(self, BrickletRGBLED.FUNCTION_SET_RGB_VALUE, (r, g, b), 'B B B', 0, '')

    def get_rgb_value(self):
        r"""
        Returns the *r*, *g* and *b* values of the LED as set by :func:`Set RGB Value`.
        """
        self.check_validity()

        return GetRGBValue(*self.ipcon.send_request(self, BrickletRGBLED.FUNCTION_GET_RGB_VALUE, (), '', 11, 'B B B'))

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletRGBLED.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

RGBLED = BrickletRGBLED # for backward compatibility
