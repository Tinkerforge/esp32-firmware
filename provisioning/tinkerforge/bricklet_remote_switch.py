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

class BrickletRemoteSwitch(Device):
    r"""
    Controls remote mains switches
    """

    DEVICE_IDENTIFIER = 235
    DEVICE_DISPLAY_NAME = 'Remote Switch Bricklet'
    DEVICE_URL_PART = 'remote_switch' # internal

    CALLBACK_SWITCHING_DONE = 3


    FUNCTION_SWITCH_SOCKET = 1
    FUNCTION_GET_SWITCHING_STATE = 2
    FUNCTION_SET_REPEATS = 4
    FUNCTION_GET_REPEATS = 5
    FUNCTION_SWITCH_SOCKET_A = 6
    FUNCTION_SWITCH_SOCKET_B = 7
    FUNCTION_DIM_SOCKET_B = 8
    FUNCTION_SWITCH_SOCKET_C = 9
    FUNCTION_GET_IDENTITY = 255

    SWITCH_TO_OFF = 0
    SWITCH_TO_ON = 1
    SWITCHING_STATE_READY = 0
    SWITCHING_STATE_BUSY = 1

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletRemoteSwitch.DEVICE_IDENTIFIER, BrickletRemoteSwitch.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 1)

        self.response_expected[BrickletRemoteSwitch.FUNCTION_SWITCH_SOCKET] = BrickletRemoteSwitch.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRemoteSwitch.FUNCTION_GET_SWITCHING_STATE] = BrickletRemoteSwitch.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRemoteSwitch.FUNCTION_SET_REPEATS] = BrickletRemoteSwitch.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRemoteSwitch.FUNCTION_GET_REPEATS] = BrickletRemoteSwitch.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRemoteSwitch.FUNCTION_SWITCH_SOCKET_A] = BrickletRemoteSwitch.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRemoteSwitch.FUNCTION_SWITCH_SOCKET_B] = BrickletRemoteSwitch.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRemoteSwitch.FUNCTION_DIM_SOCKET_B] = BrickletRemoteSwitch.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRemoteSwitch.FUNCTION_SWITCH_SOCKET_C] = BrickletRemoteSwitch.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRemoteSwitch.FUNCTION_GET_IDENTITY] = BrickletRemoteSwitch.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletRemoteSwitch.CALLBACK_SWITCHING_DONE] = (8, '')

        ipcon.add_device(self)

    def switch_socket(self, house_code, receiver_code, switch_to):
        r"""
        This function is deprecated, use :func:`Switch Socket A` instead.
        """
        self.check_validity()

        house_code = int(house_code)
        receiver_code = int(receiver_code)
        switch_to = int(switch_to)

        self.ipcon.send_request(self, BrickletRemoteSwitch.FUNCTION_SWITCH_SOCKET, (house_code, receiver_code, switch_to), 'B B B', 0, '')

    def get_switching_state(self):
        r"""
        Returns the current switching state. If the current state is busy, the
        Bricklet is currently sending a code to switch a socket. It will not
        accept any requests to switch sockets until the state changes to ready.

        How long the switching takes is dependent on the number of repeats, see
        :func:`Set Repeats`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRemoteSwitch.FUNCTION_GET_SWITCHING_STATE, (), '', 9, 'B')

    def set_repeats(self, repeats):
        r"""
        Sets the number of times the code is sent when one of the switch socket
        functions is called. The repeats basically correspond to the amount of time
        that a button of the remote is pressed.

        Some dimmers are controlled by the length of a button pressed,
        this can be simulated by increasing the repeats.
        """
        self.check_validity()

        repeats = int(repeats)

        self.ipcon.send_request(self, BrickletRemoteSwitch.FUNCTION_SET_REPEATS, (repeats,), 'B', 0, '')

    def get_repeats(self):
        r"""
        Returns the number of repeats as set by :func:`Set Repeats`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRemoteSwitch.FUNCTION_GET_REPEATS, (), '', 9, 'B')

    def switch_socket_a(self, house_code, receiver_code, switch_to):
        r"""
        To switch a type A socket you have to give the house code, receiver code and the
        state (on or off) you want to switch to.

        A detailed description on how you can figure out the house and receiver code
        can be found :ref:`here <remote_switch_bricklet_type_a_house_and_receiver_code>`.

        .. versionadded:: 2.0.1$nbsp;(Plugin)
        """
        self.check_validity()

        house_code = int(house_code)
        receiver_code = int(receiver_code)
        switch_to = int(switch_to)

        self.ipcon.send_request(self, BrickletRemoteSwitch.FUNCTION_SWITCH_SOCKET_A, (house_code, receiver_code, switch_to), 'B B B', 0, '')

    def switch_socket_b(self, address, unit, switch_to):
        r"""
        To switch a type B socket you have to give the address, unit and the state
        (on or off) you want to switch to.

        To switch all devices with the same address use 255 for the unit.

        A detailed description on how you can teach a socket the address and unit can
        be found :ref:`here <remote_switch_bricklet_type_b_address_and_unit>`.

        .. versionadded:: 2.0.1$nbsp;(Plugin)
        """
        self.check_validity()

        address = int(address)
        unit = int(unit)
        switch_to = int(switch_to)

        self.ipcon.send_request(self, BrickletRemoteSwitch.FUNCTION_SWITCH_SOCKET_B, (address, unit, switch_to), 'I B B', 0, '')

    def dim_socket_b(self, address, unit, dim_value):
        r"""
        To control a type B dimmer you have to give the address, unit and the
        dim value you want to set the dimmer to.

        A detailed description on how you can teach a dimmer the address and unit can
        be found :ref:`here <remote_switch_bricklet_type_b_address_and_unit>`.

        .. versionadded:: 2.0.1$nbsp;(Plugin)
        """
        self.check_validity()

        address = int(address)
        unit = int(unit)
        dim_value = int(dim_value)

        self.ipcon.send_request(self, BrickletRemoteSwitch.FUNCTION_DIM_SOCKET_B, (address, unit, dim_value), 'I B B', 0, '')

    def switch_socket_c(self, system_code, device_code, switch_to):
        r"""
        To switch a type C socket you have to give the system code, device code and the
        state (on or off) you want to switch to.

        A detailed description on how you can figure out the system and device code
        can be found :ref:`here <remote_switch_bricklet_type_c_system_and_device_code>`.

        .. versionadded:: 2.0.1$nbsp;(Plugin)
        """
        self.check_validity()

        system_code = create_char(system_code)
        device_code = int(device_code)
        switch_to = int(switch_to)

        self.ipcon.send_request(self, BrickletRemoteSwitch.FUNCTION_SWITCH_SOCKET_C, (system_code, device_code, switch_to), 'c B B', 0, '')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletRemoteSwitch.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

RemoteSwitch = BrickletRemoteSwitch # for backward compatibility
