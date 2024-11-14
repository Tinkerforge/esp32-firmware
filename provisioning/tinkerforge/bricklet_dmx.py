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

ReadFrameLowLevel = namedtuple('ReadFrameLowLevel', ['frame_length', 'frame_chunk_offset', 'frame_chunk_data', 'frame_number'])
GetFrameErrorCount = namedtuple('FrameErrorCount', ['overrun_error_count', 'framing_error_count'])
GetFrameCallbackConfig = namedtuple('FrameCallbackConfig', ['frame_started_callback_enabled', 'frame_available_callback_enabled', 'frame_callback_enabled', 'frame_error_count_callback_enabled'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])
ReadFrame = namedtuple('ReadFrame', ['frame', 'frame_number'])

class BrickletDMX(Device):
    r"""
    DMX master and slave
    """

    DEVICE_IDENTIFIER = 285
    DEVICE_DISPLAY_NAME = 'DMX Bricklet'
    DEVICE_URL_PART = 'dmx' # internal

    CALLBACK_FRAME_STARTED = 14
    CALLBACK_FRAME_AVAILABLE = 15
    CALLBACK_FRAME_LOW_LEVEL = 16
    CALLBACK_FRAME_ERROR_COUNT = 17

    CALLBACK_FRAME = -16

    FUNCTION_SET_DMX_MODE = 1
    FUNCTION_GET_DMX_MODE = 2
    FUNCTION_WRITE_FRAME_LOW_LEVEL = 3
    FUNCTION_READ_FRAME_LOW_LEVEL = 4
    FUNCTION_SET_FRAME_DURATION = 5
    FUNCTION_GET_FRAME_DURATION = 6
    FUNCTION_GET_FRAME_ERROR_COUNT = 7
    FUNCTION_SET_COMMUNICATION_LED_CONFIG = 8
    FUNCTION_GET_COMMUNICATION_LED_CONFIG = 9
    FUNCTION_SET_ERROR_LED_CONFIG = 10
    FUNCTION_GET_ERROR_LED_CONFIG = 11
    FUNCTION_SET_FRAME_CALLBACK_CONFIG = 12
    FUNCTION_GET_FRAME_CALLBACK_CONFIG = 13
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

    DMX_MODE_MASTER = 0
    DMX_MODE_SLAVE = 1
    COMMUNICATION_LED_CONFIG_OFF = 0
    COMMUNICATION_LED_CONFIG_ON = 1
    COMMUNICATION_LED_CONFIG_SHOW_HEARTBEAT = 2
    COMMUNICATION_LED_CONFIG_SHOW_COMMUNICATION = 3
    ERROR_LED_CONFIG_OFF = 0
    ERROR_LED_CONFIG_ON = 1
    ERROR_LED_CONFIG_SHOW_HEARTBEAT = 2
    ERROR_LED_CONFIG_SHOW_ERROR = 3
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
        Device.__init__(self, uid, ipcon, BrickletDMX.DEVICE_IDENTIFIER, BrickletDMX.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletDMX.FUNCTION_SET_DMX_MODE] = BrickletDMX.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletDMX.FUNCTION_GET_DMX_MODE] = BrickletDMX.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDMX.FUNCTION_WRITE_FRAME_LOW_LEVEL] = BrickletDMX.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletDMX.FUNCTION_READ_FRAME_LOW_LEVEL] = BrickletDMX.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDMX.FUNCTION_SET_FRAME_DURATION] = BrickletDMX.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletDMX.FUNCTION_GET_FRAME_DURATION] = BrickletDMX.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDMX.FUNCTION_GET_FRAME_ERROR_COUNT] = BrickletDMX.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDMX.FUNCTION_SET_COMMUNICATION_LED_CONFIG] = BrickletDMX.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletDMX.FUNCTION_GET_COMMUNICATION_LED_CONFIG] = BrickletDMX.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDMX.FUNCTION_SET_ERROR_LED_CONFIG] = BrickletDMX.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletDMX.FUNCTION_GET_ERROR_LED_CONFIG] = BrickletDMX.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDMX.FUNCTION_SET_FRAME_CALLBACK_CONFIG] = BrickletDMX.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletDMX.FUNCTION_GET_FRAME_CALLBACK_CONFIG] = BrickletDMX.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDMX.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletDMX.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDMX.FUNCTION_SET_BOOTLOADER_MODE] = BrickletDMX.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDMX.FUNCTION_GET_BOOTLOADER_MODE] = BrickletDMX.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDMX.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletDMX.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletDMX.FUNCTION_WRITE_FIRMWARE] = BrickletDMX.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDMX.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletDMX.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletDMX.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletDMX.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDMX.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletDMX.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDMX.FUNCTION_RESET] = BrickletDMX.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletDMX.FUNCTION_WRITE_UID] = BrickletDMX.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletDMX.FUNCTION_READ_UID] = BrickletDMX.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletDMX.FUNCTION_GET_IDENTITY] = BrickletDMX.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletDMX.CALLBACK_FRAME_STARTED] = (8, '')
        self.callback_formats[BrickletDMX.CALLBACK_FRAME_AVAILABLE] = (12, 'I')
        self.callback_formats[BrickletDMX.CALLBACK_FRAME_LOW_LEVEL] = (72, 'H H 56B I')
        self.callback_formats[BrickletDMX.CALLBACK_FRAME_ERROR_COUNT] = (16, 'I I')

        self.high_level_callbacks[BrickletDMX.CALLBACK_FRAME] = [('stream_length', 'stream_chunk_offset', 'stream_chunk_data', None), {'fixed_length': None, 'single_chunk': False}, None]
        ipcon.add_device(self)

    def set_dmx_mode(self, dmx_mode):
        r"""
        Sets the DMX mode to either master or slave.

        Calling this function sets frame number to 0.
        """
        self.check_validity()

        dmx_mode = int(dmx_mode)

        self.ipcon.send_request(self, BrickletDMX.FUNCTION_SET_DMX_MODE, (dmx_mode,), 'B', 0, '')

    def get_dmx_mode(self):
        r"""
        Returns the DMX mode, as set by :func:`Set DMX Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletDMX.FUNCTION_GET_DMX_MODE, (), '', 9, 'B')

    def write_frame_low_level(self, frame_length, frame_chunk_offset, frame_chunk_data):
        r"""
        Writes a DMX frame. The maximum frame size is 512 byte. Each byte represents one channel.

        The next frame can be written after the :cb:`Frame Started` callback was called. The frame
        is double buffered, so a new frame can be written as soon as the writing of the prior frame
        starts.

        The data will be transfered when the next frame duration ends, see :func:`Set Frame Duration`.

        Generic approach:

        * Set the frame duration to a value that represents the number of frames per second you want to achieve.
        * Set channels for first frame.
        * Wait for the :cb:`Frame Started` callback.
        * Set channels for next frame.
        * Wait for the :cb:`Frame Started` callback.
        * and so on.

        This approach ensures that you can set new DMX data with a fixed frame rate.

        This function can only be called in master mode.
        """
        self.check_validity()

        frame_length = int(frame_length)
        frame_chunk_offset = int(frame_chunk_offset)
        frame_chunk_data = list(map(int, frame_chunk_data))

        self.ipcon.send_request(self, BrickletDMX.FUNCTION_WRITE_FRAME_LOW_LEVEL, (frame_length, frame_chunk_offset, frame_chunk_data), 'H H 60B', 0, '')

    def read_frame_low_level(self):
        r"""
        Returns the last frame that was written by the DMX master. The size of the array
        is equivalent to the number of channels in the frame. Each byte represents one channel.

        The next frame is available after the :cb:`Frame Available` callback was called.

        Generic approach:

        * Call :func:`Read Frame` to get first frame.
        * Wait for the :cb:`Frame Available` callback.
        * Call :func:`Read Frame` to get second frame.
        * Wait for the :cb:`Frame Available` callback.
        * and so on.

        Instead of polling this function you can also use the :cb:`Frame` callback.
        You can enable it with :func:`Set Frame Callback Config`.

        The frame number starts at 0 and it is increased by one with each received frame.

        This function can only be called in slave mode.
        """
        self.check_validity()

        return ReadFrameLowLevel(*self.ipcon.send_request(self, BrickletDMX.FUNCTION_READ_FRAME_LOW_LEVEL, (), '', 72, 'H H 56B I'))

    def set_frame_duration(self, frame_duration):
        r"""
        Sets the duration of a frame.

        Example: If you want to achieve 20 frames per second, you should
        set the frame duration to 50ms (50ms * 20 = 1 second).

        If you always want to send a frame as fast as possible you can set
        this value to 0.

        This setting is only used in master mode.
        """
        self.check_validity()

        frame_duration = int(frame_duration)

        self.ipcon.send_request(self, BrickletDMX.FUNCTION_SET_FRAME_DURATION, (frame_duration,), 'H', 0, '')

    def get_frame_duration(self):
        r"""
        Returns the frame duration as set by :func:`Set Frame Duration`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletDMX.FUNCTION_GET_FRAME_DURATION, (), '', 10, 'H')

    def get_frame_error_count(self):
        r"""
        Returns the current number of overrun and framing errors.
        """
        self.check_validity()

        return GetFrameErrorCount(*self.ipcon.send_request(self, BrickletDMX.FUNCTION_GET_FRAME_ERROR_COUNT, (), '', 16, 'I I'))

    def set_communication_led_config(self, config):
        r"""
        Sets the communication LED configuration. By default the LED shows
        communication traffic, it flickers once for every 10 received data packets.

        You can also turn the LED permanently on/off or show a heartbeat.

        If the Bricklet is in bootloader mode, the LED is off.
        """
        self.check_validity()

        config = int(config)

        self.ipcon.send_request(self, BrickletDMX.FUNCTION_SET_COMMUNICATION_LED_CONFIG, (config,), 'B', 0, '')

    def get_communication_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Communication LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletDMX.FUNCTION_GET_COMMUNICATION_LED_CONFIG, (), '', 9, 'B')

    def set_error_led_config(self, config):
        r"""
        Sets the error LED configuration.

        By default the error LED turns on if there is any error (see :cb:`Frame Error Count`
        callback). If you call this function with the Show-Error option again, the LED
        will turn off until the next error occurs.

        You can also turn the LED permanently on/off or show a heartbeat.

        If the Bricklet is in bootloader mode, the LED is off.
        """
        self.check_validity()

        config = int(config)

        self.ipcon.send_request(self, BrickletDMX.FUNCTION_SET_ERROR_LED_CONFIG, (config,), 'B', 0, '')

    def get_error_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Error LED Config`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletDMX.FUNCTION_GET_ERROR_LED_CONFIG, (), '', 9, 'B')

    def set_frame_callback_config(self, frame_started_callback_enabled, frame_available_callback_enabled, frame_callback_enabled, frame_error_count_callback_enabled):
        r"""
        Enables/Disables the different callbacks. By default the
        :cb:`Frame Started` callback and :cb:`Frame Available` callback are enabled while
        the :cb:`Frame` callback and :cb:`Frame Error Count` callback are disabled.

        If you want to use the :cb:`Frame` callback you can enable it and disable
        the :cb:`Frame Available` callback at the same time. It becomes redundant in
        this case.
        """
        self.check_validity()

        frame_started_callback_enabled = bool(frame_started_callback_enabled)
        frame_available_callback_enabled = bool(frame_available_callback_enabled)
        frame_callback_enabled = bool(frame_callback_enabled)
        frame_error_count_callback_enabled = bool(frame_error_count_callback_enabled)

        self.ipcon.send_request(self, BrickletDMX.FUNCTION_SET_FRAME_CALLBACK_CONFIG, (frame_started_callback_enabled, frame_available_callback_enabled, frame_callback_enabled, frame_error_count_callback_enabled), '! ! ! !', 0, '')

    def get_frame_callback_config(self):
        r"""
        Returns the frame callback config as set by :func:`Set Frame Callback Config`.
        """
        self.check_validity()

        return GetFrameCallbackConfig(*self.ipcon.send_request(self, BrickletDMX.FUNCTION_GET_FRAME_CALLBACK_CONFIG, (), '', 12, '! ! ! !'))

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletDMX.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletDMX.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletDMX.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletDMX.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletDMX.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletDMX.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletDMX.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletDMX.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletDMX.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletDMX.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletDMX.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletDMX.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def write_frame(self, frame):
        r"""
        Writes a DMX frame. The maximum frame size is 512 byte. Each byte represents one channel.

        The next frame can be written after the :cb:`Frame Started` callback was called. The frame
        is double buffered, so a new frame can be written as soon as the writing of the prior frame
        starts.

        The data will be transfered when the next frame duration ends, see :func:`Set Frame Duration`.

        Generic approach:

        * Set the frame duration to a value that represents the number of frames per second you want to achieve.
        * Set channels for first frame.
        * Wait for the :cb:`Frame Started` callback.
        * Set channels for next frame.
        * Wait for the :cb:`Frame Started` callback.
        * and so on.

        This approach ensures that you can set new DMX data with a fixed frame rate.

        This function can only be called in master mode.
        """
        frame = list(map(int, frame))

        if len(frame) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Frame can be at most 65535 items long')

        frame_length = len(frame)
        frame_chunk_offset = 0

        if frame_length == 0:
            frame_chunk_data = [0] * 60
            ret = self.write_frame_low_level(frame_length, frame_chunk_offset, frame_chunk_data)
        else:
            with self.stream_lock:
                while frame_chunk_offset < frame_length:
                    frame_chunk_data = create_chunk_data(frame, frame_chunk_offset, 60, 0)
                    ret = self.write_frame_low_level(frame_length, frame_chunk_offset, frame_chunk_data)
                    frame_chunk_offset += 60

        return ret

    def read_frame(self):
        r"""
        Returns the last frame that was written by the DMX master. The size of the array
        is equivalent to the number of channels in the frame. Each byte represents one channel.

        The next frame is available after the :cb:`Frame Available` callback was called.

        Generic approach:

        * Call :func:`Read Frame` to get first frame.
        * Wait for the :cb:`Frame Available` callback.
        * Call :func:`Read Frame` to get second frame.
        * Wait for the :cb:`Frame Available` callback.
        * and so on.

        Instead of polling this function you can also use the :cb:`Frame` callback.
        You can enable it with :func:`Set Frame Callback Config`.

        The frame number starts at 0 and it is increased by one with each received frame.

        This function can only be called in slave mode.
        """
        with self.stream_lock:
            ret = self.read_frame_low_level()
            frame_length = ret.frame_length
            frame_out_of_sync = ret.frame_chunk_offset != 0
            frame_data = ret.frame_chunk_data

            while not frame_out_of_sync and len(frame_data) < frame_length:
                ret = self.read_frame_low_level()
                frame_length = ret.frame_length
                frame_out_of_sync = ret.frame_chunk_offset != len(frame_data)
                frame_data += ret.frame_chunk_data

            if frame_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.frame_chunk_offset + 56 < frame_length:
                    ret = self.read_frame_low_level()
                    frame_length = ret.frame_length

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Frame stream is out-of-sync')

        return ReadFrame(frame_data[:frame_length], ret.frame_number)

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

DMX = BrickletDMX # for backward compatibility
