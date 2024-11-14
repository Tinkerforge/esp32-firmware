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

GetLEDState = namedtuple('LEDState', ['led_l', 'led_r'])
GetButtonState = namedtuple('ButtonState', ['button_l', 'button_r'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletDualButton(Device):
    r"""
    Two tactile buttons with built-in blue LEDs
    """

    DEVICE_IDENTIFIER = 230
    DEVICE_DISPLAY_NAME = 'Dual Button Bricklet'
    DEVICE_URL_PART = 'dual_button' # internal

    CALLBACK_STATE_CHANGED = 4


    FUNCTION_SET_LED_STATE = 1
    FUNCTION_GET_LED_STATE = 2
    FUNCTION_GET_BUTTON_STATE = 3
    FUNCTION_SET_SELECTED_LED_STATE = 5
    FUNCTION_GET_IDENTITY = 255

    LED_STATE_AUTO_TOGGLE_ON = 0
    LED_STATE_AUTO_TOGGLE_OFF = 1
    LED_STATE_ON = 2
    LED_STATE_OFF = 3
    BUTTON_STATE_PRESSED = 0
    BUTTON_STATE_RELEASED = 1
    LED_LEFT = 0
    LED_RIGHT = 1

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletDualButton.DEVICE_IDENTIFIER, BrickletDualButton.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletDualButton.FUNCTION_SET_LED_STATE] = BrickletDualButton.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletDualButton.FUNCTION_GET_LED_STATE] = BrickletDualButton.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDualButton.FUNCTION_GET_BUTTON_STATE] = BrickletDualButton.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDualButton.FUNCTION_SET_SELECTED_LED_STATE] = BrickletDualButton.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletDualButton.FUNCTION_GET_IDENTITY] = BrickletDualButton.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletDualButton.CALLBACK_STATE_CHANGED] = (12, 'B B B B')

        ipcon.add_device(self)

    def set_led_state(self, led_l, led_r):
        r"""
        Sets the state of the LEDs. Possible states are:

        * 0 = AutoToggleOn: Enables auto toggle with initially enabled LED.
        * 1 = AutoToggleOff: Activates auto toggle with initially disabled LED.
        * 2 = On: Enables LED (auto toggle is disabled).
        * 3 = Off: Disables LED (auto toggle is disabled).

        In auto toggle mode the LED is toggled automatically at each press of a button.

        If you just want to set one of the LEDs and don't know the current state
        of the other LED, you can get the state with :func:`Get LED State` or you
        can use :func:`Set Selected LED State`.
        """
        self.check_validity()

        led_l = int(led_l)
        led_r = int(led_r)

        self.ipcon.send_request(self, BrickletDualButton.FUNCTION_SET_LED_STATE, (led_l, led_r), 'B B', 0, '')

    def get_led_state(self):
        r"""
        Returns the current state of the LEDs, as set by :func:`Set LED State`.
        """
        self.check_validity()

        return GetLEDState(*self.ipcon.send_request(self, BrickletDualButton.FUNCTION_GET_LED_STATE, (), '', 10, 'B B'))

    def get_button_state(self):
        r"""
        Returns the current state for both buttons. Possible states are:

        * 0 = pressed
        * 1 = released
        """
        self.check_validity()

        return GetButtonState(*self.ipcon.send_request(self, BrickletDualButton.FUNCTION_GET_BUTTON_STATE, (), '', 10, 'B B'))

    def set_selected_led_state(self, led, state):
        r"""
        Sets the state of the selected LED (0 or 1).

        The other LED remains untouched.
        """
        self.check_validity()

        led = int(led)
        state = int(state)

        self.ipcon.send_request(self, BrickletDualButton.FUNCTION_SET_SELECTED_LED_STATE, (led, state), 'B B', 0, '')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletDualButton.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

DualButton = BrickletDualButton # for backward compatibility
