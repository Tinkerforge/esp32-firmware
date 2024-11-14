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

class BrickletAnalogOut(Device):
    r"""
    Generates configurable DC voltage between 0V and 5V
    """

    DEVICE_IDENTIFIER = 220
    DEVICE_DISPLAY_NAME = 'Analog Out Bricklet'
    DEVICE_URL_PART = 'analog_out' # internal



    FUNCTION_SET_VOLTAGE = 1
    FUNCTION_GET_VOLTAGE = 2
    FUNCTION_SET_MODE = 3
    FUNCTION_GET_MODE = 4
    FUNCTION_GET_IDENTITY = 255

    MODE_ANALOG_VALUE = 0
    MODE_1K_TO_GROUND = 1
    MODE_100K_TO_GROUND = 2
    MODE_500K_TO_GROUND = 3

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletAnalogOut.DEVICE_IDENTIFIER, BrickletAnalogOut.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletAnalogOut.FUNCTION_SET_VOLTAGE] = BrickletAnalogOut.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAnalogOut.FUNCTION_GET_VOLTAGE] = BrickletAnalogOut.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogOut.FUNCTION_SET_MODE] = BrickletAnalogOut.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAnalogOut.FUNCTION_GET_MODE] = BrickletAnalogOut.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAnalogOut.FUNCTION_GET_IDENTITY] = BrickletAnalogOut.RESPONSE_EXPECTED_ALWAYS_TRUE


        ipcon.add_device(self)

    def set_voltage(self, voltage):
        r"""
        Sets the voltage. Calling this function will set
        the mode to 0 (see :func:`Set Mode`).
        """
        self.check_validity()

        voltage = int(voltage)

        self.ipcon.send_request(self, BrickletAnalogOut.FUNCTION_SET_VOLTAGE, (voltage,), 'H', 0, '')

    def get_voltage(self):
        r"""
        Returns the voltage as set by :func:`Set Voltage`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAnalogOut.FUNCTION_GET_VOLTAGE, (), '', 10, 'H')

    def set_mode(self, mode):
        r"""
        Sets the mode of the analog value. Possible modes:

        * 0: Normal Mode (Analog value as set by :func:`Set Voltage` is applied)
        * 1: 1k Ohm resistor to ground
        * 2: 100k Ohm resistor to ground
        * 3: 500k Ohm resistor to ground

        Setting the mode to 0 will result in an output voltage of 0 V. You can jump
        to a higher output voltage directly by calling :func:`Set Voltage`.
        """
        self.check_validity()

        mode = int(mode)

        self.ipcon.send_request(self, BrickletAnalogOut.FUNCTION_SET_MODE, (mode,), 'B', 0, '')

    def get_mode(self):
        r"""
        Returns the mode as set by :func:`Set Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAnalogOut.FUNCTION_GET_MODE, (), '', 9, 'B')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletAnalogOut.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

AnalogOut = BrickletAnalogOut # for backward compatibility
