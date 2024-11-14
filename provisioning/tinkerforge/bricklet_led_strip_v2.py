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

GetLEDValuesLowLevel = namedtuple('LEDValuesLowLevel', ['value_length', 'value_chunk_offset', 'value_chunk_data'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletLEDStripV2(Device):
    r"""
    Controls up to 2048 RGB(W) LEDs
    """

    DEVICE_IDENTIFIER = 2103
    DEVICE_DISPLAY_NAME = 'LED Strip Bricklet 2.0'
    DEVICE_URL_PART = 'led_strip_v2' # internal

    CALLBACK_FRAME_STARTED = 6


    FUNCTION_SET_LED_VALUES_LOW_LEVEL = 1
    FUNCTION_GET_LED_VALUES_LOW_LEVEL = 2
    FUNCTION_SET_FRAME_DURATION = 3
    FUNCTION_GET_FRAME_DURATION = 4
    FUNCTION_GET_SUPPLY_VOLTAGE = 5
    FUNCTION_SET_CLOCK_FREQUENCY = 7
    FUNCTION_GET_CLOCK_FREQUENCY = 8
    FUNCTION_SET_CHIP_TYPE = 9
    FUNCTION_GET_CHIP_TYPE = 10
    FUNCTION_SET_CHANNEL_MAPPING = 11
    FUNCTION_GET_CHANNEL_MAPPING = 12
    FUNCTION_SET_FRAME_STARTED_CALLBACK_CONFIGURATION = 13
    FUNCTION_GET_FRAME_STARTED_CALLBACK_CONFIGURATION = 14
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
        Device.__init__(self, uid, ipcon, BrickletLEDStripV2.DEVICE_IDENTIFIER, BrickletLEDStripV2.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletLEDStripV2.FUNCTION_SET_LED_VALUES_LOW_LEVEL] = BrickletLEDStripV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletLEDStripV2.FUNCTION_GET_LED_VALUES_LOW_LEVEL] = BrickletLEDStripV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLEDStripV2.FUNCTION_SET_FRAME_DURATION] = BrickletLEDStripV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLEDStripV2.FUNCTION_GET_FRAME_DURATION] = BrickletLEDStripV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLEDStripV2.FUNCTION_GET_SUPPLY_VOLTAGE] = BrickletLEDStripV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLEDStripV2.FUNCTION_SET_CLOCK_FREQUENCY] = BrickletLEDStripV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLEDStripV2.FUNCTION_GET_CLOCK_FREQUENCY] = BrickletLEDStripV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLEDStripV2.FUNCTION_SET_CHIP_TYPE] = BrickletLEDStripV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLEDStripV2.FUNCTION_GET_CHIP_TYPE] = BrickletLEDStripV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLEDStripV2.FUNCTION_SET_CHANNEL_MAPPING] = BrickletLEDStripV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLEDStripV2.FUNCTION_GET_CHANNEL_MAPPING] = BrickletLEDStripV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLEDStripV2.FUNCTION_SET_FRAME_STARTED_CALLBACK_CONFIGURATION] = BrickletLEDStripV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletLEDStripV2.FUNCTION_GET_FRAME_STARTED_CALLBACK_CONFIGURATION] = BrickletLEDStripV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLEDStripV2.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletLEDStripV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLEDStripV2.FUNCTION_SET_BOOTLOADER_MODE] = BrickletLEDStripV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLEDStripV2.FUNCTION_GET_BOOTLOADER_MODE] = BrickletLEDStripV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLEDStripV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletLEDStripV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLEDStripV2.FUNCTION_WRITE_FIRMWARE] = BrickletLEDStripV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLEDStripV2.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletLEDStripV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLEDStripV2.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletLEDStripV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLEDStripV2.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletLEDStripV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLEDStripV2.FUNCTION_RESET] = BrickletLEDStripV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLEDStripV2.FUNCTION_WRITE_UID] = BrickletLEDStripV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletLEDStripV2.FUNCTION_READ_UID] = BrickletLEDStripV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletLEDStripV2.FUNCTION_GET_IDENTITY] = BrickletLEDStripV2.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletLEDStripV2.CALLBACK_FRAME_STARTED] = (10, 'H')

        ipcon.add_device(self)

    def set_led_values_low_level(self, index, value_length, value_chunk_offset, value_chunk_data):
        r"""
        Sets the RGB(W) values for the LEDs starting from *index*.
        You can set at most 2048 RGB values or 1536 RGBW values (6144 byte each).

        To make the colors show correctly you need to configure the chip type
        (see :func:`Set Chip Type`) and a channel mapping (see :func:`Set Channel Mapping`)
        according to the connected LEDs.

        If the channel mapping has 3 colors, you need to give the data in the sequence
        RGBRGBRGB... if the channel mapping has 4 colors you need to give data in the
        sequence RGBWRGBWRGBW...

        The data is double buffered and the colors will be transfered to the
        LEDs when the next frame duration ends (see :func:`Set Frame Duration`).

        Generic approach:

        * Set the frame duration to a value that represents the number of frames per
          second you want to achieve.
        * Set all of the LED colors for one frame.
        * Wait for the :cb:`Frame Started` callback.
        * Set all of the LED colors for next frame.
        * Wait for the :cb:`Frame Started` callback.
        * And so on.

        This approach ensures that you can change the LED colors with a fixed frame rate.
        """
        self.check_validity()

        index = int(index)
        value_length = int(value_length)
        value_chunk_offset = int(value_chunk_offset)
        value_chunk_data = list(map(int, value_chunk_data))

        self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_SET_LED_VALUES_LOW_LEVEL, (index, value_length, value_chunk_offset, value_chunk_data), 'H H H 58B', 0, '')

    def get_led_values_low_level(self, index, length):
        r"""
        Returns *length* RGB(W) values starting from the
        given *index*.

        If the channel mapping has 3 colors, you will get the data in the sequence
        RGBRGBRGB... if the channel mapping has 4 colors you will get the data in the
        sequence RGBWRGBWRGBW...
        (assuming you start at an index divisible by 3 (RGB) or 4 (RGBW)).
        """
        self.check_validity()

        index = int(index)
        length = int(length)

        return GetLEDValuesLowLevel(*self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_GET_LED_VALUES_LOW_LEVEL, (index, length), 'H H', 72, 'H H 60B'))

    def set_frame_duration(self, duration):
        r"""
        Sets the frame duration.

        Example: If you want to achieve 20 frames per second, you should
        set the frame duration to 50ms (50ms * 20 = 1 second).

        For an explanation of the general approach see :func:`Set LED Values`.

        Default value: 100ms (10 frames per second).
        """
        self.check_validity()

        duration = int(duration)

        self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_SET_FRAME_DURATION, (duration,), 'H', 0, '')

    def get_frame_duration(self):
        r"""
        Returns the frame duration as set by :func:`Set Frame Duration`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_GET_FRAME_DURATION, (), '', 10, 'H')

    def get_supply_voltage(self):
        r"""
        Returns the current supply voltage of the LEDs.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_GET_SUPPLY_VOLTAGE, (), '', 10, 'H')

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
        """
        self.check_validity()

        frequency = int(frequency)

        self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_SET_CLOCK_FREQUENCY, (frequency,), 'I', 0, '')

    def get_clock_frequency(self):
        r"""
        Returns the currently used clock frequency as set by :func:`Set Clock Frequency`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_GET_CLOCK_FREQUENCY, (), '', 12, 'I')

    def set_chip_type(self, chip):
        r"""
        Sets the type of the LED driver chip. We currently support the chips

        * WS2801,
        * WS2811,
        * WS2812 / SK6812 / NeoPixel RGB,
        * SK6812RGBW / NeoPixel RGBW (Chip Type = WS2812),
        * WS2813 / WS2815 (Chip Type = WS2812)
        * LPD8806 and
        * APA102 / DotStar.
        """
        self.check_validity()

        chip = int(chip)

        self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_SET_CHIP_TYPE, (chip,), 'H', 0, '')

    def get_chip_type(self):
        r"""
        Returns the currently used chip type as set by :func:`Set Chip Type`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_GET_CHIP_TYPE, (), '', 10, 'H')

    def set_channel_mapping(self, mapping):
        r"""
        Sets the channel mapping for the connected LEDs.

        If the mapping has 4 colors, the function :func:`Set LED Values` expects 4
        values per pixel and if the mapping has 3 colors it expects 3 values per pixel.

        The function always expects the order RGB(W). The connected LED driver chips
        might have their 3 or 4 channels in a different order. For example, the WS2801
        chips typically use BGR order, then WS2812 chips typically use GRB order and
        the APA102 chips typically use WBGR order.

        The APA102 chips are special. They have three 8-bit channels for RGB
        and an additional 5-bit channel for the overall brightness of the RGB LED
        making them 4-channel chips. Internally the brightness channel is the first
        channel, therefore one of the Wxyz channel mappings should be used. Then
        the W channel controls the brightness.
        """
        self.check_validity()

        mapping = int(mapping)

        self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_SET_CHANNEL_MAPPING, (mapping,), 'B', 0, '')

    def get_channel_mapping(self):
        r"""
        Returns the currently used channel mapping as set by :func:`Set Channel Mapping`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_GET_CHANNEL_MAPPING, (), '', 9, 'B')

    def set_frame_started_callback_configuration(self, enable):
        r"""
        Enables/disables the :cb:`Frame Started` callback.
        """
        self.check_validity()

        enable = bool(enable)

        self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_SET_FRAME_STARTED_CALLBACK_CONFIGURATION, (enable,), '!', 0, '')

    def get_frame_started_callback_configuration(self):
        r"""
        Returns the configuration as set by
        :func:`Set Frame Started Callback Configuration`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_GET_FRAME_STARTED_CALLBACK_CONFIGURATION, (), '', 9, '!')

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletLEDStripV2.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def set_led_values(self, index, value):
        r"""
        Sets the RGB(W) values for the LEDs starting from *index*.
        You can set at most 2048 RGB values or 1536 RGBW values (6144 byte each).

        To make the colors show correctly you need to configure the chip type
        (see :func:`Set Chip Type`) and a channel mapping (see :func:`Set Channel Mapping`)
        according to the connected LEDs.

        If the channel mapping has 3 colors, you need to give the data in the sequence
        RGBRGBRGB... if the channel mapping has 4 colors you need to give data in the
        sequence RGBWRGBWRGBW...

        The data is double buffered and the colors will be transfered to the
        LEDs when the next frame duration ends (see :func:`Set Frame Duration`).

        Generic approach:

        * Set the frame duration to a value that represents the number of frames per
          second you want to achieve.
        * Set all of the LED colors for one frame.
        * Wait for the :cb:`Frame Started` callback.
        * Set all of the LED colors for next frame.
        * Wait for the :cb:`Frame Started` callback.
        * And so on.

        This approach ensures that you can change the LED colors with a fixed frame rate.
        """
        index = int(index)
        value = list(map(int, value))

        if len(value) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Value can be at most 65535 items long')

        value_length = len(value)
        value_chunk_offset = 0

        if value_length == 0:
            value_chunk_data = [0] * 58
            ret = self.set_led_values_low_level(index, value_length, value_chunk_offset, value_chunk_data)
        else:
            with self.stream_lock:
                while value_chunk_offset < value_length:
                    value_chunk_data = create_chunk_data(value, value_chunk_offset, 58, 0)
                    ret = self.set_led_values_low_level(index, value_length, value_chunk_offset, value_chunk_data)
                    value_chunk_offset += 58

        return ret

    def get_led_values(self, index, length):
        r"""
        Returns *length* RGB(W) values starting from the
        given *index*.

        If the channel mapping has 3 colors, you will get the data in the sequence
        RGBRGBRGB... if the channel mapping has 4 colors you will get the data in the
        sequence RGBWRGBWRGBW...
        (assuming you start at an index divisible by 3 (RGB) or 4 (RGBW)).
        """
        index = int(index)
        length = int(length)

        with self.stream_lock:
            ret = self.get_led_values_low_level(index, length)
            value_length = ret.value_length
            value_out_of_sync = ret.value_chunk_offset != 0
            value_data = ret.value_chunk_data

            while not value_out_of_sync and len(value_data) < value_length:
                ret = self.get_led_values_low_level(index, length)
                value_length = ret.value_length
                value_out_of_sync = ret.value_chunk_offset != len(value_data)
                value_data += ret.value_chunk_data

            if value_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.value_chunk_offset + 60 < value_length:
                    ret = self.get_led_values_low_level(index, length)
                    value_length = ret.value_length

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Value stream is out-of-sync')

        return value_data[:value_length]

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

LEDStripV2 = BrickletLEDStripV2 # for backward compatibility
