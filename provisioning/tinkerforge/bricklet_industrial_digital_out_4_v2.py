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

GetMonoflop = namedtuple('Monoflop', ['value', 'time', 'time_remaining'])
GetPWMConfiguration = namedtuple('PWMConfiguration', ['frequency', 'duty_cycle'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletIndustrialDigitalOut4V2(Device):
    r"""
    4 galvanically isolated digital outputs
    """

    DEVICE_IDENTIFIER = 2124
    DEVICE_DISPLAY_NAME = 'Industrial Digital Out 4 Bricklet 2.0'
    DEVICE_URL_PART = 'industrial_digital_out_4_v2' # internal

    CALLBACK_MONOFLOP_DONE = 6


    FUNCTION_SET_VALUE = 1
    FUNCTION_GET_VALUE = 2
    FUNCTION_SET_SELECTED_VALUE = 3
    FUNCTION_SET_MONOFLOP = 4
    FUNCTION_GET_MONOFLOP = 5
    FUNCTION_SET_CHANNEL_LED_CONFIG = 7
    FUNCTION_GET_CHANNEL_LED_CONFIG = 8
    FUNCTION_SET_PWM_CONFIGURATION = 9
    FUNCTION_GET_PWM_CONFIGURATION = 10
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

    CHANNEL_LED_CONFIG_OFF = 0
    CHANNEL_LED_CONFIG_ON = 1
    CHANNEL_LED_CONFIG_SHOW_HEARTBEAT = 2
    CHANNEL_LED_CONFIG_SHOW_CHANNEL_STATUS = 3
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
        Device.__init__(self, uid, ipcon, BrickletIndustrialDigitalOut4V2.DEVICE_IDENTIFIER, BrickletIndustrialDigitalOut4V2.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletIndustrialDigitalOut4V2.FUNCTION_SET_VALUE] = BrickletIndustrialDigitalOut4V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDigitalOut4V2.FUNCTION_GET_VALUE] = BrickletIndustrialDigitalOut4V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDigitalOut4V2.FUNCTION_SET_SELECTED_VALUE] = BrickletIndustrialDigitalOut4V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDigitalOut4V2.FUNCTION_SET_MONOFLOP] = BrickletIndustrialDigitalOut4V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDigitalOut4V2.FUNCTION_GET_MONOFLOP] = BrickletIndustrialDigitalOut4V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDigitalOut4V2.FUNCTION_SET_CHANNEL_LED_CONFIG] = BrickletIndustrialDigitalOut4V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDigitalOut4V2.FUNCTION_GET_CHANNEL_LED_CONFIG] = BrickletIndustrialDigitalOut4V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDigitalOut4V2.FUNCTION_SET_PWM_CONFIGURATION] = BrickletIndustrialDigitalOut4V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDigitalOut4V2.FUNCTION_GET_PWM_CONFIGURATION] = BrickletIndustrialDigitalOut4V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDigitalOut4V2.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletIndustrialDigitalOut4V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDigitalOut4V2.FUNCTION_SET_BOOTLOADER_MODE] = BrickletIndustrialDigitalOut4V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDigitalOut4V2.FUNCTION_GET_BOOTLOADER_MODE] = BrickletIndustrialDigitalOut4V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDigitalOut4V2.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletIndustrialDigitalOut4V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDigitalOut4V2.FUNCTION_WRITE_FIRMWARE] = BrickletIndustrialDigitalOut4V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDigitalOut4V2.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletIndustrialDigitalOut4V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDigitalOut4V2.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletIndustrialDigitalOut4V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDigitalOut4V2.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletIndustrialDigitalOut4V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDigitalOut4V2.FUNCTION_RESET] = BrickletIndustrialDigitalOut4V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDigitalOut4V2.FUNCTION_WRITE_UID] = BrickletIndustrialDigitalOut4V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialDigitalOut4V2.FUNCTION_READ_UID] = BrickletIndustrialDigitalOut4V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialDigitalOut4V2.FUNCTION_GET_IDENTITY] = BrickletIndustrialDigitalOut4V2.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletIndustrialDigitalOut4V2.CALLBACK_MONOFLOP_DONE] = (10, 'B !')

        ipcon.add_device(self)

    def set_value(self, value):
        r"""
        Sets the output value of all four channels. A value of *true* or *false* outputs
        logic 1 or logic 0 respectively on the corresponding channel.

        Use :func:`Set Selected Value` to change only one output channel state.

        All running monoflop timers and PWMs will be aborted if this function is called.

        For example: (True, True, False, False) will turn the channels 0-1 high and the
        channels 2-3 low.
        """
        self.check_validity()

        value = list(map(bool, value))

        self.ipcon.send_request(self, BrickletIndustrialDigitalOut4V2.FUNCTION_SET_VALUE, (value,), '4!', 0, '')

    def get_value(self):
        r"""
        Returns the logic levels that are currently output on the channels.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDigitalOut4V2.FUNCTION_GET_VALUE, (), '', 9, '4!')

    def set_selected_value(self, channel, value):
        r"""
        Sets the output value of a specific channel without affecting the other channels.

        A running monoflop timer or PWM for the specified channel will be aborted if this
        function is called.
        """
        self.check_validity()

        channel = int(channel)
        value = bool(value)

        self.ipcon.send_request(self, BrickletIndustrialDigitalOut4V2.FUNCTION_SET_SELECTED_VALUE, (channel, value), 'B !', 0, '')

    def set_monoflop(self, channel, value, time):
        r"""
        The first parameter is the desired state of the channel (*true* means output *high*
        and *false* means output *low*). The second parameter indicates the time that
        the channel should hold the state.

        If this function is called with the parameters (true, 1500):
        The channel will turn on and in 1.5s it will turn off again.

        A PWM for the selected channel will be aborted if this function is called.

        A monoflop can be used as a failsafe mechanism. For example: Lets assume you
        have a RS485 bus and a IO-4 Bricklet is connected to one of the slave
        stacks. You can now call this function every second, with a time parameter
        of two seconds. The channel will be *high* all the time. If now the RS485
        connection is lost, the channel will turn *low* in at most two seconds.
        """
        self.check_validity()

        channel = int(channel)
        value = bool(value)
        time = int(time)

        self.ipcon.send_request(self, BrickletIndustrialDigitalOut4V2.FUNCTION_SET_MONOFLOP, (channel, value, time), 'B ! I', 0, '')

    def get_monoflop(self, channel):
        r"""
        Returns (for the given channel) the current value and the time as set by
        :func:`Set Monoflop` as well as the remaining time until the value flips.

        If the timer is not running currently, the remaining time will be returned
        as 0.
        """
        self.check_validity()

        channel = int(channel)

        return GetMonoflop(*self.ipcon.send_request(self, BrickletIndustrialDigitalOut4V2.FUNCTION_GET_MONOFLOP, (channel,), 'B', 17, '! I I'))

    def set_channel_led_config(self, channel, config):
        r"""
        Each channel has a corresponding LED. You can turn the LED off, on or show a
        heartbeat. You can also set the LED to "Channel Status". In this mode the
        LED is on if the channel is high and off otherwise.

        By default all channel LEDs are configured as "Channel Status".
        """
        self.check_validity()

        channel = int(channel)
        config = int(config)

        self.ipcon.send_request(self, BrickletIndustrialDigitalOut4V2.FUNCTION_SET_CHANNEL_LED_CONFIG, (channel, config), 'B B', 0, '')

    def get_channel_led_config(self, channel):
        r"""
        Returns the channel LED configuration as set by :func:`Set Channel LED Config`
        """
        self.check_validity()

        channel = int(channel)

        return self.ipcon.send_request(self, BrickletIndustrialDigitalOut4V2.FUNCTION_GET_CHANNEL_LED_CONFIG, (channel,), 'B', 9, 'B')

    def set_pwm_configuration(self, channel, frequency, duty_cycle):
        r"""
        Activates a PWM for the given channel.

        To turn the PWM off again, you can set the frequency to 0 or any other
        function that changes a value of the channel (e.g. :func:`Set Selected Value`).

        The optocoupler of the Industrial Digital
        Out 4 Bricklet 2.0 has a rise time and fall time of 11.5us (each) at 24V. So the maximum
        useful frequency value is about 400000 (40kHz).

        A running monoflop timer for the given channel will be aborted if this function
        is called.
        """
        self.check_validity()

        channel = int(channel)
        frequency = int(frequency)
        duty_cycle = int(duty_cycle)

        self.ipcon.send_request(self, BrickletIndustrialDigitalOut4V2.FUNCTION_SET_PWM_CONFIGURATION, (channel, frequency, duty_cycle), 'B I H', 0, '')

    def get_pwm_configuration(self, channel):
        r"""
        Returns the PWM configuration as set by :func:`Set PWM Configuration`.
        """
        self.check_validity()

        channel = int(channel)

        return GetPWMConfiguration(*self.ipcon.send_request(self, BrickletIndustrialDigitalOut4V2.FUNCTION_GET_PWM_CONFIGURATION, (channel,), 'B', 14, 'I H'))

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletIndustrialDigitalOut4V2.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletIndustrialDigitalOut4V2.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDigitalOut4V2.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletIndustrialDigitalOut4V2.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletIndustrialDigitalOut4V2.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletIndustrialDigitalOut4V2.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDigitalOut4V2.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDigitalOut4V2.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletIndustrialDigitalOut4V2.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletIndustrialDigitalOut4V2.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialDigitalOut4V2.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletIndustrialDigitalOut4V2.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

IndustrialDigitalOut4V2 = BrickletIndustrialDigitalOut4V2 # for backward compatibility
