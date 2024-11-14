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

class BrickletTilt(Device):
    r"""
    Detects inclination of Bricklet (tilt switch open/closed)
    """

    DEVICE_IDENTIFIER = 239
    DEVICE_DISPLAY_NAME = 'Tilt Bricklet'
    DEVICE_URL_PART = 'tilt' # internal

    CALLBACK_TILT_STATE = 5


    FUNCTION_GET_TILT_STATE = 1
    FUNCTION_ENABLE_TILT_STATE_CALLBACK = 2
    FUNCTION_DISABLE_TILT_STATE_CALLBACK = 3
    FUNCTION_IS_TILT_STATE_CALLBACK_ENABLED = 4
    FUNCTION_GET_IDENTITY = 255

    TILT_STATE_CLOSED = 0
    TILT_STATE_OPEN = 1
    TILT_STATE_CLOSED_VIBRATING = 2

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletTilt.DEVICE_IDENTIFIER, BrickletTilt.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletTilt.FUNCTION_GET_TILT_STATE] = BrickletTilt.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletTilt.FUNCTION_ENABLE_TILT_STATE_CALLBACK] = BrickletTilt.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletTilt.FUNCTION_DISABLE_TILT_STATE_CALLBACK] = BrickletTilt.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletTilt.FUNCTION_IS_TILT_STATE_CALLBACK_ENABLED] = BrickletTilt.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletTilt.FUNCTION_GET_IDENTITY] = BrickletTilt.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletTilt.CALLBACK_TILT_STATE] = (9, 'B')

        ipcon.add_device(self)

    def get_tilt_state(self):
        r"""
        Returns the current tilt state. The state can either be

        * 0 = Closed: The ball in the tilt switch closes the circuit.
        * 1 = Open: The ball in the tilt switch does not close the circuit.
        * 2 = Closed Vibrating: The tilt switch is in motion (rapid change between open and close).

        .. image:: /Images/Bricklets/bricklet_tilt_mechanics.jpg
           :scale: 100 %
           :alt: Tilt states
           :align: center
           :target: ../../_images/Bricklets/bricklet_tilt_mechanics.jpg
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletTilt.FUNCTION_GET_TILT_STATE, (), '', 9, 'B')

    def enable_tilt_state_callback(self):
        r"""
        Enables the :cb:`Tilt State` callback.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletTilt.FUNCTION_ENABLE_TILT_STATE_CALLBACK, (), '', 0, '')

    def disable_tilt_state_callback(self):
        r"""
        Disables the :cb:`Tilt State` callback.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletTilt.FUNCTION_DISABLE_TILT_STATE_CALLBACK, (), '', 0, '')

    def is_tilt_state_callback_enabled(self):
        r"""
        Returns *true* if the :cb:`Tilt State` callback is enabled.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletTilt.FUNCTION_IS_TILT_STATE_CALLBACK_ENABLED, (), '', 9, '!')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletTilt.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

Tilt = BrickletTilt # for backward compatibility
