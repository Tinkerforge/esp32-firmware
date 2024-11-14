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

GetCurrentCallbackConfiguration = namedtuple('CurrentCallbackConfiguration', ['period', 'value_has_to_change', 'option', 'min', 'max'])
GetChannelLEDStatusConfig = namedtuple('ChannelLEDStatusConfig', ['min', 'max', 'config'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletIndustrialDual020mAV2(Device):
    r"""
    Measures two DC currents between 0mA and 20mA (IEC 60381-1)
    """

    DEVICE_IDENTIFIER = 2120
    DEVICE_DISPLAY_NAME = 'Industrial Dual 0-20mA Bricklet 2.0'
    DEVICE_URL_PART = 'industrial_dual_0_20ma_v2' # internal

    CALLBACK_CURRENT = 4


    FUNCTION_GET_CURRENT = 1
    FUNCTION_SET_CURRENT_CALLBACK_CONFIGURATION = 2
    FUNCTION_GET_CURRENT_CALLBACK_CONFIGURATION = 3
    FUNCTION_SET_SAMPLE_RATE = 5
    FUNCTION_GET_SAMPLE_RATE = 6
    FUNCTION_SET_GAIN = 7
    FUNCTION_GET_GAIN = 8
    FUNCTION_SET_CHANNEL_LED_CONFIG = 9
    FUNCTION_GET_CHANNEL_LED_CONFIG = 10
    FUNCTION_SET_CHANNEL_LED_STATUS_CONFIG = 11
    FUNCTION_GET_CHANNEL_LED_STATUS_CONFIG = 12
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

    THRESHOLD_OPTION_OFF = 'x'
    THRESHOLD_OPTION_OUTSIDE = 'o'
    THRESHOLD_OPTION_INSIDE = 'i'
    THRESHOLD_OPTION_SMALLER = '<'
    THRESHOLD_OPTION_GREATER = '>'
    SAMPLE_RATE_240_SPS = 0
    SAMPLE_RATE_60_SPS = 1
    SAMPLE_RATE_15_SPS = 2
    SAMPLE_RATE_4_SPS = 3
    GAIN_1X = 0
    GAIN_2X = 1
    GAIN_4X = 2
    GAIN_8X = 3
    CHANNEL_LED_CONFIG_OFF = 0
    CHANNEL_LED_CONFIG_ON = 1
    CHANNEL_LED_CONFIG_SHOW_HEARTBEAT = 2
    CHANNEL_LED_CONFIG_SHOW_CHANNEL_STATUS = 3
    CHANNEL_LED_STATUS_CONFIG_THRESHOLD = 0
    CHANNEL_LED_STATUS_CONFIG_INTENSITY = 1
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
        Device.__init__(self, uid, ipcon, BrickletIndustrialDual020mAV2.DEVICE_IDENTIFIER, BrickletIndustrialDual020mAV2.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletIndustrialDual020mAV2.FUNCTION_GET_CURRENT] = BrickletIndustrialDual020mAV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDual020mAV2.FUNCTION_SET_CURRENT_CALLBACK_CONFIGURATION] = BrickletIndustrialDual020mAV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIndustrialDual020mAV2.FUNCTION_GET_CURRENT_CALLBACK_CONFIGURATION] = BrickletIndustrialDual020mAV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDual020mAV2.FUNCTION_SET_SAMPLE_RATE] = BrickletIndustrialDual020mAV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDual020mAV2.FUNCTION_GET_SAMPLE_RATE] = BrickletIndustrialDual020mAV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDual020mAV2.FUNCTION_SET_GAIN] = BrickletIndustrialDual020mAV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDual020mAV2.FUNCTION_GET_GAIN] = BrickletIndustrialDual020mAV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDual020mAV2.FUNCTION_SET_CHANNEL_LED_CONFIG] = BrickletIndustrialDual020mAV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDual020mAV2.FUNCTION_GET_CHANNEL_LED_CONFIG] = BrickletIndustrialDual020mAV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDual020mAV2.FUNCTION_SET_CHANNEL_LED_STATUS_CONFIG] = BrickletIndustrialDual020mAV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDual020mAV2.FUNCTION_GET_CHANNEL_LED_STATUS_CONFIG] = BrickletIndustrialDual020mAV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDual020mAV2.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletIndustrialDual020mAV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDual020mAV2.FUNCTION_SET_BOOTLOADER_MODE] = BrickletIndustrialDual020mAV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDual020mAV2.FUNCTION_GET_BOOTLOADER_MODE] = BrickletIndustrialDual020mAV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDual020mAV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletIndustrialDual020mAV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDual020mAV2.FUNCTION_WRITE_FIRMWARE] = BrickletIndustrialDual020mAV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDual020mAV2.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletIndustrialDual020mAV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDual020mAV2.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletIndustrialDual020mAV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDual020mAV2.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletIndustrialDual020mAV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDual020mAV2.FUNCTION_RESET] = BrickletIndustrialDual020mAV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDual020mAV2.FUNCTION_WRITE_UID] = BrickletIndustrialDual020mAV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDual020mAV2.FUNCTION_READ_UID] = BrickletIndustrialDual020mAV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDual020mAV2.FUNCTION_GET_IDENTITY] = BrickletIndustrialDual020mAV2.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletIndustrialDual020mAV2.CALLBACK_CURRENT] = (13, 'B i')

        ipcon.add_device(self)

    def get_current(self, channel):
        r"""
        Returns the current of the specified channel.

        It is possible to detect if an IEC 60381-1 compatible sensor is connected
        and if it works probably.

        If the returned current is below 4mA, there is likely no sensor connected
        or the connected sensor is defective. If the returned current is over 20mA,
        there might be a short circuit or the sensor is defective.


        If you want to get the value periodically, it is recommended to use the
        :cb:`Current` callback. You can set the callback configuration
        with :func:`Set Current Callback Configuration`.
        """
        self.check_validity()

        channel = int(channel)

        return self.ipcon.send_request(self, BrickletIndustrialDual020mAV2.FUNCTION_GET_CURRENT, (channel,), 'B', 12, 'i')

    def set_current_callback_configuration(self, channel, period, value_has_to_change, option, min, max):
        r"""
        The period is the period with which the :cb:`Current` callback is triggered
        periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change
        within the period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.

        It is furthermore possible to constrain the callback with thresholds.

        The `option`-parameter together with min/max sets a threshold for the :cb:`Current` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Threshold is turned off"
         "'o'",    "Threshold is triggered when the value is *outside* the min and max values"
         "'i'",    "Threshold is triggered when the value is *inside* or equal to the min and max values"
         "'<'",    "Threshold is triggered when the value is smaller than the min value (max is ignored)"
         "'>'",    "Threshold is triggered when the value is greater than the min value (max is ignored)"

        If the option is set to 'x' (threshold turned off) the callback is triggered with the fixed period.
        """
        self.check_validity()

        channel = int(channel)
        period = int(period)
        value_has_to_change = bool(value_has_to_change)
        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickletIndustrialDual020mAV2.FUNCTION_SET_CURRENT_CALLBACK_CONFIGURATION, (channel, period, value_has_to_change, option, min, max), 'B I ! c i i', 0, '')

    def get_current_callback_configuration(self, channel):
        r"""
        Returns the callback configuration as set by :func:`Set Current Callback Configuration`.
        """
        self.check_validity()

        channel = int(channel)

        return GetCurrentCallbackConfiguration(*self.ipcon.send_request(self, BrickletIndustrialDual020mAV2.FUNCTION_GET_CURRENT_CALLBACK_CONFIGURATION, (channel,), 'B', 22, 'I ! c i i'))

    def set_sample_rate(self, rate):
        r"""
        Sets the sample rate to either 240, 60, 15 or 4 samples per second.
        The resolution for the rates is 12, 14, 16 and 18 bit respectively.

        .. csv-table::
         :header: "Value", "Description"
         :widths: 10, 100

         "0",    "240 samples per second, 12 bit resolution"
         "1",    "60 samples per second, 14 bit resolution"
         "2",    "15 samples per second, 16 bit resolution"
         "3",    "4 samples per second, 18 bit resolution"
        """
        self.check_validity()

        rate = int(rate)

        self.ipcon.send_request(self, BrickletIndustrialDual020mAV2.FUNCTION_SET_SAMPLE_RATE, (rate,), 'B', 0, '')

    def get_sample_rate(self):
        r"""
        Returns the sample rate as set by :func:`Set Sample Rate`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDual020mAV2.FUNCTION_GET_SAMPLE_RATE, (), '', 9, 'B')

    def set_gain(self, gain):
        r"""
        Sets a gain between 1x and 8x. If you want to measure a very small current,
        you can increase the gain to get some more resolution.

        Example: If you measure 0.5mA with a gain of 8x the return value will be
        4mA.
        """
        self.check_validity()

        gain = int(gain)

        self.ipcon.send_request(self, BrickletIndustrialDual020mAV2.FUNCTION_SET_GAIN, (gain,), 'B', 0, '')

    def get_gain(self):
        r"""
        Returns the gain as set by :func:`Set Gain`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDual020mAV2.FUNCTION_GET_GAIN, (), '', 9, 'B')

    def set_channel_led_config(self, channel, config):
        r"""
        Each channel has a corresponding LED. You can turn the LED off, on or show a
        heartbeat. You can also set the LED to "Channel Status". In this mode the
        LED can either be turned on with a pre-defined threshold or the intensity
        of the LED can change with the measured value.

        You can configure the channel status behavior with :func:`Set Channel LED Status Config`.
        """
        self.check_validity()

        channel = int(channel)
        config = int(config)

        self.ipcon.send_request(self, BrickletIndustrialDual020mAV2.FUNCTION_SET_CHANNEL_LED_CONFIG, (channel, config), 'B B', 0, '')

    def get_channel_led_config(self, channel):
        r"""
        Returns the channel LED configuration as set by :func:`Set Channel LED Config`
        """
        self.check_validity()

        channel = int(channel)

        return self.ipcon.send_request(self, BrickletIndustrialDual020mAV2.FUNCTION_GET_CHANNEL_LED_CONFIG, (channel,), 'B', 9, 'B')

    def set_channel_led_status_config(self, channel, min, max, config):
        r"""
        Sets the channel LED status config. This config is used if the channel LED is
        configured as "Channel Status", see :func:`Set Channel LED Config`.

        For each channel you can choose between threshold and intensity mode.

        In threshold mode you can define a positive or a negative threshold.
        For a positive threshold set the "min" parameter to the threshold value in nA
        above which the LED should turn on and set the "max" parameter to 0. Example:
        If you set a positive threshold of 10mA, the LED will turn on as soon as the
        current exceeds 10mA and turn off again if it goes below 10mA.
        For a negative threshold set the "max" parameter to the threshold value in nA
        below which the LED should turn on and set the "min" parameter to 0. Example:
        If you set a negative threshold of 10mA, the LED will turn on as soon as the
        current goes below 10mA and the LED will turn off when the current exceeds 10mA.

        In intensity mode you can define a range in nA that is used to scale the brightness
        of the LED. Example with min=4mA and max=20mA: The LED is off at 4mA and below,
        on at 20mA and above and the brightness is linearly scaled between the values
        4mA and 20mA. If the min value is greater than the max value, the LED brightness
        is scaled the other way around.
        """
        self.check_validity()

        channel = int(channel)
        min = int(min)
        max = int(max)
        config = int(config)

        self.ipcon.send_request(self, BrickletIndustrialDual020mAV2.FUNCTION_SET_CHANNEL_LED_STATUS_CONFIG, (channel, min, max, config), 'B i i B', 0, '')

    def get_channel_led_status_config(self, channel):
        r"""
        Returns the channel LED status configuration as set by
        :func:`Set Channel LED Status Config`.
        """
        self.check_validity()

        channel = int(channel)

        return GetChannelLEDStatusConfig(*self.ipcon.send_request(self, BrickletIndustrialDual020mAV2.FUNCTION_GET_CHANNEL_LED_STATUS_CONFIG, (channel,), 'B', 17, 'i i B'))

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletIndustrialDual020mAV2.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletIndustrialDual020mAV2.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDual020mAV2.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletIndustrialDual020mAV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletIndustrialDual020mAV2.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletIndustrialDual020mAV2.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDual020mAV2.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDual020mAV2.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletIndustrialDual020mAV2.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletIndustrialDual020mAV2.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDual020mAV2.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletIndustrialDual020mAV2.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

IndustrialDual020mAV2 = BrickletIndustrialDual020mAV2 # for backward compatibility
