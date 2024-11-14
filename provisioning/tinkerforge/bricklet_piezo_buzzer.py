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

class BrickletPiezoBuzzer(Device):
    r"""
    Creates 1kHz beep
    """

    DEVICE_IDENTIFIER = 214
    DEVICE_DISPLAY_NAME = 'Piezo Buzzer Bricklet'
    DEVICE_URL_PART = 'piezo_buzzer' # internal

    CALLBACK_BEEP_FINISHED = 3
    CALLBACK_MORSE_CODE_FINISHED = 4


    FUNCTION_BEEP = 1
    FUNCTION_MORSE_CODE = 2
    FUNCTION_GET_IDENTITY = 255


    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletPiezoBuzzer.DEVICE_IDENTIFIER, BrickletPiezoBuzzer.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletPiezoBuzzer.FUNCTION_BEEP] = BrickletPiezoBuzzer.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPiezoBuzzer.FUNCTION_MORSE_CODE] = BrickletPiezoBuzzer.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPiezoBuzzer.FUNCTION_GET_IDENTITY] = BrickletPiezoBuzzer.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletPiezoBuzzer.CALLBACK_BEEP_FINISHED] = (8, '')
        self.callback_formats[BrickletPiezoBuzzer.CALLBACK_MORSE_CODE_FINISHED] = (8, '')

        ipcon.add_device(self)

    def beep(self, duration):
        r"""
        Beeps for the given duration.
        """
        self.check_validity()

        duration = int(duration)

        self.ipcon.send_request(self, BrickletPiezoBuzzer.FUNCTION_BEEP, (duration,), 'I', 0, '')

    def morse_code(self, morse):
        r"""
        Sets morse code that will be played by the piezo buzzer. The morse code
        is given as a string consisting of "." (dot), "-" (minus) and " " (space)
        for *dits*, *dahs* and *pauses*. Every other character is ignored.

        For example: If you set the string "...---...", the piezo buzzer will beep
        nine times with the durations "short short short long long long short
        short short".
        """
        self.check_validity()

        morse = create_string(morse)

        self.ipcon.send_request(self, BrickletPiezoBuzzer.FUNCTION_MORSE_CODE, (morse,), '60s', 0, '')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletPiezoBuzzer.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

PiezoBuzzer = BrickletPiezoBuzzer # for backward compatibility
