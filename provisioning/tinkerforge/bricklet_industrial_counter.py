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

GetSignalData = namedtuple('SignalData', ['duty_cycle', 'period', 'frequency', 'value'])
GetAllSignalData = namedtuple('AllSignalData', ['duty_cycle', 'period', 'frequency', 'value'])
GetCounterConfiguration = namedtuple('CounterConfiguration', ['count_edge', 'count_direction', 'duty_cycle_prescaler', 'frequency_integration_time'])
GetAllCounterCallbackConfiguration = namedtuple('AllCounterCallbackConfiguration', ['period', 'value_has_to_change'])
GetAllSignalDataCallbackConfiguration = namedtuple('AllSignalDataCallbackConfiguration', ['period', 'value_has_to_change'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletIndustrialCounter(Device):
    r"""
    4 channel counter up to 4MHz
    """

    DEVICE_IDENTIFIER = 293
    DEVICE_DISPLAY_NAME = 'Industrial Counter Bricklet'
    DEVICE_URL_PART = 'industrial_counter' # internal

    CALLBACK_ALL_COUNTER = 19
    CALLBACK_ALL_SIGNAL_DATA = 20


    FUNCTION_GET_COUNTER = 1
    FUNCTION_GET_ALL_COUNTER = 2
    FUNCTION_SET_COUNTER = 3
    FUNCTION_SET_ALL_COUNTER = 4
    FUNCTION_GET_SIGNAL_DATA = 5
    FUNCTION_GET_ALL_SIGNAL_DATA = 6
    FUNCTION_SET_COUNTER_ACTIVE = 7
    FUNCTION_SET_ALL_COUNTER_ACTIVE = 8
    FUNCTION_GET_COUNTER_ACTIVE = 9
    FUNCTION_GET_ALL_COUNTER_ACTIVE = 10
    FUNCTION_SET_COUNTER_CONFIGURATION = 11
    FUNCTION_GET_COUNTER_CONFIGURATION = 12
    FUNCTION_SET_ALL_COUNTER_CALLBACK_CONFIGURATION = 13
    FUNCTION_GET_ALL_COUNTER_CALLBACK_CONFIGURATION = 14
    FUNCTION_SET_ALL_SIGNAL_DATA_CALLBACK_CONFIGURATION = 15
    FUNCTION_GET_ALL_SIGNAL_DATA_CALLBACK_CONFIGURATION = 16
    FUNCTION_SET_CHANNEL_LED_CONFIG = 17
    FUNCTION_GET_CHANNEL_LED_CONFIG = 18
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

    CHANNEL_0 = 0
    CHANNEL_1 = 1
    CHANNEL_2 = 2
    CHANNEL_3 = 3
    COUNT_EDGE_RISING = 0
    COUNT_EDGE_FALLING = 1
    COUNT_EDGE_BOTH = 2
    COUNT_DIRECTION_UP = 0
    COUNT_DIRECTION_DOWN = 1
    COUNT_DIRECTION_EXTERNAL_UP = 2
    COUNT_DIRECTION_EXTERNAL_DOWN = 3
    DUTY_CYCLE_PRESCALER_1 = 0
    DUTY_CYCLE_PRESCALER_2 = 1
    DUTY_CYCLE_PRESCALER_4 = 2
    DUTY_CYCLE_PRESCALER_8 = 3
    DUTY_CYCLE_PRESCALER_16 = 4
    DUTY_CYCLE_PRESCALER_32 = 5
    DUTY_CYCLE_PRESCALER_64 = 6
    DUTY_CYCLE_PRESCALER_128 = 7
    DUTY_CYCLE_PRESCALER_256 = 8
    DUTY_CYCLE_PRESCALER_512 = 9
    DUTY_CYCLE_PRESCALER_1024 = 10
    DUTY_CYCLE_PRESCALER_2048 = 11
    DUTY_CYCLE_PRESCALER_4096 = 12
    DUTY_CYCLE_PRESCALER_8192 = 13
    DUTY_CYCLE_PRESCALER_16384 = 14
    DUTY_CYCLE_PRESCALER_32768 = 15
    FREQUENCY_INTEGRATION_TIME_128_MS = 0
    FREQUENCY_INTEGRATION_TIME_256_MS = 1
    FREQUENCY_INTEGRATION_TIME_512_MS = 2
    FREQUENCY_INTEGRATION_TIME_1024_MS = 3
    FREQUENCY_INTEGRATION_TIME_2048_MS = 4
    FREQUENCY_INTEGRATION_TIME_4096_MS = 5
    FREQUENCY_INTEGRATION_TIME_8192_MS = 6
    FREQUENCY_INTEGRATION_TIME_16384_MS = 7
    FREQUENCY_INTEGRATION_TIME_32768_MS = 8
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
        Device.__init__(self, uid, ipcon, BrickletIndustrialCounter.DEVICE_IDENTIFIER, BrickletIndustrialCounter.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletIndustrialCounter.FUNCTION_GET_COUNTER] = BrickletIndustrialCounter.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_GET_ALL_COUNTER] = BrickletIndustrialCounter.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_SET_COUNTER] = BrickletIndustrialCounter.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_SET_ALL_COUNTER] = BrickletIndustrialCounter.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_GET_SIGNAL_DATA] = BrickletIndustrialCounter.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_GET_ALL_SIGNAL_DATA] = BrickletIndustrialCounter.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_SET_COUNTER_ACTIVE] = BrickletIndustrialCounter.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_SET_ALL_COUNTER_ACTIVE] = BrickletIndustrialCounter.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_GET_COUNTER_ACTIVE] = BrickletIndustrialCounter.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_GET_ALL_COUNTER_ACTIVE] = BrickletIndustrialCounter.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_SET_COUNTER_CONFIGURATION] = BrickletIndustrialCounter.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_GET_COUNTER_CONFIGURATION] = BrickletIndustrialCounter.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_SET_ALL_COUNTER_CALLBACK_CONFIGURATION] = BrickletIndustrialCounter.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_GET_ALL_COUNTER_CALLBACK_CONFIGURATION] = BrickletIndustrialCounter.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_SET_ALL_SIGNAL_DATA_CALLBACK_CONFIGURATION] = BrickletIndustrialCounter.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_GET_ALL_SIGNAL_DATA_CALLBACK_CONFIGURATION] = BrickletIndustrialCounter.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_SET_CHANNEL_LED_CONFIG] = BrickletIndustrialCounter.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_GET_CHANNEL_LED_CONFIG] = BrickletIndustrialCounter.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletIndustrialCounter.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_SET_BOOTLOADER_MODE] = BrickletIndustrialCounter.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_GET_BOOTLOADER_MODE] = BrickletIndustrialCounter.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletIndustrialCounter.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_WRITE_FIRMWARE] = BrickletIndustrialCounter.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletIndustrialCounter.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletIndustrialCounter.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletIndustrialCounter.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_RESET] = BrickletIndustrialCounter.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_WRITE_UID] = BrickletIndustrialCounter.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_READ_UID] = BrickletIndustrialCounter.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIndustrialCounter.FUNCTION_GET_IDENTITY] = BrickletIndustrialCounter.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletIndustrialCounter.CALLBACK_ALL_COUNTER] = (40, '4q')
        self.callback_formats[BrickletIndustrialCounter.CALLBACK_ALL_SIGNAL_DATA] = (65, '4H 4Q 4I 4!')

        ipcon.add_device(self)

    def get_counter(self, channel):
        r"""
        Returns the current counter value for the given channel.
        """
        self.check_validity()

        channel = int(channel)

        return self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_GET_COUNTER, (channel,), 'B', 16, 'q')

    def get_all_counter(self):
        r"""
        Returns the current counter values for all four channels.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_GET_ALL_COUNTER, (), '', 40, '4q')

    def set_counter(self, channel, counter):
        r"""
        Sets the counter value for the given channel.

        The default value for the counters on startup is 0.
        """
        self.check_validity()

        channel = int(channel)
        counter = int(counter)

        self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_SET_COUNTER, (channel, counter), 'B q', 0, '')

    def set_all_counter(self, counter):
        r"""
        Sets the counter values for all four channels.

        The default value for the counters on startup is 0.
        """
        self.check_validity()

        counter = list(map(int, counter))

        self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_SET_ALL_COUNTER, (counter,), '4q', 0, '')

    def get_signal_data(self, channel):
        r"""
        Returns the signal data (duty cycle, period, frequency and value) for the
        given channel.
        """
        self.check_validity()

        channel = int(channel)

        return GetSignalData(*self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_GET_SIGNAL_DATA, (channel,), 'B', 23, 'H Q I !'))

    def get_all_signal_data(self):
        r"""
        Returns the signal data (duty cycle, period, frequency and value) for all four
        channels.
        """
        self.check_validity()

        return GetAllSignalData(*self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_GET_ALL_SIGNAL_DATA, (), '', 65, '4H 4Q 4I 4!'))

    def set_counter_active(self, channel, active):
        r"""
        Activates/deactivates the counter of the given channel.

        true = activate, false = deactivate.

        By default all channels are activated.
        """
        self.check_validity()

        channel = int(channel)
        active = bool(active)

        self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_SET_COUNTER_ACTIVE, (channel, active), 'B !', 0, '')

    def set_all_counter_active(self, active):
        r"""
        Activates/deactivates the counter of all four channels.

        true = activate, false = deactivate.

        By default all channels are activated.
        """
        self.check_validity()

        active = list(map(bool, active))

        self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_SET_ALL_COUNTER_ACTIVE, (active,), '4!', 0, '')

    def get_counter_active(self, channel):
        r"""
        Returns the activation state of the given channel.

        true = activated, false = deactivated.
        """
        self.check_validity()

        channel = int(channel)

        return self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_GET_COUNTER_ACTIVE, (channel,), 'B', 9, '!')

    def get_all_counter_active(self):
        r"""
        Returns the activation state of all four channels.

        true = activated, false = deactivated.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_GET_ALL_COUNTER_ACTIVE, (), '', 9, '4!')

    def set_counter_configuration(self, channel, count_edge, count_direction, duty_cycle_prescaler, frequency_integration_time):
        r"""
        Sets the counter configuration for the given channel.

        * Count Edge: Counter can count on rising, falling or both edges.
        * Count Direction: Counter can count up or down. You can also use
          another channel as direction input, see
          `here <https://www.tinkerforge.com/en/doc/Hardware/Bricklets/Industrial_Counter.html#external-count-direction>`__
          for details.
        * Duty Cycle Prescaler: Sets a divider for the internal clock. See
          `here <https://www.tinkerforge.com/en/doc/Hardware/Bricklets/Industrial_Counter.html#duty-cycle-prescaler-and-frequency-integration-time>`__
          for details.
        * Frequency Integration Time: Sets the integration time for the
          frequency measurement. See
          `here <https://www.tinkerforge.com/en/doc/Hardware/Bricklets/Industrial_Counter.html#duty-cycle-prescaler-and-frequency-integration-time>`__
          for details.
        """
        self.check_validity()

        channel = int(channel)
        count_edge = int(count_edge)
        count_direction = int(count_direction)
        duty_cycle_prescaler = int(duty_cycle_prescaler)
        frequency_integration_time = int(frequency_integration_time)

        self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_SET_COUNTER_CONFIGURATION, (channel, count_edge, count_direction, duty_cycle_prescaler, frequency_integration_time), 'B B B B B', 0, '')

    def get_counter_configuration(self, channel):
        r"""
        Returns the counter configuration as set by :func:`Set Counter Configuration`.
        """
        self.check_validity()

        channel = int(channel)

        return GetCounterConfiguration(*self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_GET_COUNTER_CONFIGURATION, (channel,), 'B', 12, 'B B B B'))

    def set_all_counter_callback_configuration(self, period, value_has_to_change):
        r"""
        The period is the period with which the :cb:`All Counter`
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

        self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_SET_ALL_COUNTER_CALLBACK_CONFIGURATION, (period, value_has_to_change), 'I !', 0, '')

    def get_all_counter_callback_configuration(self):
        r"""
        Returns the callback configuration as set by
        :func:`Set All Counter Callback Configuration`.
        """
        self.check_validity()

        return GetAllCounterCallbackConfiguration(*self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_GET_ALL_COUNTER_CALLBACK_CONFIGURATION, (), '', 13, 'I !'))

    def set_all_signal_data_callback_configuration(self, period, value_has_to_change):
        r"""
        The period is the period with which the :cb:`All Signal Data`
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

        self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_SET_ALL_SIGNAL_DATA_CALLBACK_CONFIGURATION, (period, value_has_to_change), 'I !', 0, '')

    def get_all_signal_data_callback_configuration(self):
        r"""
        Returns the callback configuration as set by
        :func:`Set All Signal Data Callback Configuration`.
        """
        self.check_validity()

        return GetAllSignalDataCallbackConfiguration(*self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_GET_ALL_SIGNAL_DATA_CALLBACK_CONFIGURATION, (), '', 13, 'I !'))

    def set_channel_led_config(self, channel, config):
        r"""
        Each channel has a corresponding LED. You can turn the LED off, on or show a
        heartbeat. You can also set the LED to "Channel Status". In this mode the
        LED is on if the channel is high and off otherwise.
        """
        self.check_validity()

        channel = int(channel)
        config = int(config)

        self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_SET_CHANNEL_LED_CONFIG, (channel, config), 'B B', 0, '')

    def get_channel_led_config(self, channel):
        r"""
        Returns the channel LED configuration as set by :func:`Set Channel LED Config`
        """
        self.check_validity()

        channel = int(channel)

        return self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_GET_CHANNEL_LED_CONFIG, (channel,), 'B', 9, 'B')

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletIndustrialCounter.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

IndustrialCounter = BrickletIndustrialCounter # for backward compatibility
