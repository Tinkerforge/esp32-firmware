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

GetEdgeCountConfig = namedtuple('EdgeCountConfig', ['edge_type', 'debounce'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletIndustrialDigitalIn4(Device):
    r"""
    4 galvanically isolated digital inputs
    """

    DEVICE_IDENTIFIER = 223
    DEVICE_DISPLAY_NAME = 'Industrial Digital In 4 Bricklet'
    DEVICE_URL_PART = 'industrial_digital_in_4' # internal

    CALLBACK_INTERRUPT = 9


    FUNCTION_GET_VALUE = 1
    FUNCTION_SET_GROUP = 2
    FUNCTION_GET_GROUP = 3
    FUNCTION_GET_AVAILABLE_FOR_GROUP = 4
    FUNCTION_SET_DEBOUNCE_PERIOD = 5
    FUNCTION_GET_DEBOUNCE_PERIOD = 6
    FUNCTION_SET_INTERRUPT = 7
    FUNCTION_GET_INTERRUPT = 8
    FUNCTION_GET_EDGE_COUNT = 10
    FUNCTION_SET_EDGE_COUNT_CONFIG = 11
    FUNCTION_GET_EDGE_COUNT_CONFIG = 12
    FUNCTION_GET_IDENTITY = 255

    EDGE_TYPE_RISING = 0
    EDGE_TYPE_FALLING = 1
    EDGE_TYPE_BOTH = 2

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletIndustrialDigitalIn4.DEVICE_IDENTIFIER, BrickletIndustrialDigitalIn4.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 1)

        self.response_expected[BrickletIndustrialDigitalIn4.FUNCTION_GET_VALUE] = BrickletIndustrialDigitalIn4.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDigitalIn4.FUNCTION_SET_GROUP] = BrickletIndustrialDigitalIn4.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDigitalIn4.FUNCTION_GET_GROUP] = BrickletIndustrialDigitalIn4.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDigitalIn4.FUNCTION_GET_AVAILABLE_FOR_GROUP] = BrickletIndustrialDigitalIn4.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDigitalIn4.FUNCTION_SET_DEBOUNCE_PERIOD] = BrickletIndustrialDigitalIn4.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIndustrialDigitalIn4.FUNCTION_GET_DEBOUNCE_PERIOD] = BrickletIndustrialDigitalIn4.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDigitalIn4.FUNCTION_SET_INTERRUPT] = BrickletIndustrialDigitalIn4.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIndustrialDigitalIn4.FUNCTION_GET_INTERRUPT] = BrickletIndustrialDigitalIn4.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDigitalIn4.FUNCTION_GET_EDGE_COUNT] = BrickletIndustrialDigitalIn4.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDigitalIn4.FUNCTION_SET_EDGE_COUNT_CONFIG] = BrickletIndustrialDigitalIn4.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDigitalIn4.FUNCTION_GET_EDGE_COUNT_CONFIG] = BrickletIndustrialDigitalIn4.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDigitalIn4.FUNCTION_GET_IDENTITY] = BrickletIndustrialDigitalIn4.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletIndustrialDigitalIn4.CALLBACK_INTERRUPT] = (12, 'H H')

        ipcon.add_device(self)

    def get_value(self):
        r"""
        Returns the input value with a bitmask. The bitmask is 16bit long, *true*
        refers to high and *false* refers to low.

        For example: The value 3 or 0b0011 means that pins 0-1 are high and the other
        pins are low.

        If no groups are used (see :func:`Set Group`), the pins correspond to the
        markings on the IndustrialDigital In 4 Bricklet.

        If groups are used, the pins correspond to the element in the group.
        Element 1 in the group will get pins 0-3, element 2 pins 4-7, element 3
        pins 8-11 and element 4 pins 12-15.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDigitalIn4.FUNCTION_GET_VALUE, (), '', 10, 'H')

    def set_group(self, group):
        r"""
        Sets a group of Digital In 4 Bricklets that should work together. You can
        find Bricklets that can be grouped together with :func:`Get Available For Group`.

        The group consists of 4 elements. Element 1 in the group will get pins 0-3,
        element 2 pins 4-7, element 3 pins 8-11 and element 4 pins 12-15.

        Each element can either be one of the ports ('a' to 'd') or 'n' if it should
        not be used.

        For example: If you have two Digital In 4 Bricklets connected to port A and
        port B respectively, you could call with ``['a', 'b', 'n', 'n']``.

        Now the pins on the Digital In 4 on port A are assigned to 0-3 and the
        pins on the Digital In 4 on port B are assigned to 4-7. It is now possible
        to call :func:`Get Value` and read out two Bricklets at the same time.

        Changing the group configuration resets all edge counter configurations
        and values.
        """
        self.check_validity()

        group = create_char_list(group)

        self.ipcon.send_request(self, BrickletIndustrialDigitalIn4.FUNCTION_SET_GROUP, (group,), '4c', 0, '')

    def get_group(self):
        r"""
        Returns the group as set by :func:`Set Group`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDigitalIn4.FUNCTION_GET_GROUP, (), '', 12, '4c')

    def get_available_for_group(self):
        r"""
        Returns a bitmask of ports that are available for grouping. For example the
        value 5 or 0b0101 means: Port A and port C are connected to Bricklets that
        can be grouped together.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDigitalIn4.FUNCTION_GET_AVAILABLE_FOR_GROUP, (), '', 9, 'B')

    def set_debounce_period(self, debounce):
        r"""
        Sets the debounce period of the :cb:`Interrupt` callback.

        For example: If you set this value to 100, you will get the interrupt
        maximal every 100ms. This is necessary if something that bounces is
        connected to the Digital In 4 Bricklet, such as a button.
        """
        self.check_validity()

        debounce = int(debounce)

        self.ipcon.send_request(self, BrickletIndustrialDigitalIn4.FUNCTION_SET_DEBOUNCE_PERIOD, (debounce,), 'I', 0, '')

    def get_debounce_period(self):
        r"""
        Returns the debounce period as set by :func:`Set Debounce Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDigitalIn4.FUNCTION_GET_DEBOUNCE_PERIOD, (), '', 12, 'I')

    def set_interrupt(self, interrupt_mask):
        r"""
        Sets the pins on which an interrupt is activated with a bitmask.
        Interrupts are triggered on changes of the voltage level of the pin,
        i.e. changes from high to low and low to high.

        For example: An interrupt bitmask of 9 or 0b1001 will enable the interrupt for
        pins 0 and 3.

        The interrupts use the grouping as set by :func:`Set Group`.

        The interrupt is delivered with the :cb:`Interrupt` callback.
        """
        self.check_validity()

        interrupt_mask = int(interrupt_mask)

        self.ipcon.send_request(self, BrickletIndustrialDigitalIn4.FUNCTION_SET_INTERRUPT, (interrupt_mask,), 'H', 0, '')

    def get_interrupt(self):
        r"""
        Returns the interrupt bitmask as set by :func:`Set Interrupt`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDigitalIn4.FUNCTION_GET_INTERRUPT, (), '', 10, 'H')

    def get_edge_count(self, pin, reset_counter):
        r"""
        Returns the current value of the edge counter for the selected pin. You can
        configure the edges that are counted with :func:`Set Edge Count Config`.

        If you set the reset counter to *true*, the count is set back to 0
        directly after it is read.

        The edge counters use the grouping as set by :func:`Set Group`.

        .. versionadded:: 2.0.1$nbsp;(Plugin)
        """
        self.check_validity()

        pin = int(pin)
        reset_counter = bool(reset_counter)

        return self.ipcon.send_request(self, BrickletIndustrialDigitalIn4.FUNCTION_GET_EDGE_COUNT, (pin, reset_counter), 'B !', 12, 'I')

    def set_edge_count_config(self, selection_mask, edge_type, debounce):
        r"""
        Configures the edge counter for the selected pins. A bitmask of 9 or 0b1001 will
        enable the edge counter for pins 0 and 3.

        The edge type parameter configures if rising edges, falling edges or
        both are counted if the pin is configured for input. Possible edge types are:

        * 0 = rising
        * 1 = falling
        * 2 = both

        Configuring an edge counter resets its value to 0.

        If you don't know what any of this means, just leave it at default. The
        default configuration is very likely OK for you.

        The edge counters use the grouping as set by :func:`Set Group`.

        .. versionadded:: 2.0.1$nbsp;(Plugin)
        """
        self.check_validity()

        selection_mask = int(selection_mask)
        edge_type = int(edge_type)
        debounce = int(debounce)

        self.ipcon.send_request(self, BrickletIndustrialDigitalIn4.FUNCTION_SET_EDGE_COUNT_CONFIG, (selection_mask, edge_type, debounce), 'H B B', 0, '')

    def get_edge_count_config(self, pin):
        r"""
        Returns the edge type and debounce time for the selected pin as set by
        :func:`Set Edge Count Config`.

        .. versionadded:: 2.0.1$nbsp;(Plugin)
        """
        self.check_validity()

        pin = int(pin)

        return GetEdgeCountConfig(*self.ipcon.send_request(self, BrickletIndustrialDigitalIn4.FUNCTION_GET_EDGE_COUNT_CONFIG, (pin,), 'B', 10, 'B B'))

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletIndustrialDigitalIn4.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

IndustrialDigitalIn4 = BrickletIndustrialDigitalIn4 # for backward compatibility
