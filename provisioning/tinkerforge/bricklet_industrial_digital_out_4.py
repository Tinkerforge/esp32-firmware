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

GetMonoflop = namedtuple('Monoflop', ['value', 'time', 'time_remaining'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletIndustrialDigitalOut4(Device):
    r"""
    4 galvanically isolated digital outputs
    """

    DEVICE_IDENTIFIER = 224
    DEVICE_DISPLAY_NAME = 'Industrial Digital Out 4 Bricklet'
    DEVICE_URL_PART = 'industrial_digital_out_4' # internal

    CALLBACK_MONOFLOP_DONE = 8


    FUNCTION_SET_VALUE = 1
    FUNCTION_GET_VALUE = 2
    FUNCTION_SET_MONOFLOP = 3
    FUNCTION_GET_MONOFLOP = 4
    FUNCTION_SET_GROUP = 5
    FUNCTION_GET_GROUP = 6
    FUNCTION_GET_AVAILABLE_FOR_GROUP = 7
    FUNCTION_SET_SELECTED_VALUES = 9
    FUNCTION_GET_IDENTITY = 255


    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletIndustrialDigitalOut4.DEVICE_IDENTIFIER, BrickletIndustrialDigitalOut4.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletIndustrialDigitalOut4.FUNCTION_SET_VALUE] = BrickletIndustrialDigitalOut4.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDigitalOut4.FUNCTION_GET_VALUE] = BrickletIndustrialDigitalOut4.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDigitalOut4.FUNCTION_SET_MONOFLOP] = BrickletIndustrialDigitalOut4.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDigitalOut4.FUNCTION_GET_MONOFLOP] = BrickletIndustrialDigitalOut4.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDigitalOut4.FUNCTION_SET_GROUP] = BrickletIndustrialDigitalOut4.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDigitalOut4.FUNCTION_GET_GROUP] = BrickletIndustrialDigitalOut4.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDigitalOut4.FUNCTION_GET_AVAILABLE_FOR_GROUP] = BrickletIndustrialDigitalOut4.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDigitalOut4.FUNCTION_SET_SELECTED_VALUES] = BrickletIndustrialDigitalOut4.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDigitalOut4.FUNCTION_GET_IDENTITY] = BrickletIndustrialDigitalOut4.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletIndustrialDigitalOut4.CALLBACK_MONOFLOP_DONE] = (12, 'H H')

        ipcon.add_device(self)

    def set_value(self, value_mask):
        r"""
        Sets the output value with a bitmask (16bit). A 1 in the bitmask means high
        and a 0 in the bitmask means low.

        For example: The value 3 or 0b0011 will turn pins 0-1 high and the other pins
        low.

        If no groups are used (see :func:`Set Group`), the pins correspond to the
        markings on the Industrial Digital Out 4 Bricklet.

        If groups are used, the pins correspond to the element in the group.
        Element 1 in the group will get pins 0-3, element 2 pins 4-7, element 3
        pins 8-11 and element 4 pins 12-15.

        All running monoflop timers will be aborted if this function is called.
        """
        self.check_validity()

        value_mask = int(value_mask)

        self.ipcon.send_request(self, BrickletIndustrialDigitalOut4.FUNCTION_SET_VALUE, (value_mask,), 'H', 0, '')

    def get_value(self):
        r"""
        Returns the bitmask as set by :func:`Set Value`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDigitalOut4.FUNCTION_GET_VALUE, (), '', 10, 'H')

    def set_monoflop(self, selection_mask, value_mask, time):
        r"""
        Configures a monoflop of the pins specified by the first parameter
        bitmask.

        The second parameter is a bitmask with the desired value of the specified
        pins. A 1 in the bitmask means high and a 0 in the bitmask means low.

        The third parameter indicates the time that the pins should hold
        the value.

        If this function is called with the parameters (9, 1, 1500) or
        (0b1001, 0b0001, 1500): Pin 0 will get high and pin 3 will get low. In 1.5s
        pin 0 will get low and pin 3 will get high again.

        A monoflop can be used as a fail-safe mechanism. For example: Lets assume you
        have a RS485 bus and a Digital Out 4 Bricklet connected to one of the slave
        stacks. You can now call this function every second, with a time parameter
        of two seconds and pin 0 high. Pin 0 will be high all the time. If now
        the RS485 connection is lost, then pin 0 will turn low in at most two seconds.
        """
        self.check_validity()

        selection_mask = int(selection_mask)
        value_mask = int(value_mask)
        time = int(time)

        self.ipcon.send_request(self, BrickletIndustrialDigitalOut4.FUNCTION_SET_MONOFLOP, (selection_mask, value_mask, time), 'H H I', 0, '')

    def get_monoflop(self, pin):
        r"""
        Returns (for the given pin) the current value and the time as set by
        :func:`Set Monoflop` as well as the remaining time until the value flips.

        If the timer is not running currently, the remaining time will be returned
        as 0.
        """
        self.check_validity()

        pin = int(pin)

        return GetMonoflop(*self.ipcon.send_request(self, BrickletIndustrialDigitalOut4.FUNCTION_GET_MONOFLOP, (pin,), 'B', 18, 'H I I'))

    def set_group(self, group):
        r"""
        Sets a group of Digital Out 4 Bricklets that should work together. You can
        find Bricklets that can be grouped together with :func:`Get Available For Group`.

        The group consists of 4 elements. Element 1 in the group will get pins 0-3,
        element 2 pins 4-7, element 3 pins 8-11 and element 4 pins 12-15.

        Each element can either be one of the ports ('a' to 'd') or 'n' if it should
        not be used.

        For example: If you have two Digital Out 4 Bricklets connected to port A and
        port B respectively, you could call with ``['a', 'b', 'n', 'n']``.

        Now the pins on the Digital Out 4 on port A are assigned to 0-3 and the
        pins on the Digital Out 4 on port B are assigned to 4-7. It is now possible
        to call :func:`Set Value` and control two Bricklets at the same time.
        """
        self.check_validity()

        group = create_char_list(group)

        self.ipcon.send_request(self, BrickletIndustrialDigitalOut4.FUNCTION_SET_GROUP, (group,), '4c', 0, '')

    def get_group(self):
        r"""
        Returns the group as set by :func:`Set Group`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDigitalOut4.FUNCTION_GET_GROUP, (), '', 12, '4c')

    def get_available_for_group(self):
        r"""
        Returns a bitmask of ports that are available for grouping. For example the
        value 5 or 0b0101 means: Port A and port C are connected to Bricklets that
        can be grouped together.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDigitalOut4.FUNCTION_GET_AVAILABLE_FOR_GROUP, (), '', 9, 'B')

    def set_selected_values(self, selection_mask, value_mask):
        r"""
        Sets the output value with a bitmask, according to the selection mask.
        The bitmask is 16 bit long, *true* refers to high and *false* refers to
        low.

        For example: The values (3, 1) or (0b0011, 0b0001) will turn pin 0 high, pin 1
        low the other pins remain untouched.

        If no groups are used (see :func:`Set Group`), the pins correspond to the
        markings on the Industrial Digital Out 4 Bricklet.

        If groups are used, the pins correspond to the element in the group.
        Element 1 in the group will get pins 0-3, element 2 pins 4-7, element 3
        pins 8-11 and element 4 pins 12-15.

        Running monoflop timers for the selected pins will be aborted if this function
        is called.
        """
        self.check_validity()

        selection_mask = int(selection_mask)
        value_mask = int(value_mask)

        self.ipcon.send_request(self, BrickletIndustrialDigitalOut4.FUNCTION_SET_SELECTED_VALUES, (selection_mask, value_mask), 'H H', 0, '')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletIndustrialDigitalOut4.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

IndustrialDigitalOut4 = BrickletIndustrialDigitalOut4 # for backward compatibility
