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

class BrickUnknownV2(Device):
    r"""

    """

    DEVICE_IDENTIFIER = -11
    DEVICE_DISPLAY_NAME = 'Unknown Brick 2.0'
    DEVICE_URL_PART = 'unknown_v2' # internal



    FUNCTION_GET_IDENTITY = 255


    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickUnknownV2.DEVICE_IDENTIFIER, BrickUnknownV2.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickUnknownV2.FUNCTION_GET_IDENTITY] = BrickUnknownV2.RESPONSE_EXPECTED_ALWAYS_TRUE


        ipcon.add_device(self)

    def get_identity(self):
        r"""
        Returns the UID, the UID where the Brick is connected to,
        the position, the hardware and firmware version as well as the
        device identifier.

        The position is the position in the stack from '0' (bottom) to '8' (top).

        The device identifier numbers can be found :ref:`here <device_identifier>`.
        |device_identifier_constant|
        """
        return GetIdentity(*self.ipcon.send_request(self, BrickUnknownV2.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

UnknownV2 = BrickUnknownV2 # for backward compatibility
