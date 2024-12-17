# -*- coding: utf-8 -*-
#############################################################
# This file was automatically generated on 2024-12-17.      #
#                                                           #
# Python Bindings Version 2.1.31                            #
#                                                           #
# If you have a bugfix for this file and want to commit it, #
# please fix the bug in the generator. You can find a link  #
# to the generators git repository on tinkerforge.com       #
#############################################################

#### __DEVICE_IS_NOT_RELEASED__ ####

from collections import namedtuple

try:
    from .ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data
except (ValueError, ImportError):
    try:
        from ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data
    except (ValueError, ImportError):
        from tinkerforge.ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data

GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletCommonTest(Device):
    r"""

    """

    DEVICE_IDENTIFIER = 21112
    DEVICE_DISPLAY_NAME = 'Common Test Bricklet'
    DEVICE_URL_PART = 'common_test' # internal



    FUNCTION_SET_INT8_VALUE = 1
    FUNCTION_GET_INT8_VALUE = 2
    FUNCTION_GET_IDENTITY = 255


    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletCommonTest.DEVICE_IDENTIFIER, BrickletCommonTest.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletCommonTest.FUNCTION_SET_INT8_VALUE] = BrickletCommonTest.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletCommonTest.FUNCTION_GET_INT8_VALUE] = BrickletCommonTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCommonTest.FUNCTION_GET_IDENTITY] = BrickletCommonTest.RESPONSE_EXPECTED_ALWAYS_TRUE


        ipcon.add_device(self)

    def set_int8_value(self, value):
        r"""

        """
        self.check_validity()

        value = int(value)

        self.ipcon.send_request(self, BrickletCommonTest.FUNCTION_SET_INT8_VALUE, (value,), 'b', 0, '')

    def get_int8_value(self):
        r"""

        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletCommonTest.FUNCTION_GET_INT8_VALUE, (), '', 9, 'b')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletCommonTest.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

CommonTest = BrickletCommonTest # for backward compatibility
