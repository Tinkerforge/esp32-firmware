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

GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletAnalogOutV2(Device):
    r"""
    Generates configurable DC voltage between 0V and 12V
    """

    DEVICE_IDENTIFIER = 256
    DEVICE_DISPLAY_NAME = 'Analog Out Bricklet 2.0'
    DEVICE_URL_PART = 'analog_out_v2' # internal



    FUNCTION_SET_OUTPUT_VOLTAGE = 1
    FUNCTION_GET_OUTPUT_VOLTAGE = 2
    FUNCTION_GET_INPUT_VOLTAGE = 3
    FUNCTION_GET_IDENTITY = 255


    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletAnalogOutV2.DEVICE_IDENTIFIER, BrickletAnalogOutV2.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletAnalogOutV2.FUNCTION_SET_OUTPUT_VOLTAGE] = BrickletAnalogOutV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAnalogOutV2.FUNCTION_GET_OUTPUT_VOLTAGE] = BrickletAnalogOutV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogOutV2.FUNCTION_GET_INPUT_VOLTAGE] = BrickletAnalogOutV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogOutV2.FUNCTION_GET_IDENTITY] = BrickletAnalogOutV2.RESPONSE_EXPECTED_ALWAYS_TRUE


        ipcon.add_device(self)

    def set_output_voltage(self, voltage):
        r"""
        Sets the voltage.
        """
        self.check_validity()

        voltage = int(voltage)

        self.ipcon.send_request(self, BrickletAnalogOutV2.FUNCTION_SET_OUTPUT_VOLTAGE, (voltage,), 'H', 0, '')

    def get_output_voltage(self):
        r"""
        Returns the voltage as set by :func:`Set Output Voltage`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAnalogOutV2.FUNCTION_GET_OUTPUT_VOLTAGE, (), '', 10, 'H')

    def get_input_voltage(self):
        r"""
        Returns the input voltage.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAnalogOutV2.FUNCTION_GET_INPUT_VOLTAGE, (), '', 10, 'H')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletAnalogOutV2.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

AnalogOutV2 = BrickletAnalogOutV2 # for backward compatibility
