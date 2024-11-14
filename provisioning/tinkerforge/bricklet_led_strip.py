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

GetRGBValues = namedtuple('RGBValues', ['r', 'g', 'b'])
GetRGBWValues = namedtuple('RGBWValues', ['r', 'g', 'b', 'w'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletLEDStrip(Device):
    r"""
    Controls up to 320 RGB LEDs
    """

    DEVICE_IDENTIFIER = 231
    DEVICE_DISPLAY_NAME = 'LED Strip Bricklet'
    DEVICE_URL_PART = 'led_strip' # internal

    CALLBACK_FRAME_RENDERED = 6


    FUNCTION_SET_RGB_VALUES = 1
    FUNCTION_GET_RGB_VALUES = 2
    FUNCTION_SET_FRAME_DURATION = 3
    FUNCTION_GET_FRAME_DURATION = 4
    FUNCTION_GET_SUPPLY_VOLTAGE = 5
    FUNCTION_SET_CLOCK_FREQUENCY = 7
    FUNCTION_GET_CLOCK_FREQUENCY = 8
    FUNCTION_SET_CHIP_TYPE = 9
    FUNCTION_GET_CHIP_TYPE = 10
    FUNCTION_SET_RGBW_VALUES = 11
    FUNCTION_GET_RGBW_VALUES = 12
    FUNCTION_SET_CHANNEL_MAPPING = 13
    FUNCTION_GET_CHANNEL_MAPPING = 14
    FUNCTION_ENABLE_FRAME_RENDERED_CALLBACK = 15
    FUNCTION_DISABLE_FRAME_RENDERED_CALLBACK = 16
    FUNCTION_IS_FRAME_RENDERED_CALLBACK_ENABLED = 17
    FUNCTION_GET_IDENTITY = 255

    CHIP_TYPE_WS2801 = 2801
    CHIP_TYPE_WS2811 = 2811
    CHIP_TYPE_WS2812 = 2812
    CHIP_TYPE_LPD8806 = 8806
    CHIP_TYPE_APA102 = 102
    CHANNEL_MAPPING_RGB = 6
    CHANNEL_MAPPING_RBG = 9
    CHANNEL_MAPPING_BRG = 33
    CHANNEL_MAPPING_BGR = 36
    CHANNEL_MAPPING_GRB = 18
    CHANNEL_MAPPING_GBR = 24
    CHANNEL_MAPPING_RGBW = 27
    CHANNEL_MAPPING_RGWB = 30
    CHANNEL_MAPPING_RBGW = 39
    CHANNEL_MAPPING_RBWG = 45
    CHANNEL_MAPPING_RWGB = 54
    CHANNEL_MAPPING_RWBG = 57
    CHANNEL_MAPPING_GRWB = 78
    CHANNEL_MAPPING_GRBW = 75
    CHANNEL_MAPPING_GBWR = 108
    CHANNEL_MAPPING_GBRW = 99
    CHANNEL_MAPPING_GWBR = 120
    CHANNEL_MAPPING_GWRB = 114
    CHANNEL_MAPPING_BRGW = 135
    CHANNEL_MAPPING_BRWG = 141
    CHANNEL_MAPPING_BGRW = 147
    CHANNEL_MAPPING_BGWR = 156
    CHANNEL_MAPPING_BWRG = 177
    CHANNEL_MAPPING_BWGR = 180
    CHANNEL_MAPPING_WRBG = 201
    CHANNEL_MAPPING_WRGB = 198
    CHANNEL_MAPPING_WGBR = 216
    CHANNEL_MAPPING_WGRB = 210
    CHANNEL_MAPPING_WBGR = 228
    CHANNEL_MAPPING_WBRG = 225

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletLEDStrip.DEVICE_IDENTIFIER, BrickletLEDStrip.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 3)

        self.response_expected[BrickletLEDStrip.FUNCTION_SET_RGB_VALUES] = BrickletLEDStrip.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLEDStrip.FUNCTION_GET_RGB_VALUES] = BrickletLEDStrip.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLEDStrip.FUNCTION_SET_FRAME_DURATION] = BrickletLEDStrip.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLEDStrip.FUNCTION_GET_FRAME_DURATION] = BrickletLEDStrip.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLEDStrip.FUNCTION_GET_SUPPLY_VOLTAGE] = BrickletLEDStrip.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLEDStrip.FUNCTION_SET_CLOCK_FREQUENCY] = BrickletLEDStrip.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLEDStrip.FUNCTION_GET_CLOCK_FREQUENCY] = BrickletLEDStrip.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLEDStrip.FUNCTION_SET_CHIP_TYPE] = BrickletLEDStrip.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLEDStrip.FUNCTION_GET_CHIP_TYPE] = BrickletLEDStrip.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLEDStrip.FUNCTION_SET_RGBW_VALUES] = BrickletLEDStrip.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLEDStrip.FUNCTION_GET_RGBW_VALUES] = BrickletLEDStrip.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLEDStrip.FUNCTION_SET_CHANNEL_MAPPING] = BrickletLEDStrip.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLEDStrip.FUNCTION_GET_CHANNEL_MAPPING] = BrickletLEDStrip.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLEDStrip.FUNCTION_ENABLE_FRAME_RENDERED_CALLBACK] = BrickletLEDStrip.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletLEDStrip.FUNCTION_DISABLE_FRAME_RENDERED_CALLBACK] = BrickletLEDStrip.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletLEDStrip.FUNCTION_IS_FRAME_RENDERED_CALLBACK_ENABLED] = BrickletLEDStrip.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLEDStrip.FUNCTION_GET_IDENTITY] = BrickletLEDStrip.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletLEDStrip.CALLBACK_FRAME_RENDERED] = (10, 'H')

        ipcon.add_device(self)

    def set_rgb_values(self, index, length, r, g, b):
        r"""
        Sets *length* RGB values for the LEDs starting from *index*.

        To make the colors show correctly you need to configure the chip type
        (:func:`Set Chip Type`) and a 3-channel channel mapping (:func:`Set Channel Mapping`)
        according to the connected LEDs.

        Example: If you set

        * index to 5,
        * length to 3,
        * r to [255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
        * g to [0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0] and
        * b to [0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

        the LED with index 5 will be red, 6 will be green and 7 will be blue.

        .. note:: Depending on the LED circuitry colors can be permuted.

        The colors will be transfered to actual LEDs when the next
        frame duration ends, see :func:`Set Frame Duration`.

        Generic approach:

        * Set the frame duration to a value that represents
          the number of frames per second you want to achieve.
        * Set all of the LED colors for one frame.
        * Wait for the :cb:`Frame Rendered` callback.
        * Set all of the LED colors for next frame.
        * Wait for the :cb:`Frame Rendered` callback.
        * and so on.

        This approach ensures that you can change the LED colors with
        a fixed frame rate.

        The actual number of controllable LEDs depends on the number of free
        Bricklet ports. See :ref:`here <led_strip_bricklet_ram_constraints>` for more
        information. A call of :func:`Set RGB Values` with index + length above the
        bounds is ignored completely.
        """
        self.check_validity()

        index = int(index)
        length = int(length)
        r = list(map(int, r))
        g = list(map(int, g))
        b = list(map(int, b))

        self.ipcon.send_request(self, BrickletLEDStrip.FUNCTION_SET_RGB_VALUES, (index, length, r, g, b), 'H B 16B 16B 16B', 0, '')

    def get_rgb_values(self, index, length):
        r"""
        Returns *length* R, G and B values starting from the
        given LED *index*.

        The values are the last values that were set by :func:`Set RGB Values`.
        """
        self.check_validity()

        index = int(index)
        length = int(length)

        return GetRGBValues(*self.ipcon.send_request(self, BrickletLEDStrip.FUNCTION_GET_RGB_VALUES, (index, length), 'H B', 56, '16B 16B 16B'))

    def set_frame_duration(self, duration):
        r"""
        Sets the frame duration.

        Example: If you want to achieve 20 frames per second, you should
        set the frame duration to 50ms (50ms * 20 = 1 second).

        For an explanation of the general approach see :func:`Set RGB Values`.
        """
        self.check_validity()

        duration = int(duration)

        self.ipcon.send_request(self, BrickletLEDStrip.FUNCTION_SET_FRAME_DURATION, (duration,), 'H', 0, '')

    def get_frame_duration(self):
        r"""
        Returns the frame duration as set by :func:`Set Frame Duration`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLEDStrip.FUNCTION_GET_FRAME_DURATION, (), '', 10, 'H')

    def get_supply_voltage(self):
        r"""
        Returns the current supply voltage of the LEDs.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLEDStrip.FUNCTION_GET_SUPPLY_VOLTAGE, (), '', 10, 'H')

    def set_clock_frequency(self, frequency):
        r"""
        Sets the frequency of the clock.

        The Bricklet will choose the nearest achievable frequency, which may
        be off by a few Hz. You can get the exact frequency that is used by
        calling :func:`Get Clock Frequency`.

        If you have problems with flickering LEDs, they may be bits flipping. You
        can fix this by either making the connection between the LEDs and the
        Bricklet shorter or by reducing the frequency.

        With a decreasing frequency your maximum frames per second will decrease
        too.

        .. note::
         The frequency in firmware version 2.0.0 is fixed at 2MHz.

        .. versionadded:: 2.0.1$nbsp;(Plugin)
        """
        self.check_validity()

        frequency = int(frequency)

        self.ipcon.send_request(self, BrickletLEDStrip.FUNCTION_SET_CLOCK_FREQUENCY, (frequency,), 'I', 0, '')

    def get_clock_frequency(self):
        r"""
        Returns the currently used clock frequency as set by :func:`Set Clock Frequency`.

        .. versionadded:: 2.0.1$nbsp;(Plugin)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLEDStrip.FUNCTION_GET_CLOCK_FREQUENCY, (), '', 12, 'I')

    def set_chip_type(self, chip):
        r"""
        Sets the type of the LED driver chip. We currently support the chips

        * WS2801,
        * WS2811,
        * WS2812 / SK6812 / NeoPixel RGB,
        * SK6812RGBW / NeoPixel RGBW (Chip Type = WS2812),
        * LPD8806 and
        * APA102 / DotStar.

        .. versionadded:: 2.0.2$nbsp;(Plugin)
        """
        self.check_validity()

        chip = int(chip)

        self.ipcon.send_request(self, BrickletLEDStrip.FUNCTION_SET_CHIP_TYPE, (chip,), 'H', 0, '')

    def get_chip_type(self):
        r"""
        Returns the currently used chip type as set by :func:`Set Chip Type`.

        .. versionadded:: 2.0.2$nbsp;(Plugin)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLEDStrip.FUNCTION_GET_CHIP_TYPE, (), '', 10, 'H')

    def set_rgbw_values(self, index, length, r, g, b, w):
        r"""
        Sets *length* RGBW values for the LEDs starting from *index*.

        To make the colors show correctly you need to configure the chip type
        (:func:`Set Chip Type`) and a 4-channel channel mapping (:func:`Set Channel Mapping`)
        according to the connected LEDs.

        The maximum length is 12, the index goes from 0 to 239 and the rgbw values
        have 8 bits each.

        Example: If you set

        * index to 5,
        * length to 4,
        * r to [255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
        * g to [0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
        * b to [0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0] and
        * w to [0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0]

        the LED with index 5 will be red, 6 will be green, 7 will be blue and 8 will be white.

        .. note:: Depending on the LED circuitry colors can be permuted.

        The colors will be transfered to actual LEDs when the next
        frame duration ends, see :func:`Set Frame Duration`.

        Generic approach:

        * Set the frame duration to a value that represents
          the number of frames per second you want to achieve.
        * Set all of the LED colors for one frame.
        * Wait for the :cb:`Frame Rendered` callback.
        * Set all of the LED colors for next frame.
        * Wait for the :cb:`Frame Rendered` callback.
        * and so on.

        This approach ensures that you can change the LED colors with
        a fixed frame rate.

        The actual number of controllable LEDs depends on the number of free
        Bricklet ports. See :ref:`here <led_strip_bricklet_ram_constraints>` for more
        information. A call of :func:`Set RGBW Values` with index + length above the
        bounds is ignored completely.

        The LPD8806 LED driver chips have 7-bit channels for RGB. Internally the LED
        Strip Bricklets divides the 8-bit values set using this function by 2 to make
        them 7-bit. Therefore, you can just use the normal value range (0-255) for
        LPD8806 LEDs.

        The brightness channel of the APA102 LED driver chips has 5-bit. Internally the
        LED Strip Bricklets divides the 8-bit values set using this function by 8 to make
        them 5-bit. Therefore, you can just use the normal value range (0-255) for
        the brightness channel of APA102 LEDs.

        .. versionadded:: 2.0.6$nbsp;(Plugin)
        """
        self.check_validity()

        index = int(index)
        length = int(length)
        r = list(map(int, r))
        g = list(map(int, g))
        b = list(map(int, b))
        w = list(map(int, w))

        self.ipcon.send_request(self, BrickletLEDStrip.FUNCTION_SET_RGBW_VALUES, (index, length, r, g, b, w), 'H B 12B 12B 12B 12B', 0, '')

    def get_rgbw_values(self, index, length):
        r"""
        Returns *length* RGBW values starting from the given *index*.

        The values are the last values that were set by :func:`Set RGBW Values`.

        .. versionadded:: 2.0.6$nbsp;(Plugin)
        """
        self.check_validity()

        index = int(index)
        length = int(length)

        return GetRGBWValues(*self.ipcon.send_request(self, BrickletLEDStrip.FUNCTION_GET_RGBW_VALUES, (index, length), 'H B', 56, '12B 12B 12B 12B'))

    def set_channel_mapping(self, mapping):
        r"""
        Sets the channel mapping for the connected LEDs.

        :func:`Set RGB Values` and :func:`Set RGBW Values` take the data in RGB(W) order.
        But the connected LED driver chips might have their 3 or 4 channels in a
        different order. For example, the WS2801 chips typically use BGR order, the
        WS2812 chips typically use GRB order and the APA102 chips typically use WBGR
        order.

        The APA102 chips are special. They have three 8-bit channels for RGB
        and an additional 5-bit channel for the overall brightness of the RGB LED
        making them 4-channel chips. Internally the brightness channel is the first
        channel, therefore one of the Wxyz channel mappings should be used. Then
        the W channel controls the brightness.

        If a 3-channel mapping is selected then :func:`Set RGB Values` has to be used.
        Calling :func:`Set RGBW Values` with a 3-channel mapping will produce incorrect
        results. Vice-versa if a 4-channel mapping is selected then
        :func:`Set RGBW Values` has to be used. Calling :func:`Set RGB Values` with a
        4-channel mapping will produce incorrect results.

        .. versionadded:: 2.0.6$nbsp;(Plugin)
        """
        self.check_validity()

        mapping = int(mapping)

        self.ipcon.send_request(self, BrickletLEDStrip.FUNCTION_SET_CHANNEL_MAPPING, (mapping,), 'B', 0, '')

    def get_channel_mapping(self):
        r"""
        Returns the currently used channel mapping as set by :func:`Set Channel Mapping`.

        .. versionadded:: 2.0.6$nbsp;(Plugin)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLEDStrip.FUNCTION_GET_CHANNEL_MAPPING, (), '', 9, 'B')

    def enable_frame_rendered_callback(self):
        r"""
        Enables the :cb:`Frame Rendered` callback.

        By default the callback is enabled.

        .. versionadded:: 2.0.6$nbsp;(Plugin)
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletLEDStrip.FUNCTION_ENABLE_FRAME_RENDERED_CALLBACK, (), '', 0, '')

    def disable_frame_rendered_callback(self):
        r"""
        Disables the :cb:`Frame Rendered` callback.

        By default the callback is enabled.

        .. versionadded:: 2.0.6$nbsp;(Plugin)
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletLEDStrip.FUNCTION_DISABLE_FRAME_RENDERED_CALLBACK, (), '', 0, '')

    def is_frame_rendered_callback_enabled(self):
        r"""
        Returns *true* if the :cb:`Frame Rendered` callback is enabled, *false* otherwise.

        .. versionadded:: 2.0.6$nbsp;(Plugin)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLEDStrip.FUNCTION_IS_FRAME_RENDERED_CALLBACK_ENABLED, (), '', 9, '!')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletLEDStrip.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

LEDStrip = BrickletLEDStrip # for backward compatibility
