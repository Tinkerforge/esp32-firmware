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

GetConfiguration = namedtuple('Configuration', ['direction', 'value'])
GetInputValueCallbackConfiguration = namedtuple('InputValueCallbackConfiguration', ['period', 'value_has_to_change'])
GetAllInputValueCallbackConfiguration = namedtuple('AllInputValueCallbackConfiguration', ['period', 'value_has_to_change'])
GetMonoflop = namedtuple('Monoflop', ['value', 'time', 'time_remaining'])
GetEdgeCountConfiguration = namedtuple('EdgeCountConfiguration', ['edge_type', 'debounce'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletIO16V2(Device):
    r"""
    16-channel digital input/output
    """

    DEVICE_IDENTIFIER = 2114
    DEVICE_DISPLAY_NAME = 'IO-16 Bricklet 2.0'
    DEVICE_URL_PART = 'io16_v2' # internal

    CALLBACK_INPUT_VALUE = 15
    CALLBACK_ALL_INPUT_VALUE = 16
    CALLBACK_MONOFLOP_DONE = 17


    FUNCTION_SET_VALUE = 1
    FUNCTION_GET_VALUE = 2
    FUNCTION_SET_SELECTED_VALUE = 3
    FUNCTION_SET_CONFIGURATION = 4
    FUNCTION_GET_CONFIGURATION = 5
    FUNCTION_SET_INPUT_VALUE_CALLBACK_CONFIGURATION = 6
    FUNCTION_GET_INPUT_VALUE_CALLBACK_CONFIGURATION = 7
    FUNCTION_SET_ALL_INPUT_VALUE_CALLBACK_CONFIGURATION = 8
    FUNCTION_GET_ALL_INPUT_VALUE_CALLBACK_CONFIGURATION = 9
    FUNCTION_SET_MONOFLOP = 10
    FUNCTION_GET_MONOFLOP = 11
    FUNCTION_GET_EDGE_COUNT = 12
    FUNCTION_SET_EDGE_COUNT_CONFIGURATION = 13
    FUNCTION_GET_EDGE_COUNT_CONFIGURATION = 14
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

    DIRECTION_IN = 'i'
    DIRECTION_OUT = 'o'
    EDGE_TYPE_RISING = 0
    EDGE_TYPE_FALLING = 1
    EDGE_TYPE_BOTH = 2
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
        Device.__init__(self, uid, ipcon, BrickletIO16V2.DEVICE_IDENTIFIER, BrickletIO16V2.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletIO16V2.FUNCTION_SET_VALUE] = BrickletIO16V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIO16V2.FUNCTION_GET_VALUE] = BrickletIO16V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIO16V2.FUNCTION_SET_SELECTED_VALUE] = BrickletIO16V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIO16V2.FUNCTION_SET_CONFIGURATION] = BrickletIO16V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIO16V2.FUNCTION_GET_CONFIGURATION] = BrickletIO16V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIO16V2.FUNCTION_SET_INPUT_VALUE_CALLBACK_CONFIGURATION] = BrickletIO16V2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIO16V2.FUNCTION_GET_INPUT_VALUE_CALLBACK_CONFIGURATION] = BrickletIO16V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIO16V2.FUNCTION_SET_ALL_INPUT_VALUE_CALLBACK_CONFIGURATION] = BrickletIO16V2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIO16V2.FUNCTION_GET_ALL_INPUT_VALUE_CALLBACK_CONFIGURATION] = BrickletIO16V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIO16V2.FUNCTION_SET_MONOFLOP] = BrickletIO16V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIO16V2.FUNCTION_GET_MONOFLOP] = BrickletIO16V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIO16V2.FUNCTION_GET_EDGE_COUNT] = BrickletIO16V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIO16V2.FUNCTION_SET_EDGE_COUNT_CONFIGURATION] = BrickletIO16V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIO16V2.FUNCTION_GET_EDGE_COUNT_CONFIGURATION] = BrickletIO16V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIO16V2.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletIO16V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIO16V2.FUNCTION_SET_BOOTLOADER_MODE] = BrickletIO16V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIO16V2.FUNCTION_GET_BOOTLOADER_MODE] = BrickletIO16V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIO16V2.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletIO16V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIO16V2.FUNCTION_WRITE_FIRMWARE] = BrickletIO16V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIO16V2.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletIO16V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIO16V2.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletIO16V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIO16V2.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletIO16V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIO16V2.FUNCTION_RESET] = BrickletIO16V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIO16V2.FUNCTION_WRITE_UID] = BrickletIO16V2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIO16V2.FUNCTION_READ_UID] = BrickletIO16V2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIO16V2.FUNCTION_GET_IDENTITY] = BrickletIO16V2.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletIO16V2.CALLBACK_INPUT_VALUE] = (11, 'B ! !')
        self.callback_formats[BrickletIO16V2.CALLBACK_ALL_INPUT_VALUE] = (12, '16! 16!')
        self.callback_formats[BrickletIO16V2.CALLBACK_MONOFLOP_DONE] = (10, 'B !')

        ipcon.add_device(self)

    def set_value(self, value):
        r"""
        Sets the output value of all sixteen channels. A value of *true* or *false* outputs
        logic 1 or logic 0 respectively on the corresponding channel.

        Use :func:`Set Selected Value` to change only one output channel state.

        For example: (True, True, False, False, ..., False) will turn the channels 0-1
        high and the channels 2-15 low.

        All running monoflop timers will be aborted if this function is called.

        .. note::
         This function does nothing for channels that are configured as input. Pull-up
         resistors can be switched on with :func:`Set Configuration`.
        """
        self.check_validity()

        value = list(map(bool, value))

        self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_SET_VALUE, (value,), '16!', 0, '')

    def get_value(self):
        r"""
        Returns the logic levels that are currently measured on the channels.
        This function works if the channel is configured as input as well as if it is
        configured as output.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_GET_VALUE, (), '', 10, '16!')

    def set_selected_value(self, channel, value):
        r"""
        Sets the output value of a specific channel without affecting the other channels.

        A running monoflop timer for the specific channel will be aborted if this
        function is called.

        .. note::
         This function does nothing for channels that are configured as input. Pull-up
         resistors can be switched on with :func:`Set Configuration`.
        """
        self.check_validity()

        channel = int(channel)
        value = bool(value)

        self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_SET_SELECTED_VALUE, (channel, value), 'B !', 0, '')

    def set_configuration(self, channel, direction, value):
        r"""
        Configures the value and direction of a specific channel. Possible directions
        are 'i' and 'o' for input and output.

        If the direction is configured as output, the value is either high or low
        (set as *true* or *false*).

        If the direction is configured as input, the value is either pull-up or
        default (set as *true* or *false*).

        For example:

        * (0, 'i', true) will set channel-0 as input pull-up.
        * (1, 'i', false) will set channel-1 as input default (floating if nothing is connected).
        * (2, 'o', true) will set channel-2 as output high.
        * (3, 'o', false) will set channel-3 as output low.

        A running monoflop timer for the specific channel will be aborted if this
        function is called.
        """
        self.check_validity()

        channel = int(channel)
        direction = create_char(direction)
        value = bool(value)

        self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_SET_CONFIGURATION, (channel, direction, value), 'B c !', 0, '')

    def get_configuration(self, channel):
        r"""
        Returns the channel configuration as set by :func:`Set Configuration`.
        """
        self.check_validity()

        channel = int(channel)

        return GetConfiguration(*self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_GET_CONFIGURATION, (channel,), 'B', 10, 'c !'))

    def set_input_value_callback_configuration(self, channel, period, value_has_to_change):
        r"""
        This callback can be configured per channel.

        The period is the period with which the :cb:`Input Value`
        callback is triggered periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change within the
        period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.
        """
        self.check_validity()

        channel = int(channel)
        period = int(period)
        value_has_to_change = bool(value_has_to_change)

        self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_SET_INPUT_VALUE_CALLBACK_CONFIGURATION, (channel, period, value_has_to_change), 'B I !', 0, '')

    def get_input_value_callback_configuration(self, channel):
        r"""
        Returns the callback configuration as set by
        :func:`Set Input Value Callback Configuration`.
        """
        self.check_validity()

        channel = int(channel)

        return GetInputValueCallbackConfiguration(*self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_GET_INPUT_VALUE_CALLBACK_CONFIGURATION, (channel,), 'B', 13, 'I !'))

    def set_all_input_value_callback_configuration(self, period, value_has_to_change):
        r"""
        The period is the period with which the :cb:`All Input Value`
        callback is triggered periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change within the
        period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.
        """
        self.check_validity()

        period = int(period)
        value_has_to_change = bool(value_has_to_change)

        self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_SET_ALL_INPUT_VALUE_CALLBACK_CONFIGURATION, (period, value_has_to_change), 'I !', 0, '')

    def get_all_input_value_callback_configuration(self):
        r"""
        Returns the callback configuration as set by
        :func:`Set All Input Value Callback Configuration`.
        """
        self.check_validity()

        return GetAllInputValueCallbackConfiguration(*self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_GET_ALL_INPUT_VALUE_CALLBACK_CONFIGURATION, (), '', 13, 'I !'))

    def set_monoflop(self, channel, value, time):
        r"""
        Configures a monoflop of the specified channel.

        The second parameter is the desired value of the specified
        channel. A *true* means relay closed and a *false* means relay open.

        The third parameter indicates the time that the channels should hold
        the value.

        If this function is called with the parameters (0, 1, 1500) channel 0 will
        close and in 1.5s channel 0 will open again

        A monoflop can be used as a fail-safe mechanism. For example: Lets assume you
        have a RS485 bus and a IO-16 Bricklet 2.0 connected to one of
        the slave stacks. You can now call this function every second, with a time
        parameter of two seconds and channel 0 closed. Channel 0 will be closed all the
        time. If now the RS485 connection is lost, then channel 0 will be opened in at
        most two seconds.
        """
        self.check_validity()

        channel = int(channel)
        value = bool(value)
        time = int(time)

        self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_SET_MONOFLOP, (channel, value, time), 'B ! I', 0, '')

    def get_monoflop(self, channel):
        r"""
        Returns (for the given channel) the current value and the time as set by
        :func:`Set Monoflop` as well as the remaining time until the value flips.

        If the timer is not running currently, the remaining time will be returned
        as 0.
        """
        self.check_validity()

        channel = int(channel)

        return GetMonoflop(*self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_GET_MONOFLOP, (channel,), 'B', 17, '! I I'))

    def get_edge_count(self, channel, reset_counter):
        r"""
        Returns the current value of the edge counter for the selected channel. You can
        configure the edges that are counted with :func:`Set Edge Count Configuration`.

        If you set the reset counter to *true*, the count is set back to 0
        directly after it is read.
        """
        self.check_validity()

        channel = int(channel)
        reset_counter = bool(reset_counter)

        return self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_GET_EDGE_COUNT, (channel, reset_counter), 'B !', 12, 'I')

    def set_edge_count_configuration(self, channel, edge_type, debounce):
        r"""
        Configures the edge counter for a specific channel.

        The edge type parameter configures if rising edges, falling edges or
        both are counted if the channel is configured for input. Possible edge types are:

        * 0 = rising
        * 1 = falling
        * 2 = both

        Configuring an edge counter resets its value to 0.

        If you don't know what any of this means, just leave it at default. The
        default configuration is very likely OK for you.
        """
        self.check_validity()

        channel = int(channel)
        edge_type = int(edge_type)
        debounce = int(debounce)

        self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_SET_EDGE_COUNT_CONFIGURATION, (channel, edge_type, debounce), 'B B B', 0, '')

    def get_edge_count_configuration(self, channel):
        r"""
        Returns the edge type and debounce time for the selected channel as set by
        :func:`Set Edge Count Configuration`.
        """
        self.check_validity()

        channel = int(channel)

        return GetEdgeCountConfiguration(*self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_GET_EDGE_COUNT_CONFIGURATION, (channel,), 'B', 10, 'B B'))

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletIO16V2.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

IO16V2 = BrickletIO16V2 # for backward compatibility
