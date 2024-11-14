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

GetDisplayConfiguration = namedtuple('DisplayConfiguration', ['contrast', 'invert'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletOLED64x48(Device):
    r"""
    1.68cm (0.66") OLED display with 64x48 pixels
    """

    DEVICE_IDENTIFIER = 264
    DEVICE_DISPLAY_NAME = 'OLED 64x48 Bricklet'
    DEVICE_URL_PART = 'oled_64x48' # internal



    FUNCTION_WRITE = 1
    FUNCTION_NEW_WINDOW = 2
    FUNCTION_CLEAR_DISPLAY = 3
    FUNCTION_SET_DISPLAY_CONFIGURATION = 4
    FUNCTION_GET_DISPLAY_CONFIGURATION = 5
    FUNCTION_WRITE_LINE = 6
    FUNCTION_GET_IDENTITY = 255


    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletOLED64x48.DEVICE_IDENTIFIER, BrickletOLED64x48.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletOLED64x48.FUNCTION_WRITE] = BrickletOLED64x48.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletOLED64x48.FUNCTION_NEW_WINDOW] = BrickletOLED64x48.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletOLED64x48.FUNCTION_CLEAR_DISPLAY] = BrickletOLED64x48.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletOLED64x48.FUNCTION_SET_DISPLAY_CONFIGURATION] = BrickletOLED64x48.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletOLED64x48.FUNCTION_GET_DISPLAY_CONFIGURATION] = BrickletOLED64x48.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletOLED64x48.FUNCTION_WRITE_LINE] = BrickletOLED64x48.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletOLED64x48.FUNCTION_GET_IDENTITY] = BrickletOLED64x48.RESPONSE_EXPECTED_ALWAYS_TRUE


        ipcon.add_device(self)

    def write(self, data):
        r"""
        Appends 64 byte of data to the window as set by :func:`New Window`.

        Each row has a height of 8 pixels which corresponds to one byte of data.

        Example: if you call :func:`New Window` with column from 0 to 63 and row
        from 0 to 5 (the whole display) each call of :func:`Write` (red arrow) will
        write one row.

        .. image:: /Images/Bricklets/bricklet_oled_64x48_display.png
           :scale: 100 %
           :alt: Display pixel order
           :align: center
           :target: ../../_images/Bricklets/bricklet_oled_64x48_display.png

        The LSB (D0) of each data byte is at the top and the MSB (D7) is at the
        bottom of the row.

        The next call of :func:`Write` will write the second row and so on. To
        fill the whole display you need to call :func:`Write` 6 times.
        """
        self.check_validity()

        data = list(map(int, data))

        self.ipcon.send_request(self, BrickletOLED64x48.FUNCTION_WRITE, (data,), '64B', 0, '')

    def new_window(self, column_from, column_to, row_from, row_to):
        r"""
        Sets the window in which you can write with :func:`Write`. One row
        has a height of 8 pixels.
        """
        self.check_validity()

        column_from = int(column_from)
        column_to = int(column_to)
        row_from = int(row_from)
        row_to = int(row_to)

        self.ipcon.send_request(self, BrickletOLED64x48.FUNCTION_NEW_WINDOW, (column_from, column_to, row_from, row_to), 'B B B B', 0, '')

    def clear_display(self):
        r"""
        Clears the current content of the window as set by :func:`New Window`.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletOLED64x48.FUNCTION_CLEAR_DISPLAY, (), '', 0, '')

    def set_display_configuration(self, contrast, invert):
        r"""
        Sets the configuration of the display.

        You can set a contrast value from 0 to 255 and you can invert the color
        (black/white) of the display.
        """
        self.check_validity()

        contrast = int(contrast)
        invert = bool(invert)

        self.ipcon.send_request(self, BrickletOLED64x48.FUNCTION_SET_DISPLAY_CONFIGURATION, (contrast, invert), 'B !', 0, '')

    def get_display_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Display Configuration`.
        """
        self.check_validity()

        return GetDisplayConfiguration(*self.ipcon.send_request(self, BrickletOLED64x48.FUNCTION_GET_DISPLAY_CONFIGURATION, (), '', 10, 'B !'))

    def write_line(self, line, position, text):
        r"""
        Writes text to a specific line with a specific position.
        The text can have a maximum of 13 characters.

        For example: (1, 4, "Hello") will write *Hello* in the middle of the
        second line of the display.

        You can draw to the display with :func:`Write` and then add text to it
        afterwards.

        The display uses a special 5x7 pixel charset. You can view the characters
        of the charset in Brick Viewer.

        The font conforms to code page 437.
        """
        self.check_validity()

        line = int(line)
        position = int(position)
        text = create_string(text)

        self.ipcon.send_request(self, BrickletOLED64x48.FUNCTION_WRITE_LINE, (line, position, text), 'B B 13s', 0, '')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletOLED64x48.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

OLED64x48 = BrickletOLED64x48 # for backward compatibility
