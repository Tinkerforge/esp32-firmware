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

class BrickletMotionDetector(Device):
    r"""
    Passive infrared (PIR) motion sensor with 7m range
    """

    DEVICE_IDENTIFIER = 233
    DEVICE_DISPLAY_NAME = 'Motion Detector Bricklet'
    DEVICE_URL_PART = 'motion_detector' # internal

    CALLBACK_MOTION_DETECTED = 2
    CALLBACK_DETECTION_CYCLE_ENDED = 3


    FUNCTION_GET_MOTION_DETECTED = 1
    FUNCTION_SET_STATUS_LED_CONFIG = 4
    FUNCTION_GET_STATUS_LED_CONFIG = 5
    FUNCTION_GET_IDENTITY = 255

    MOTION_NOT_DETECTED = 0
    MOTION_DETECTED = 1
    STATUS_LED_CONFIG_OFF = 0
    STATUS_LED_CONFIG_ON = 1
    STATUS_LED_CONFIG_SHOW_STATUS = 2

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletMotionDetector.DEVICE_IDENTIFIER, BrickletMotionDetector.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 1)

        self.response_expected[BrickletMotionDetector.FUNCTION_GET_MOTION_DETECTED] = BrickletMotionDetector.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMotionDetector.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletMotionDetector.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletMotionDetector.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletMotionDetector.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletMotionDetector.FUNCTION_GET_IDENTITY] = BrickletMotionDetector.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletMotionDetector.CALLBACK_MOTION_DETECTED] = (8, '')
        self.callback_formats[BrickletMotionDetector.CALLBACK_DETECTION_CYCLE_ENDED] = (8, '')

        ipcon.add_device(self)

    def get_motion_detected(self):
        r"""
        Returns 1 if a motion was detected. How long this returns 1 after a motion
        was detected can be adjusted with one of the small potentiometers on the
        Motion Detector Bricklet, see :ref:`here
        <motion_detector_bricklet_sensitivity_delay_block_time>`.

        There is also a blue LED on the Bricklet that is on as long as the Bricklet is
        in the "motion detected" state.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMotionDetector.FUNCTION_GET_MOTION_DETECTED, (), '', 9, 'B')

    def set_status_led_config(self, config):
        r"""
        Sets the status led configuration.

        By default the status LED turns on if a motion is detected and off is no motion
        is detected.

        You can also turn the LED permanently on/off.

        .. versionadded:: 2.0.1$nbsp;(Plugin)
        """
        self.check_validity()

        config = int(config)

        self.ipcon.send_request(self, BrickletMotionDetector.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`.

        .. versionadded:: 2.0.1$nbsp;(Plugin)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletMotionDetector.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletMotionDetector.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

MotionDetector = BrickletMotionDetector # for backward compatibility
