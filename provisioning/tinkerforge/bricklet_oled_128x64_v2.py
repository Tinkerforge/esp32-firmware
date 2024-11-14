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

ReadPixelsLowLevel = namedtuple('ReadPixelsLowLevel', ['pixels_length', 'pixels_chunk_offset', 'pixels_chunk_data'])
GetDisplayConfiguration = namedtuple('DisplayConfiguration', ['contrast', 'invert', 'automatic_draw'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletOLED128x64V2(Device):
    r"""
    3.3cm (1.3") OLED display with 128x64 pixels
    """

    DEVICE_IDENTIFIER = 2112
    DEVICE_DISPLAY_NAME = 'OLED 128x64 Bricklet 2.0'
    DEVICE_URL_PART = 'oled_128x64_v2' # internal



    FUNCTION_WRITE_PIXELS_LOW_LEVEL = 1
    FUNCTION_READ_PIXELS_LOW_LEVEL = 2
    FUNCTION_CLEAR_DISPLAY = 3
    FUNCTION_SET_DISPLAY_CONFIGURATION = 4
    FUNCTION_GET_DISPLAY_CONFIGURATION = 5
    FUNCTION_WRITE_LINE = 6
    FUNCTION_DRAW_BUFFERED_FRAME = 7
    FUNCTION_GET_SPITFP_ERROR_COUNT = 234
    FUNCTION_SET_BOOTLOADER_MODE = 235
    FUNCTION_GET_BOOTLOADER_MODE = 236
    FUNCTION_SET_WRITE_FIRMWARE_POINTER = 237
    FUNCTION_WRITE_FIRMWARE = 238
    FUNCTION_SET_STATUS_LED_CONFIG = 239
    FUNCTION_GET_STATUS_LED_CONFIG = 240
    FUNCTION_GET_CHIP_TEMPERATURE = 242
    FUNCTION_RESET = 243
    FUNCTION_WRITE_UID = 248
    FUNCTION_READ_UID = 249
    FUNCTION_GET_IDENTITY = 255

    BOOTLOADER_MODE_BOOTLOADER = 0
    BOOTLOADER_MODE_FIRMWARE = 1
    BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT = 2
    BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT = 3
    BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT = 4
    BOOTLOADER_STATUS_OK = 0
    BOOTLOADER_STATUS_INVALID_MODE = 1
    BOOTLOADER_STATUS_NO_CHANGE = 2
    BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT = 3
    BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT = 4
    BOOTLOADER_STATUS_CRC_MISMATCH = 5
    STATUS_LED_CONFIG_OFF = 0
    STATUS_LED_CONFIG_ON = 1
    STATUS_LED_CONFIG_SHOW_HEARTBEAT = 2
    STATUS_LED_CONFIG_SHOW_STATUS = 3

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletOLED128x64V2.DEVICE_IDENTIFIER, BrickletOLED128x64V2.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletOLED128x64V2.FUNCTION_WRITE_PIXELS_LOW_LEVEL] = BrickletOLED128x64V2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletOLED128x64V2.FUNCTION_READ_PIXELS_LOW_LEVEL] = BrickletOLED128x64V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletOLED128x64V2.FUNCTION_CLEAR_DISPLAY] = BrickletOLED128x64V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletOLED128x64V2.FUNCTION_SET_DISPLAY_CONFIGURATION] = BrickletOLED128x64V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletOLED128x64V2.FUNCTION_GET_DISPLAY_CONFIGURATION] = BrickletOLED128x64V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletOLED128x64V2.FUNCTION_WRITE_LINE] = BrickletOLED128x64V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletOLED128x64V2.FUNCTION_DRAW_BUFFERED_FRAME] = BrickletOLED128x64V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletOLED128x64V2.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletOLED128x64V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletOLED128x64V2.FUNCTION_SET_BOOTLOADER_MODE] = BrickletOLED128x64V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletOLED128x64V2.FUNCTION_GET_BOOTLOADER_MODE] = BrickletOLED128x64V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletOLED128x64V2.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletOLED128x64V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletOLED128x64V2.FUNCTION_WRITE_FIRMWARE] = BrickletOLED128x64V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletOLED128x64V2.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletOLED128x64V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletOLED128x64V2.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletOLED128x64V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletOLED128x64V2.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletOLED128x64V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletOLED128x64V2.FUNCTION_RESET] = BrickletOLED128x64V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletOLED128x64V2.FUNCTION_WRITE_UID] = BrickletOLED128x64V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletOLED128x64V2.FUNCTION_READ_UID] = BrickletOLED128x64V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletOLED128x64V2.FUNCTION_GET_IDENTITY] = BrickletOLED128x64V2.RESPONSE_EXPECTED_ALWAYS_TRUE


        ipcon.add_device(self)

    def write_pixels_low_level(self, x_start, y_start, x_end, y_end, pixels_length, pixels_chunk_offset, pixels_chunk_data):
        r"""
        Writes pixels to the specified window.

        The pixels are written into the window line by line top to bottom
        and each line is written from left to right.

        If automatic draw is enabled (default) the pixels are directly written to
        the screen. Only pixels that have actually changed are updated on the screen,
        the rest stays the same.

        If automatic draw is disabled the pixels are written to an internal buffer and
        the buffer is transferred to the display only after :func:`Draw Buffered Frame`
        is called. This can be used to avoid flicker when drawing a complex frame in
        multiple steps.

        Automatic draw can be configured with the :func:`Set Display Configuration`
        function.
        """
        self.check_validity()

        x_start = int(x_start)
        y_start = int(y_start)
        x_end = int(x_end)
        y_end = int(y_end)
        pixels_length = int(pixels_length)
        pixels_chunk_offset = int(pixels_chunk_offset)
        pixels_chunk_data = list(map(bool, pixels_chunk_data))

        self.ipcon.send_request(self, BrickletOLED128x64V2.FUNCTION_WRITE_PIXELS_LOW_LEVEL, (x_start, y_start, x_end, y_end, pixels_length, pixels_chunk_offset, pixels_chunk_data), 'B B B B H H 448!', 0, '')

    def read_pixels_low_level(self, x_start, y_start, x_end, y_end):
        r"""
        Reads pixels from the specified window.

        The pixels are read from the window line by line top to bottom
        and each line is read from left to right.

        If automatic draw is enabled (default) the pixels that are read are always the
        same that are shown on the display.

        If automatic draw is disabled the pixels are read from the internal buffer
        (see :func:`Draw Buffered Frame`).

        Automatic draw can be configured with the :func:`Set Display Configuration`
        function.
        """
        self.check_validity()

        x_start = int(x_start)
        y_start = int(y_start)
        x_end = int(x_end)
        y_end = int(y_end)

        return ReadPixelsLowLevel(*self.ipcon.send_request(self, BrickletOLED128x64V2.FUNCTION_READ_PIXELS_LOW_LEVEL, (x_start, y_start, x_end, y_end), 'B B B B', 72, 'H H 480!'))

    def clear_display(self):
        r"""
        Clears the complete content of the display.

        If automatic draw is enabled (default) the pixels are directly cleared.

        If automatic draw is disabled the the internal buffer is cleared and
        the buffer is transferred to the display only after :func:`Draw Buffered Frame`
        is called. This can be used to avoid flicker when drawing a complex frame in
        multiple steps.

        Automatic draw can be configured with the :func:`Set Display Configuration`
        function.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletOLED128x64V2.FUNCTION_CLEAR_DISPLAY, (), '', 0, '')

    def set_display_configuration(self, contrast, invert, automatic_draw):
        r"""
        Sets the configuration of the display.

        You can set a contrast value from 0 to 255 and you can invert the color
        (white/black) of the display.

        If automatic draw is set to *true*, the display is automatically updated with every
        call of :func:`Write Pixels` or :func:`Write Line`. If it is set to false, the
        changes are written into an internal buffer and only shown on the display after
        a call of :func:`Draw Buffered Frame`.
        """
        self.check_validity()

        contrast = int(contrast)
        invert = bool(invert)
        automatic_draw = bool(automatic_draw)

        self.ipcon.send_request(self, BrickletOLED128x64V2.FUNCTION_SET_DISPLAY_CONFIGURATION, (contrast, invert, automatic_draw), 'B ! !', 0, '')

    def get_display_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Display Configuration`.
        """
        self.check_validity()

        return GetDisplayConfiguration(*self.ipcon.send_request(self, BrickletOLED128x64V2.FUNCTION_GET_DISPLAY_CONFIGURATION, (), '', 11, 'B ! !'))

    def write_line(self, line, position, text):
        r"""
        Writes text to a specific line with a specific position.
        The text can have a maximum of 22 characters.

        For example: (1, 10, "Hello") will write *Hello* in the middle of the
        second line of the display.

        The display uses a special 5x7 pixel charset. You can view the characters
        of the charset in Brick Viewer.

        If automatic draw is enabled (default) the text is directly written to
        the screen. Only pixels that have actually changed are updated on the screen,
        the rest stays the same.

        If automatic draw is disabled the text is written to an internal buffer and
        the buffer is transferred to the display only after :func:`Draw Buffered Frame`
        is called. This can be used to avoid flicker when drawing a complex frame in
        multiple steps.

        Automatic draw can be configured with the :func:`Set Display Configuration`
        function.

        The font conforms to code page 437.
        """
        self.check_validity()

        line = int(line)
        position = int(position)
        text = create_string(text)

        self.ipcon.send_request(self, BrickletOLED128x64V2.FUNCTION_WRITE_LINE, (line, position, text), 'B B 22s', 0, '')

    def draw_buffered_frame(self, force_complete_redraw):
        r"""
        Draws the currently buffered frame. Normally each call of :func:`Write Pixels` and
        :func:`Write Line` draws directly onto the display. If you turn automatic draw off
        (:func:`Set Display Configuration`), the data is written in an internal buffer and
        only transferred to the display by calling this function. This can be used to
        avoid flicker when drawing a complex frame in multiple steps.

        Set the `force complete redraw` to *true* to redraw the whole display
        instead of only the changed parts. Normally it should not be necessary to set this to
        *true*. It may only become necessary in case of stuck pixels because of errors.
        """
        self.check_validity()

        force_complete_redraw = bool(force_complete_redraw)

        self.ipcon.send_request(self, BrickletOLED128x64V2.FUNCTION_DRAW_BUFFERED_FRAME, (force_complete_redraw,), '!', 0, '')

    def get_spitfp_error_count(self):
        r"""
        Returns the error count for the communication between Brick and Bricklet.

        The errors are divided into

        * ACK checksum errors,
        * message checksum errors,
        * framing errors and
        * overflow errors.

        The errors counts are for errors that occur on the Bricklet side. All
        Bricks have a similar function that returns the errors on the Brick side.
        """
        self.check_validity()

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletOLED128x64V2.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

    def set_bootloader_mode(self, mode):
        r"""
        Sets the bootloader mode and returns the status after the requested
        mode change was instigated.

        You can change from bootloader mode to firmware mode and vice versa. A change
        from bootloader mode to firmware mode will only take place if the entry function,
        device identifier and CRC are present and correct.

        This function is used by Brick Viewer during flashing. It should not be
        necessary to call it in a normal user program.
        """
        self.check_validity()

        mode = int(mode)

        return self.ipcon.send_request(self, BrickletOLED128x64V2.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletOLED128x64V2.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

    def set_write_firmware_pointer(self, pointer):
        r"""
        Sets the firmware pointer for :func:`Write Firmware`. The pointer has
        to be increased by chunks of size 64. The data is written to flash
        every 4 chunks (which equals to one page of size 256).

        This function is used by Brick Viewer during flashing. It should not be
        necessary to call it in a normal user program.
        """
        self.check_validity()

        pointer = int(pointer)

        self.ipcon.send_request(self, BrickletOLED128x64V2.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

    def write_firmware(self, data):
        r"""
        Writes 64 Bytes of firmware at the position as written by
        :func:`Set Write Firmware Pointer` before. The firmware is written
        to flash every 4 chunks.

        You can only write firmware in bootloader mode.

        This function is used by Brick Viewer during flashing. It should not be
        necessary to call it in a normal user program.
        """
        self.check_validity()

        data = list(map(int, data))

        return self.ipcon.send_request(self, BrickletOLED128x64V2.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

    def set_status_led_config(self, config):
        r"""
        Sets the status LED configuration. By default the LED shows
        communication traffic between Brick and Bricklet, it flickers once
        for every 10 received data packets.

        You can also turn the LED permanently on/off or show a heartbeat.

        If the Bricklet is in bootloader mode, the LED is will show heartbeat by default.
        """
        self.check_validity()

        config = int(config)

        self.ipcon.send_request(self, BrickletOLED128x64V2.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletOLED128x64V2.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletOLED128x64V2.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletOLED128x64V2.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletOLED128x64V2.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletOLED128x64V2.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletOLED128x64V2.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def write_pixels(self, x_start, y_start, x_end, y_end, pixels):
        r"""
        Writes pixels to the specified window.

        The pixels are written into the window line by line top to bottom
        and each line is written from left to right.

        If automatic draw is enabled (default) the pixels are directly written to
        the screen. Only pixels that have actually changed are updated on the screen,
        the rest stays the same.

        If automatic draw is disabled the pixels are written to an internal buffer and
        the buffer is transferred to the display only after :func:`Draw Buffered Frame`
        is called. This can be used to avoid flicker when drawing a complex frame in
        multiple steps.

        Automatic draw can be configured with the :func:`Set Display Configuration`
        function.
        """
        x_start = int(x_start)
        y_start = int(y_start)
        x_end = int(x_end)
        y_end = int(y_end)
        pixels = list(map(bool, pixels))

        if len(pixels) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Pixels can be at most 65535 items long')

        pixels_length = len(pixels)
        pixels_chunk_offset = 0

        if pixels_length == 0:
            pixels_chunk_data = [False] * 448
            ret = self.write_pixels_low_level(x_start, y_start, x_end, y_end, pixels_length, pixels_chunk_offset, pixels_chunk_data)
        else:
            with self.stream_lock:
                while pixels_chunk_offset < pixels_length:
                    pixels_chunk_data = create_chunk_data(pixels, pixels_chunk_offset, 448, False)
                    ret = self.write_pixels_low_level(x_start, y_start, x_end, y_end, pixels_length, pixels_chunk_offset, pixels_chunk_data)
                    pixels_chunk_offset += 448

        return ret

    def read_pixels(self, x_start, y_start, x_end, y_end):
        r"""
        Reads pixels from the specified window.

        The pixels are read from the window line by line top to bottom
        and each line is read from left to right.

        If automatic draw is enabled (default) the pixels that are read are always the
        same that are shown on the display.

        If automatic draw is disabled the pixels are read from the internal buffer
        (see :func:`Draw Buffered Frame`).

        Automatic draw can be configured with the :func:`Set Display Configuration`
        function.
        """
        x_start = int(x_start)
        y_start = int(y_start)
        x_end = int(x_end)
        y_end = int(y_end)

        with self.stream_lock:
            ret = self.read_pixels_low_level(x_start, y_start, x_end, y_end)
            pixels_length = ret.pixels_length
            pixels_out_of_sync = ret.pixels_chunk_offset != 0
            pixels_data = ret.pixels_chunk_data

            while not pixels_out_of_sync and len(pixels_data) < pixels_length:
                ret = self.read_pixels_low_level(x_start, y_start, x_end, y_end)
                pixels_length = ret.pixels_length
                pixels_out_of_sync = ret.pixels_chunk_offset != len(pixels_data)
                pixels_data += ret.pixels_chunk_data

            if pixels_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.pixels_chunk_offset + 480 < pixels_length:
                    ret = self.read_pixels_low_level(x_start, y_start, x_end, y_end)
                    pixels_length = ret.pixels_length

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Pixels stream is out-of-sync')

        return pixels_data[:pixels_length]

OLED128x64V2 = BrickletOLED128x64V2 # for backward compatibility
