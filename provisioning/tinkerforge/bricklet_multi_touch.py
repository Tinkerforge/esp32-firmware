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

class BrickletMultiTouch(Device):
    r"""
    Capacitive touch sensor for 12 electrodes
    """

    DEVICE_IDENTIFIER = 234
    DEVICE_DISPLAY_NAME = 'Multi Touch Bricklet'
    DEVICE_URL_PART = 'multi_touch' # internal

    CALLBACK_TOUCH_STATE = 5


    FUNCTION_GET_TOUCH_STATE = 1
    FUNCTION_RECALIBRATE = 2
    FUNCTION_SET_ELECTRODE_CONFIG = 3
    FUNCTION_GET_ELECTRODE_CONFIG = 4
    FUNCTION_SET_ELECTRODE_SENSITIVITY = 6
    FUNCTION_GET_ELECTRODE_SENSITIVITY = 7
    FUNCTION_GET_IDENTITY = 255


    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletMultiTouch.DEVICE_IDENTIFIER, BrickletMultiTouch.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletMultiTouch.FUNCTION_GET_TOUCH_STATE] = BrickletMultiTouch.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMultiTouch.FUNCTION_RECALIBRATE] = BrickletMultiTouch.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletMultiTouch.FUNCTION_SET_ELECTRODE_CONFIG] = BrickletMultiTouch.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletMultiTouch.FUNCTION_GET_ELECTRODE_CONFIG] = BrickletMultiTouch.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMultiTouch.FUNCTION_SET_ELECTRODE_SENSITIVITY] = BrickletMultiTouch.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletMultiTouch.FUNCTION_GET_ELECTRODE_SENSITIVITY] = BrickletMultiTouch.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMultiTouch.FUNCTION_GET_IDENTITY] = BrickletMultiTouch.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletMultiTouch.CALLBACK_TOUCH_STATE] = (10, 'H')

        ipcon.add_device(self)

    def get_touch_state(self):
        r"""
        Returns the current touch state. The state is given as a bitfield.

        Bits 0 to 11 represent the 12 electrodes and bit 12 represents
        the proximity.

        If an electrode is touched, the corresponding bit is *true*. If
        a hand or similar is in proximity to the electrodes, bit 12 is
        *true*.

        Example: The state 4103 = 0x1007 = 0b1000000000111 means that
        electrodes 0, 1 and 2 are touched and that something is in the
        proximity of the electrodes.

        The proximity is activated with a distance of 1-2cm. An electrode
        is already counted as touched if a finger is nearly touching the
        electrode. This means that you can put a piece of paper or foil
        or similar on top of a electrode to build a touch panel with
        a professional look.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMultiTouch.FUNCTION_GET_TOUCH_STATE, (), '', 10, 'H')

    def recalibrate(self):
        r"""
        Recalibrates the electrodes. Call this function whenever you changed
        or moved you electrodes.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletMultiTouch.FUNCTION_RECALIBRATE, (), '', 0, '')

    def set_electrode_config(self, enabled_electrodes):
        r"""
        Enables/disables electrodes with a bitfield (see :func:`Get Touch State`).

        *True* enables the electrode, *false* disables the electrode. A
        disabled electrode will always return *false* as its state. If you
        don't need all electrodes you can disable the electrodes that are
        not needed.

        It is recommended that you disable the proximity bit (bit 12) if
        the proximity feature is not needed. This will reduce the amount of
        traffic that is produced by the :cb:`Touch State` callback.

        Disabling electrodes will also reduce power consumption.

        Default: 8191 = 0x1FFF = 0b1111111111111 (all electrodes and proximity feature enabled)
        """
        self.check_validity()

        enabled_electrodes = int(enabled_electrodes)

        self.ipcon.send_request(self, BrickletMultiTouch.FUNCTION_SET_ELECTRODE_CONFIG, (enabled_electrodes,), 'H', 0, '')

    def get_electrode_config(self):
        r"""
        Returns the electrode configuration, as set by :func:`Set Electrode Config`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMultiTouch.FUNCTION_GET_ELECTRODE_CONFIG, (), '', 10, 'H')

    def set_electrode_sensitivity(self, sensitivity):
        r"""
        Sets the sensitivity of the electrodes. An electrode with a high sensitivity
        will register a touch earlier then an electrode with a low sensitivity.

        If you build a big electrode you might need to decrease the sensitivity, since
        the area that can be charged will get bigger. If you want to be able to
        activate an electrode from further away you need to increase the sensitivity.

        After a new sensitivity is set, you likely want to call :func:`Recalibrate`
        to calibrate the electrodes with the newly defined sensitivity.
        """
        self.check_validity()

        sensitivity = int(sensitivity)

        self.ipcon.send_request(self, BrickletMultiTouch.FUNCTION_SET_ELECTRODE_SENSITIVITY, (sensitivity,), 'B', 0, '')

    def get_electrode_sensitivity(self):
        r"""
        Returns the current sensitivity, as set by :func:`Set Electrode Sensitivity`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMultiTouch.FUNCTION_GET_ELECTRODE_SENSITIVITY, (), '', 9, 'B')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletMultiTouch.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

MultiTouch = BrickletMultiTouch # for backward compatibility
