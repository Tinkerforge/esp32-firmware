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

GetConfig = namedtuple('Config', ['cursor', 'blinking'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletLCD20x4(Device):
    r"""
    20x4 character alphanumeric display with blue backlight
    """

    DEVICE_IDENTIFIER = 212
    DEVICE_DISPLAY_NAME = 'LCD 20x4 Bricklet'
    DEVICE_URL_PART = 'lcd_20x4' # internal

    CALLBACK_BUTTON_PRESSED = 9
    CALLBACK_BUTTON_RELEASED = 10


    FUNCTION_WRITE_LINE = 1
    FUNCTION_CLEAR_DISPLAY = 2
    FUNCTION_BACKLIGHT_ON = 3
    FUNCTION_BACKLIGHT_OFF = 4
    FUNCTION_IS_BACKLIGHT_ON = 5
    FUNCTION_SET_CONFIG = 6
    FUNCTION_GET_CONFIG = 7
    FUNCTION_IS_BUTTON_PRESSED = 8
    FUNCTION_SET_CUSTOM_CHARACTER = 11
    FUNCTION_GET_CUSTOM_CHARACTER = 12
    FUNCTION_SET_DEFAULT_TEXT = 13
    FUNCTION_GET_DEFAULT_TEXT = 14
    FUNCTION_SET_DEFAULT_TEXT_COUNTER = 15
    FUNCTION_GET_DEFAULT_TEXT_COUNTER = 16
    FUNCTION_GET_IDENTITY = 255


    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletLCD20x4.DEVICE_IDENTIFIER, BrickletLCD20x4.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 2)

        self.response_expected[BrickletLCD20x4.FUNCTION_WRITE_LINE] = BrickletLCD20x4.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLCD20x4.FUNCTION_CLEAR_DISPLAY] = BrickletLCD20x4.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLCD20x4.FUNCTION_BACKLIGHT_ON] = BrickletLCD20x4.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLCD20x4.FUNCTION_BACKLIGHT_OFF] = BrickletLCD20x4.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLCD20x4.FUNCTION_IS_BACKLIGHT_ON] = BrickletLCD20x4.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLCD20x4.FUNCTION_SET_CONFIG] = BrickletLCD20x4.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLCD20x4.FUNCTION_GET_CONFIG] = BrickletLCD20x4.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLCD20x4.FUNCTION_IS_BUTTON_PRESSED] = BrickletLCD20x4.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLCD20x4.FUNCTION_SET_CUSTOM_CHARACTER] = BrickletLCD20x4.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLCD20x4.FUNCTION_GET_CUSTOM_CHARACTER] = BrickletLCD20x4.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLCD20x4.FUNCTION_SET_DEFAULT_TEXT] = BrickletLCD20x4.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLCD20x4.FUNCTION_GET_DEFAULT_TEXT] = BrickletLCD20x4.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLCD20x4.FUNCTION_SET_DEFAULT_TEXT_COUNTER] = BrickletLCD20x4.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLCD20x4.FUNCTION_GET_DEFAULT_TEXT_COUNTER] = BrickletLCD20x4.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLCD20x4.FUNCTION_GET_IDENTITY] = BrickletLCD20x4.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletLCD20x4.CALLBACK_BUTTON_PRESSED] = (9, 'B')
        self.callback_formats[BrickletLCD20x4.CALLBACK_BUTTON_RELEASED] = (9, 'B')

        ipcon.add_device(self)

    def write_line(self, line, position, text):
        r"""
        Writes text to a specific line with a specific position.
        The text can have a maximum of 20 characters.

        For example: (0, 7, "Hello") will write *Hello* in the middle of the
        first line of the display.

        The display uses a special charset that includes all ASCII characters except
        backslash and tilde. The LCD charset also includes several other non-ASCII characters, see
        the `charset specification <https://github.com/Tinkerforge/lcd-20x4-bricklet/raw/master/datasheets/standard_charset.pdf>`__
        for details. The Unicode example above shows how to specify non-ASCII characters
        and how to translate from Unicode to the LCD charset.
        """
        self.check_validity()

        line = int(line)
        position = int(position)
        text = create_string(text)

        self.ipcon.send_request(self, BrickletLCD20x4.FUNCTION_WRITE_LINE, (line, position, text), 'B B 20s', 0, '')

    def clear_display(self):
        r"""
        Deletes all characters from the display.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletLCD20x4.FUNCTION_CLEAR_DISPLAY, (), '', 0, '')

    def backlight_on(self):
        r"""
        Turns the backlight on.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletLCD20x4.FUNCTION_BACKLIGHT_ON, (), '', 0, '')

    def backlight_off(self):
        r"""
        Turns the backlight off.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletLCD20x4.FUNCTION_BACKLIGHT_OFF, (), '', 0, '')

    def is_backlight_on(self):
        r"""
        Returns *true* if the backlight is on and *false* otherwise.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLCD20x4.FUNCTION_IS_BACKLIGHT_ON, (), '', 9, '!')

    def set_config(self, cursor, blinking):
        r"""
        Configures if the cursor (shown as "_") should be visible and if it
        should be blinking (shown as a blinking block). The cursor position
        is one character behind the the last text written with
        :func:`Write Line`.
        """
        self.check_validity()

        cursor = bool(cursor)
        blinking = bool(blinking)

        self.ipcon.send_request(self, BrickletLCD20x4.FUNCTION_SET_CONFIG, (cursor, blinking), '! !', 0, '')

    def get_config(self):
        r"""
        Returns the configuration as set by :func:`Set Config`.
        """
        self.check_validity()

        return GetConfig(*self.ipcon.send_request(self, BrickletLCD20x4.FUNCTION_GET_CONFIG, (), '', 10, '! !'))

    def is_button_pressed(self, button):
        r"""
        Returns *true* if the button (0 to 2 or 0 to 3 since hardware version 1.2)
        is pressed.

        If you want to react on button presses and releases it is recommended to use
        the :cb:`Button Pressed` and :cb:`Button Released` callbacks.
        """
        self.check_validity()

        button = int(button)

        return self.ipcon.send_request(self, BrickletLCD20x4.FUNCTION_IS_BUTTON_PRESSED, (button,), 'B', 9, '!')

    def set_custom_character(self, index, character):
        r"""
        The LCD 20x4 Bricklet can store up to 8 custom characters. The characters
        consist of 5x8 pixels and can be addressed with the index 0-7. To describe
        the pixels, the first 5 bits of 8 bytes are used. For example, to make
        a custom character "H", you should transfer the following:

        * ``character[0] = 0b00010001`` (decimal value 17)
        * ``character[1] = 0b00010001`` (decimal value 17)
        * ``character[2] = 0b00010001`` (decimal value 17)
        * ``character[3] = 0b00011111`` (decimal value 31)
        * ``character[4] = 0b00010001`` (decimal value 17)
        * ``character[5] = 0b00010001`` (decimal value 17)
        * ``character[6] = 0b00010001`` (decimal value 17)
        * ``character[7] = 0b00000000`` (decimal value 0)

        The characters can later be written with :func:`Write Line` by using the
        characters with the byte representation 8 ("\\x08" or "\\u0008") to 15
        ("\\x0F" or "\\u000F").

        You can play around with the custom characters in Brick Viewer version
        since 2.0.1.

        Custom characters are stored by the LCD in RAM, so they have to be set
        after each startup.

        .. versionadded:: 2.0.1$nbsp;(Plugin)
        """
        self.check_validity()

        index = int(index)
        character = list(map(int, character))

        self.ipcon.send_request(self, BrickletLCD20x4.FUNCTION_SET_CUSTOM_CHARACTER, (index, character), 'B 8B', 0, '')

    def get_custom_character(self, index):
        r"""
        Returns the custom character for a given index, as set with
        :func:`Set Custom Character`.

        .. versionadded:: 2.0.1$nbsp;(Plugin)
        """
        self.check_validity()

        index = int(index)

        return self.ipcon.send_request(self, BrickletLCD20x4.FUNCTION_GET_CUSTOM_CHARACTER, (index,), 'B', 16, '8B')

    def set_default_text(self, line, text):
        r"""
        Sets the default text for lines 0-3. The max number of characters
        per line is 20.

        The default text is shown on the LCD, if the default text counter
        expires, see :func:`Set Default Text Counter`.

        .. versionadded:: 2.0.2$nbsp;(Plugin)
        """
        self.check_validity()

        line = int(line)
        text = create_string(text)

        self.ipcon.send_request(self, BrickletLCD20x4.FUNCTION_SET_DEFAULT_TEXT, (line, text), 'B 20s', 0, '')

    def get_default_text(self, line):
        r"""
        Returns the default text for a given line (0-3) as set by
        :func:`Set Default Text`.

        .. versionadded:: 2.0.2$nbsp;(Plugin)
        """
        self.check_validity()

        line = int(line)

        return self.ipcon.send_request(self, BrickletLCD20x4.FUNCTION_GET_DEFAULT_TEXT, (line,), 'B', 28, '20s')

    def set_default_text_counter(self, counter):
        r"""
        Sets the default text counter. This counter is decremented each
        ms by the LCD firmware. If the counter reaches 0, the default text
        (see :func:`Set Default Text`) is shown on the LCD.

        This functionality can be used to show a default text if the controlling
        program crashes or the connection is interrupted.

        A possible approach is to call :func:`Set Default Text Counter` every
        minute with the parameter 1000*60*2 (2 minutes). In this case the
        default text will be shown no later than 2 minutes after the
        controlling program crashes.

        A negative counter turns the default text functionality off.

        .. versionadded:: 2.0.2$nbsp;(Plugin)
        """
        self.check_validity()

        counter = int(counter)

        self.ipcon.send_request(self, BrickletLCD20x4.FUNCTION_SET_DEFAULT_TEXT_COUNTER, (counter,), 'i', 0, '')

    def get_default_text_counter(self):
        r"""
        Returns the current value of the default text counter.

        .. versionadded:: 2.0.2$nbsp;(Plugin)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLCD20x4.FUNCTION_GET_DEFAULT_TEXT_COUNTER, (), '', 12, 'i')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletLCD20x4.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

LCD20x4 = BrickletLCD20x4 # for backward compatibility
