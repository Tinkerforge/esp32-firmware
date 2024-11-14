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

GetBeep = namedtuple('Beep', ['frequency', 'volume', 'duration', 'duration_remaining'])
GetAlarm = namedtuple('Alarm', ['start_frequency', 'end_frequency', 'step_size', 'step_delay', 'volume', 'duration', 'duration_remaining', 'current_frequency'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletPiezoSpeakerV2(Device):
    r"""
    Creates beep and alarm with configurable volume and frequency
    """

    DEVICE_IDENTIFIER = 2145
    DEVICE_DISPLAY_NAME = 'Piezo Speaker Bricklet 2.0'
    DEVICE_URL_PART = 'piezo_speaker_v2' # internal

    CALLBACK_BEEP_FINISHED = 7
    CALLBACK_ALARM_FINISHED = 8


    FUNCTION_SET_BEEP = 1
    FUNCTION_GET_BEEP = 2
    FUNCTION_SET_ALARM = 3
    FUNCTION_GET_ALARM = 4
    FUNCTION_UPDATE_VOLUME = 5
    FUNCTION_UPDATE_FREQUENCY = 6
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

    BEEP_DURATION_OFF = 0
    BEEP_DURATION_INFINITE = 4294967295
    ALARM_DURATION_OFF = 0
    ALARM_DURATION_INFINITE = 4294967295
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
        Device.__init__(self, uid, ipcon, BrickletPiezoSpeakerV2.DEVICE_IDENTIFIER, BrickletPiezoSpeakerV2.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletPiezoSpeakerV2.FUNCTION_SET_BEEP] = BrickletPiezoSpeakerV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPiezoSpeakerV2.FUNCTION_GET_BEEP] = BrickletPiezoSpeakerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPiezoSpeakerV2.FUNCTION_SET_ALARM] = BrickletPiezoSpeakerV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPiezoSpeakerV2.FUNCTION_GET_ALARM] = BrickletPiezoSpeakerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPiezoSpeakerV2.FUNCTION_UPDATE_VOLUME] = BrickletPiezoSpeakerV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPiezoSpeakerV2.FUNCTION_UPDATE_FREQUENCY] = BrickletPiezoSpeakerV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPiezoSpeakerV2.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletPiezoSpeakerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPiezoSpeakerV2.FUNCTION_SET_BOOTLOADER_MODE] = BrickletPiezoSpeakerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPiezoSpeakerV2.FUNCTION_GET_BOOTLOADER_MODE] = BrickletPiezoSpeakerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPiezoSpeakerV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletPiezoSpeakerV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPiezoSpeakerV2.FUNCTION_WRITE_FIRMWARE] = BrickletPiezoSpeakerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPiezoSpeakerV2.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletPiezoSpeakerV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPiezoSpeakerV2.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletPiezoSpeakerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPiezoSpeakerV2.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletPiezoSpeakerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPiezoSpeakerV2.FUNCTION_RESET] = BrickletPiezoSpeakerV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPiezoSpeakerV2.FUNCTION_WRITE_UID] = BrickletPiezoSpeakerV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPiezoSpeakerV2.FUNCTION_READ_UID] = BrickletPiezoSpeakerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPiezoSpeakerV2.FUNCTION_GET_IDENTITY] = BrickletPiezoSpeakerV2.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletPiezoSpeakerV2.CALLBACK_BEEP_FINISHED] = (8, '')
        self.callback_formats[BrickletPiezoSpeakerV2.CALLBACK_ALARM_FINISHED] = (8, '')

        ipcon.add_device(self)

    def set_beep(self, frequency, volume, duration):
        r"""
        Beeps with the given frequency and volume for the duration.

        A duration of 0 stops the current beep if any is ongoing.
        A duration of 4294967295 results in an infinite beep.
        """
        self.check_validity()

        frequency = int(frequency)
        volume = int(volume)
        duration = int(duration)

        self.ipcon.send_request(self, BrickletPiezoSpeakerV2.FUNCTION_SET_BEEP, (frequency, volume, duration), 'H B I', 0, '')

    def get_beep(self):
        r"""
        Returns the last beep settings as set by :func:`Set Beep`. If a beep is currently
        running it also returns the remaining duration of the beep.

        If the frequency or volume is updated during a beep (with :func:`Update Frequency`
        or :func:`Update Volume`) this function returns the updated value.
        """
        self.check_validity()

        return GetBeep(*self.ipcon.send_request(self, BrickletPiezoSpeakerV2.FUNCTION_GET_BEEP, (), '', 19, 'H B I I'))

    def set_alarm(self, start_frequency, end_frequency, step_size, step_delay, volume, duration):
        r"""
        Creates an alarm (a tone that goes back and force between two specified frequencies).

        The following parameters can be set:

        * Start Frequency: Start frequency of the alarm.
        * End Frequency: End frequency of the alarm.
        * Step Size: Size of one step of the sweep between the start/end frequencies.
        * Step Delay: Delay between two steps (duration of time that one tone is used in a sweep).
        * Duration: Duration of the alarm.

        A duration of 0 stops the current alarm if any is ongoing.
        A duration of 4294967295 results in an infinite alarm.

        Below you can find two sets of example settings that you can try out. You can use
        these as a starting point to find an alarm signal that suits your application.

        Example 1: 10 seconds of loud annoying fast alarm

        * Start Frequency = 800
        * End Frequency = 2000
        * Step Size = 10
        * Step Delay = 1
        * Volume = 10
        * Duration = 10000

        Example 2: 10 seconds of soft siren sound with slow build-up

        * Start Frequency = 250
        * End Frequency = 750
        * Step Size = 1
        * Step Delay = 5
        * Volume = 0
        * Duration = 10000

        The following conditions must be met:

        * Start Frequency: has to be smaller than end frequency
        * End Frequency: has to be bigger than start frequency
        * Step Size: has to be small enough to fit into the frequency range
        * Step Delay: has to be small enough to fit into the duration
        """
        self.check_validity()

        start_frequency = int(start_frequency)
        end_frequency = int(end_frequency)
        step_size = int(step_size)
        step_delay = int(step_delay)
        volume = int(volume)
        duration = int(duration)

        self.ipcon.send_request(self, BrickletPiezoSpeakerV2.FUNCTION_SET_ALARM, (start_frequency, end_frequency, step_size, step_delay, volume, duration), 'H H H H B I', 0, '')

    def get_alarm(self):
        r"""
        Returns the last alarm settings as set by :func:`Set Alarm`. If an alarm is currently
        running it also returns the remaining duration of the alarm as well as the
        current frequency of the alarm.

        If the volume is updated during an alarm (with :func:`Update Volume`)
        this function returns the updated value.
        """
        self.check_validity()

        return GetAlarm(*self.ipcon.send_request(self, BrickletPiezoSpeakerV2.FUNCTION_GET_ALARM, (), '', 27, 'H H H H B I I H'))

    def update_volume(self, volume):
        r"""
        Updates the volume of an ongoing beep or alarm.
        """
        self.check_validity()

        volume = int(volume)

        self.ipcon.send_request(self, BrickletPiezoSpeakerV2.FUNCTION_UPDATE_VOLUME, (volume,), 'B', 0, '')

    def update_frequency(self, frequency):
        r"""
        Updates the frequency of an ongoing beep.
        """
        self.check_validity()

        frequency = int(frequency)

        self.ipcon.send_request(self, BrickletPiezoSpeakerV2.FUNCTION_UPDATE_FREQUENCY, (frequency,), 'H', 0, '')

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletPiezoSpeakerV2.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletPiezoSpeakerV2.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPiezoSpeakerV2.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletPiezoSpeakerV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletPiezoSpeakerV2.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletPiezoSpeakerV2.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPiezoSpeakerV2.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPiezoSpeakerV2.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletPiezoSpeakerV2.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletPiezoSpeakerV2.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPiezoSpeakerV2.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletPiezoSpeakerV2.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

PiezoSpeakerV2 = BrickletPiezoSpeakerV2 # for backward compatibility
