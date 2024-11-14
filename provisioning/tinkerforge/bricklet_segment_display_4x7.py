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

GetSegments = namedtuple('Segments', ['segments', 'brightness', 'colon'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletSegmentDisplay4x7(Device):
    r"""
    Four 7-segment displays with switchable colon
    """

    DEVICE_IDENTIFIER = 237
    DEVICE_DISPLAY_NAME = 'Segment Display 4x7 Bricklet'
    DEVICE_URL_PART = 'segment_display_4x7' # internal

    CALLBACK_COUNTER_FINISHED = 5


    FUNCTION_SET_SEGMENTS = 1
    FUNCTION_GET_SEGMENTS = 2
    FUNCTION_START_COUNTER = 3
    FUNCTION_GET_COUNTER_VALUE = 4
    FUNCTION_GET_IDENTITY = 255


    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletSegmentDisplay4x7.DEVICE_IDENTIFIER, BrickletSegmentDisplay4x7.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletSegmentDisplay4x7.FUNCTION_SET_SEGMENTS] = BrickletSegmentDisplay4x7.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletSegmentDisplay4x7.FUNCTION_GET_SEGMENTS] = BrickletSegmentDisplay4x7.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletSegmentDisplay4x7.FUNCTION_START_COUNTER] = BrickletSegmentDisplay4x7.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletSegmentDisplay4x7.FUNCTION_GET_COUNTER_VALUE] = BrickletSegmentDisplay4x7.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletSegmentDisplay4x7.FUNCTION_GET_IDENTITY] = BrickletSegmentDisplay4x7.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletSegmentDisplay4x7.CALLBACK_COUNTER_FINISHED] = (8, '')

        ipcon.add_device(self)

    def set_segments(self, segments, brightness, colon):
        r"""
        The 7-segment display can be set with bitmaps. Every bit controls one
        segment:

        .. image:: /Images/Bricklets/bricklet_segment_display_4x7_bit_order.png
           :scale: 100 %
           :alt: Bit order of one segment
           :align: center

        For example to set a "5" you would want to activate segments 0, 2, 3, 5 and 6.
        This is represented by the number 0b01101101 = 0x6d = 109.

        The brightness can be set between 0 (dark) and 7 (bright). The colon
        parameter turns the colon of the display on or off.
        """
        self.check_validity()

        segments = list(map(int, segments))
        brightness = int(brightness)
        colon = bool(colon)

        self.ipcon.send_request(self, BrickletSegmentDisplay4x7.FUNCTION_SET_SEGMENTS, (segments, brightness, colon), '4B B !', 0, '')

    def get_segments(self):
        r"""
        Returns the segment, brightness and color data as set by
        :func:`Set Segments`.
        """
        self.check_validity()

        return GetSegments(*self.ipcon.send_request(self, BrickletSegmentDisplay4x7.FUNCTION_GET_SEGMENTS, (), '', 14, '4B B !'))

    def start_counter(self, value_from, value_to, increment, length):
        r"""
        Starts a counter with the *from* value that counts to the *to*
        value with the each step incremented by *increment*.
        *length* is the pause between each increment.

        Example: If you set *from* to 0, *to* to 100, *increment* to 1 and
        *length* to 1000, a counter that goes from 0 to 100 with one second
        pause between each increment will be started.

        Using a negative increment allows to count backwards.

        You can stop the counter at every time by calling :func:`Set Segments`.
        """
        self.check_validity()

        value_from = int(value_from)
        value_to = int(value_to)
        increment = int(increment)
        length = int(length)

        self.ipcon.send_request(self, BrickletSegmentDisplay4x7.FUNCTION_START_COUNTER, (value_from, value_to, increment, length), 'h h h I', 0, '')

    def get_counter_value(self):
        r"""
        Returns the counter value that is currently shown on the display.

        If there is no counter running a 0 will be returned.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletSegmentDisplay4x7.FUNCTION_GET_COUNTER_VALUE, (), '', 10, 'H')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletSegmentDisplay4x7.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

SegmentDisplay4x7 = BrickletSegmentDisplay4x7 # for backward compatibility
