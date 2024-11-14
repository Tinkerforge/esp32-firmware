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

ReadFrameLowLevel = namedtuple('ReadFrameLowLevel', ['success', 'frame_type', 'identifier', 'data_length', 'data_data'])
GetTransceiverConfiguration = namedtuple('TransceiverConfiguration', ['baud_rate', 'sample_point', 'transceiver_mode'])
GetQueueConfigurationLowLevel = namedtuple('QueueConfigurationLowLevel', ['write_buffer_size', 'write_buffer_timeout', 'write_backlog_size', 'read_buffer_sizes_length', 'read_buffer_sizes_data', 'read_backlog_size'])
GetReadFilterConfiguration = namedtuple('ReadFilterConfiguration', ['filter_mode', 'filter_mask', 'filter_identifier'])
GetErrorLogLowLevel = namedtuple('ErrorLogLowLevel', ['transceiver_state', 'transceiver_write_error_level', 'transceiver_read_error_level', 'transceiver_stuffing_error_count', 'transceiver_format_error_count', 'transceiver_ack_error_count', 'transceiver_bit1_error_count', 'transceiver_bit0_error_count', 'transceiver_crc_error_count', 'write_buffer_timeout_error_count', 'read_buffer_overflow_error_count', 'read_buffer_overflow_error_occurred_length', 'read_buffer_overflow_error_occurred_data', 'read_backlog_overflow_error_count'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])
ReadFrame = namedtuple('ReadFrame', ['success', 'frame_type', 'identifier', 'data'])
GetQueueConfiguration = namedtuple('QueueConfiguration', ['write_buffer_size', 'write_buffer_timeout', 'write_backlog_size', 'read_buffer_sizes', 'read_backlog_size'])
GetErrorLog = namedtuple('ErrorLog', ['transceiver_state', 'transceiver_write_error_level', 'transceiver_read_error_level', 'transceiver_stuffing_error_count', 'transceiver_format_error_count', 'transceiver_ack_error_count', 'transceiver_bit1_error_count', 'transceiver_bit0_error_count', 'transceiver_crc_error_count', 'write_buffer_timeout_error_count', 'read_buffer_overflow_error_count', 'read_buffer_overflow_error_occurred', 'read_backlog_overflow_error_count'])

class BrickletCANV2(Device):
    r"""
    Communicates with CAN bus devices
    """

    DEVICE_IDENTIFIER = 2107
    DEVICE_DISPLAY_NAME = 'CAN Bricklet 2.0'
    DEVICE_URL_PART = 'can_v2' # internal

    CALLBACK_FRAME_READ_LOW_LEVEL = 16
    CALLBACK_FRAME_READABLE = 19
    CALLBACK_ERROR_OCCURRED = 22

    CALLBACK_FRAME_READ = -16

    FUNCTION_WRITE_FRAME_LOW_LEVEL = 1
    FUNCTION_READ_FRAME_LOW_LEVEL = 2
    FUNCTION_SET_FRAME_READ_CALLBACK_CONFIGURATION = 3
    FUNCTION_GET_FRAME_READ_CALLBACK_CONFIGURATION = 4
    FUNCTION_SET_TRANSCEIVER_CONFIGURATION = 5
    FUNCTION_GET_TRANSCEIVER_CONFIGURATION = 6
    FUNCTION_SET_QUEUE_CONFIGURATION_LOW_LEVEL = 7
    FUNCTION_GET_QUEUE_CONFIGURATION_LOW_LEVEL = 8
    FUNCTION_SET_READ_FILTER_CONFIGURATION = 9
    FUNCTION_GET_READ_FILTER_CONFIGURATION = 10
    FUNCTION_GET_ERROR_LOG_LOW_LEVEL = 11
    FUNCTION_SET_COMMUNICATION_LED_CONFIG = 12
    FUNCTION_GET_COMMUNICATION_LED_CONFIG = 13
    FUNCTION_SET_ERROR_LED_CONFIG = 14
    FUNCTION_GET_ERROR_LED_CONFIG = 15
    FUNCTION_SET_FRAME_READABLE_CALLBACK_CONFIGURATION = 17
    FUNCTION_GET_FRAME_READABLE_CALLBACK_CONFIGURATION = 18
    FUNCTION_SET_ERROR_OCCURRED_CALLBACK_CONFIGURATION = 20
    FUNCTION_GET_ERROR_OCCURRED_CALLBACK_CONFIGURATION = 21
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

    FRAME_TYPE_STANDARD_DATA = 0
    FRAME_TYPE_STANDARD_REMOTE = 1
    FRAME_TYPE_EXTENDED_DATA = 2
    FRAME_TYPE_EXTENDED_REMOTE = 3
    TRANSCEIVER_MODE_NORMAL = 0
    TRANSCEIVER_MODE_LOOPBACK = 1
    TRANSCEIVER_MODE_READ_ONLY = 2
    FILTER_MODE_ACCEPT_ALL = 0
    FILTER_MODE_MATCH_STANDARD_ONLY = 1
    FILTER_MODE_MATCH_EXTENDED_ONLY = 2
    FILTER_MODE_MATCH_STANDARD_AND_EXTENDED = 3
    TRANSCEIVER_STATE_ACTIVE = 0
    TRANSCEIVER_STATE_PASSIVE = 1
    TRANSCEIVER_STATE_DISABLED = 2
    COMMUNICATION_LED_CONFIG_OFF = 0
    COMMUNICATION_LED_CONFIG_ON = 1
    COMMUNICATION_LED_CONFIG_SHOW_HEARTBEAT = 2
    COMMUNICATION_LED_CONFIG_SHOW_COMMUNICATION = 3
    ERROR_LED_CONFIG_OFF = 0
    ERROR_LED_CONFIG_ON = 1
    ERROR_LED_CONFIG_SHOW_HEARTBEAT = 2
    ERROR_LED_CONFIG_SHOW_TRANSCEIVER_STATE = 3
    ERROR_LED_CONFIG_SHOW_ERROR = 4
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
        Device.__init__(self, uid, ipcon, BrickletCANV2.DEVICE_IDENTIFIER, BrickletCANV2.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 1)

        self.response_expected[BrickletCANV2.FUNCTION_WRITE_FRAME_LOW_LEVEL] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_READ_FRAME_LOW_LEVEL] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_SET_FRAME_READ_CALLBACK_CONFIGURATION] = BrickletCANV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_GET_FRAME_READ_CALLBACK_CONFIGURATION] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_SET_TRANSCEIVER_CONFIGURATION] = BrickletCANV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletCANV2.FUNCTION_GET_TRANSCEIVER_CONFIGURATION] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_SET_QUEUE_CONFIGURATION_LOW_LEVEL] = BrickletCANV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_GET_QUEUE_CONFIGURATION_LOW_LEVEL] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_SET_READ_FILTER_CONFIGURATION] = BrickletCANV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletCANV2.FUNCTION_GET_READ_FILTER_CONFIGURATION] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_GET_ERROR_LOG_LOW_LEVEL] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_SET_COMMUNICATION_LED_CONFIG] = BrickletCANV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletCANV2.FUNCTION_GET_COMMUNICATION_LED_CONFIG] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_SET_ERROR_LED_CONFIG] = BrickletCANV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletCANV2.FUNCTION_GET_ERROR_LED_CONFIG] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_SET_FRAME_READABLE_CALLBACK_CONFIGURATION] = BrickletCANV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_GET_FRAME_READABLE_CALLBACK_CONFIGURATION] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_SET_ERROR_OCCURRED_CALLBACK_CONFIGURATION] = BrickletCANV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_GET_ERROR_OCCURRED_CALLBACK_CONFIGURATION] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_SET_BOOTLOADER_MODE] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_GET_BOOTLOADER_MODE] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletCANV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletCANV2.FUNCTION_WRITE_FIRMWARE] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletCANV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletCANV2.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_RESET] = BrickletCANV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletCANV2.FUNCTION_WRITE_UID] = BrickletCANV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletCANV2.FUNCTION_READ_UID] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCANV2.FUNCTION_GET_IDENTITY] = BrickletCANV2.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletCANV2.CALLBACK_FRAME_READ_LOW_LEVEL] = (29, 'B I B 15B')
        self.callback_formats[BrickletCANV2.CALLBACK_FRAME_READABLE] = (8, '')
        self.callback_formats[BrickletCANV2.CALLBACK_ERROR_OCCURRED] = (8, '')

        self.high_level_callbacks[BrickletCANV2.CALLBACK_FRAME_READ] = [(None, None, 'stream_length', 'stream_chunk_data'), {'fixed_length': None, 'single_chunk': True}, None]
        ipcon.add_device(self)

    def write_frame_low_level(self, frame_type, identifier, data_length, data_data):
        r"""
        Writes a data or remote frame to the write queue to be transmitted over the
        CAN transceiver.

        The Bricklet supports the standard 11-bit (CAN 2.0A) and the additional extended
        29-bit (CAN 2.0B) identifiers. For standard frames the Bricklet uses bit 0 to 10
        from the ``identifier`` parameter as standard 11-bit identifier. For extended
        frames the Bricklet uses bit 0 to 28 from the ``identifier`` parameter as
        extended 29-bit identifier.

        The ``data`` parameter can be up to 15 bytes long. For data frames up to 8 bytes
        will be used as the actual data. The length (DLC) field in the data or remote
        frame will be set to the actual length of the ``data`` parameter. This allows
        to transmit data and remote frames with excess length. For remote frames only
        the length of the ``data`` parameter is used. The actual ``data`` bytes are
        ignored.

        Returns *true* if the frame was successfully added to the write queue. Returns
        *false* if the frame could not be added because write queue is already full or
        because the write buffer or the write backlog are configured with a size of
        zero (see :func:`Set Queue Configuration`).

        The write queue can overflow if frames are written to it at a higher rate
        than the Bricklet can transmitted them over the CAN transceiver. This may
        happen if the CAN transceiver is configured as read-only or is using a low baud
        rate (see :func:`Set Transceiver Configuration`). It can also happen if the CAN
        bus is congested and the frame cannot be transmitted because it constantly loses
        arbitration or because the CAN transceiver is currently disabled due to a high
        write error level (see :func:`Get Error Log`).
        """
        self.check_validity()

        frame_type = int(frame_type)
        identifier = int(identifier)
        data_length = int(data_length)
        data_data = list(map(int, data_data))

        return self.ipcon.send_request(self, BrickletCANV2.FUNCTION_WRITE_FRAME_LOW_LEVEL, (frame_type, identifier, data_length, data_data), 'B I B 15B', 9, '!')

    def read_frame_low_level(self):
        r"""
        Tries to read the next data or remote frame from the read queue and returns it.
        If a frame was successfully read, then the ``success`` return value is set to
        *true* and the other return values contain the frame. If the read queue is
        empty and no frame could be read, then the ``success`` return value is set to
        *false* and the other return values contain invalid data.

        The ``identifier`` return value follows the identifier format described for
        :func:`Write Frame`.

        The ``data`` return value can be up to 15 bytes long. For data frames up to the
        first 8 bytes are the actual received data. All bytes after the 8th byte are
        always zero and only there to indicate the length of a data or remote frame
        with excess length. For remote frames the length of the ``data`` return value
        represents the requested length. The actual ``data`` bytes are always zero.

        A configurable read filter can be used to define which frames should be
        received by the CAN transceiver and put into the read queue (see
        :func:`Set Read Filter Configuration`).

        Instead of polling with this function, you can also use callbacks. See the
        :func:`Set Frame Read Callback Configuration` function and the :cb:`Frame Read`
        callback.
        """
        self.check_validity()

        return ReadFrameLowLevel(*self.ipcon.send_request(self, BrickletCANV2.FUNCTION_READ_FRAME_LOW_LEVEL, (), '', 30, '! B I B 15B'))

    def set_frame_read_callback_configuration(self, enabled):
        r"""
        Enables and disables the :cb:`Frame Read` callback.

        By default the callback is disabled. Enabling this callback will disable the :cb:`Frame Readable` callback.
        """
        self.check_validity()

        enabled = bool(enabled)

        self.ipcon.send_request(self, BrickletCANV2.FUNCTION_SET_FRAME_READ_CALLBACK_CONFIGURATION, (enabled,), '!', 0, '')

    def get_frame_read_callback_configuration(self):
        r"""
        Returns *true* if the :cb:`Frame Read` callback is enabled, *false* otherwise.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletCANV2.FUNCTION_GET_FRAME_READ_CALLBACK_CONFIGURATION, (), '', 9, '!')

    def set_transceiver_configuration(self, baud_rate, sample_point, transceiver_mode):
        r"""
        Sets the transceiver configuration for the CAN bus communication.

        The CAN transceiver has three different modes:

        * Normal: Reads from and writes to the CAN bus and performs active bus
          error detection and acknowledgement.
        * Loopback: All reads and writes are performed internally. The transceiver
          is disconnected from the actual CAN bus.
        * Read-Only: Only reads from the CAN bus, but does neither active bus error
          detection nor acknowledgement. Only the receiving part of the transceiver
          is connected to the CAN bus.
        """
        self.check_validity()

        baud_rate = int(baud_rate)
        sample_point = int(sample_point)
        transceiver_mode = int(transceiver_mode)

        self.ipcon.send_request(self, BrickletCANV2.FUNCTION_SET_TRANSCEIVER_CONFIGURATION, (baud_rate, sample_point, transceiver_mode), 'I H B', 0, '')

    def get_transceiver_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Transceiver Configuration`.
        """
        self.check_validity()

        return GetTransceiverConfiguration(*self.ipcon.send_request(self, BrickletCANV2.FUNCTION_GET_TRANSCEIVER_CONFIGURATION, (), '', 15, 'I H B'))

    def set_queue_configuration_low_level(self, write_buffer_size, write_buffer_timeout, write_backlog_size, read_buffer_sizes_length, read_buffer_sizes_data, read_backlog_size):
        r"""
        Sets the write and read queue configuration.

        The CAN transceiver has 32 buffers in total in hardware for transmitting and
        receiving frames. Additionally, the Bricklet has a backlog for 768 frames in
        total in software. The buffers and the backlog can be freely assigned to the
        write and read queues.

        :func:`Write Frame` writes a frame into the write backlog. The Bricklet moves
        the frame from the backlog into a free write buffer. The CAN transceiver then
        transmits the frame from the write buffer to the CAN bus. If there are no
        write buffers (``write_buffer_size`` is zero) or there is no write backlog
        (``write_backlog_size`` is zero) then no frames can be transmitted and
        :func:`Write Frame` returns always *false*.

        The CAN transceiver receives a frame from the CAN bus and stores it into a
        free read buffer. The Bricklet moves the frame from the read buffer into the
        read backlog. :func:`Read Frame` reads the frame from the read backlog and
        returns it. If there are no read buffers (``read_buffer_sizes`` is empty) or
        there is no read backlog (``read_backlog_size`` is zero) then no frames can be
        received and :func:`Read Frame` returns always *false*.

        There can be multiple read buffers, because the CAN transceiver cannot receive
        data and remote frames into the same read buffer. A positive read buffer size
        represents a data frame read buffer and a negative read buffer size represents
        a remote frame read buffer. A read buffer size of zero is not allowed. By
        default the first read buffer is configured for data frames and the second read
        buffer is configured for remote frame. There can be up to 32 different read
        buffers, assuming that no write buffer is used. Each read buffer has its own
        filter configuration (see :func:`Set Read Filter Configuration`).

        A valid queue configuration fulfills these conditions::

         write_buffer_size + abs(read_buffer_size_0) + abs(read_buffer_size_1) + ... + abs(read_buffer_size_31) <= 32
         write_backlog_size + read_backlog_size <= 768

        The write buffer timeout has three different modes that define how a failed
        frame transmission should be handled:

        * Single-Shot (< 0): Only one transmission attempt will be made. If the
          transmission fails then the frame is discarded.
        * Infinite (= 0): Infinite transmission attempts will be made. The frame will
          never be discarded.
        * Milliseconds (> 0): A limited number of transmission attempts will be made.
          If the frame could not be transmitted successfully after the configured
          number of milliseconds then the frame is discarded.

        The current content of the queues is lost when this function is called.
        """
        self.check_validity()

        write_buffer_size = int(write_buffer_size)
        write_buffer_timeout = int(write_buffer_timeout)
        write_backlog_size = int(write_backlog_size)
        read_buffer_sizes_length = int(read_buffer_sizes_length)
        read_buffer_sizes_data = list(map(int, read_buffer_sizes_data))
        read_backlog_size = int(read_backlog_size)

        self.ipcon.send_request(self, BrickletCANV2.FUNCTION_SET_QUEUE_CONFIGURATION_LOW_LEVEL, (write_buffer_size, write_buffer_timeout, write_backlog_size, read_buffer_sizes_length, read_buffer_sizes_data, read_backlog_size), 'B i H B 32b H', 0, '')

    def get_queue_configuration_low_level(self):
        r"""
        Returns the queue configuration as set by :func:`Set Queue Configuration`.
        """
        self.check_validity()

        return GetQueueConfigurationLowLevel(*self.ipcon.send_request(self, BrickletCANV2.FUNCTION_GET_QUEUE_CONFIGURATION_LOW_LEVEL, (), '', 50, 'B i H B 32b H'))

    def set_read_filter_configuration(self, buffer_index, filter_mode, filter_mask, filter_identifier):
        r"""
        Set the read filter configuration for the given read buffer index. This can be
        used to define which frames should be received by the CAN transceiver and put
        into the read buffer.

        The read filter has four different modes that define if and how the filter mask
        and the filter identifier are applied:

        * Accept-All: All frames are received.
        * Match-Standard-Only: Only standard frames with a matching identifier are
          received.
        * Match-Extended-Only: Only extended frames with a matching identifier are
          received.
        * Match-Standard-And-Extended: Standard and extended frames with a matching
          identifier are received.

        The filter mask and filter identifier are used as bit masks. Their usage
        depends on the mode:

        * Accept-All: Mask and identifier are ignored.
        * Match-Standard-Only: Bit 0 to 10 (11 bits) of filter mask and filter
          identifier are used to match the 11-bit identifier of standard frames.
        * Match-Extended-Only: Bit 0 to 28 (29 bits) of filter mask and filter
          identifier are used to match the 29-bit identifier of extended frames.
        * Match-Standard-And-Extended: Bit 18 to 28 (11 bits) of filter mask and filter
          identifier are used to match the 11-bit identifier of standard frames, bit 0
          to 17 (18 bits) are ignored in this case. Bit 0 to 28 (29 bits) of filter
          mask and filter identifier are used to match the 29-bit identifier of extended
          frames.

        The filter mask and filter identifier are applied in this way: The filter mask
        is used to select the frame identifier bits that should be compared to the
        corresponding filter identifier bits. All unselected bits are automatically
        accepted. All selected bits have to match the filter identifier to be accepted.
        If all bits for the selected mode are accepted then the frame is accepted and
        is added to the read buffer.

        .. csv-table::
         :header: "Filter Mask Bit", "Filter Identifier Bit", "Frame Identifier Bit", "Result"
         :widths: 10, 10, 10, 10

         0, X, X, Accept
         1, 0, 0, Accept
         1, 0, 1, Reject
         1, 1, 0, Reject
         1, 1, 1, Accept

        For example, to receive standard frames with identifier 0x123 only, the mode
        can be set to Match-Standard-Only with 0x7FF as mask and 0x123 as identifier.
        The mask of 0x7FF selects all 11 identifier bits for matching so that the
        identifier has to be exactly 0x123 to be accepted.

        To accept identifier 0x123 and identifier 0x456 at the same time, just set
        filter 2 to 0x456 and keep mask and filter 1 unchanged.

        There can be up to 32 different read filters configured at the same time,
        because there can be up to 32 read buffer (see :func:`Set Queue Configuration`).

        The default mode is accept-all for all read buffers.
        """
        self.check_validity()

        buffer_index = int(buffer_index)
        filter_mode = int(filter_mode)
        filter_mask = int(filter_mask)
        filter_identifier = int(filter_identifier)

        self.ipcon.send_request(self, BrickletCANV2.FUNCTION_SET_READ_FILTER_CONFIGURATION, (buffer_index, filter_mode, filter_mask, filter_identifier), 'B B I I', 0, '')

    def get_read_filter_configuration(self, buffer_index):
        r"""
        Returns the read filter configuration as set by :func:`Set Read Filter Configuration`.
        """
        self.check_validity()

        buffer_index = int(buffer_index)

        return GetReadFilterConfiguration(*self.ipcon.send_request(self, BrickletCANV2.FUNCTION_GET_READ_FILTER_CONFIGURATION, (buffer_index,), 'B', 17, 'B I I'))

    def get_error_log_low_level(self):
        r"""
        Returns information about different kinds of errors.

        The write and read error levels indicate the current level of stuffing, form,
        acknowledgement, bit and checksum errors during CAN bus write and read
        operations. For each of this error kinds there is also an individual counter.

        When the write error level extends 255 then the CAN transceiver gets disabled
        and no frames can be transmitted or received anymore. The CAN transceiver will
        automatically be activated again after the CAN bus is idle for a while.

        The write buffer timeout, read buffer and backlog overflow counts represents the
        number of these errors:

        * A write buffer timeout occurs if a frame could not be transmitted before the
          configured write buffer timeout expired (see :func:`Set Queue Configuration`).
        * A read buffer overflow occurs if a read buffer of the CAN transceiver
          still contains the last received frame when the next frame arrives. In this
          case the last received frame is lost. This happens if the CAN transceiver
          receives more frames than the Bricklet can handle. Using the read filter
          (see :func:`Set Read Filter Configuration`) can help to reduce the amount of
          received frames. This count is not exact, but a lower bound, because the
          Bricklet might not able detect all overflows if they occur in rapid succession.
        * A read backlog overflow occurs if the read backlog of the Bricklet is already
          full when the next frame should be read from a read buffer of the CAN
          transceiver. In this case the frame in the read buffer is lost. This
          happens if the CAN transceiver receives more frames to be added to the read
          backlog than are removed from the read backlog using the :func:`Read Frame`
          function. Using the :cb:`Frame Read` callback ensures that the read backlog
          can not overflow.

        The read buffer overflow counter counts the overflows of all configured read
        buffers. Which read buffer exactly suffered from an overflow can be figured
        out from the read buffer overflow occurrence list
        (``read_buffer_overflow_error_occurred``). Reading the error log clears the
        occurence list.
        """
        self.check_validity()

        return GetErrorLogLowLevel(*self.ipcon.send_request(self, BrickletCANV2.FUNCTION_GET_ERROR_LOG_LOW_LEVEL, (), '', 52, 'B B B I I I I I I I I B 32! I'))

    def set_communication_led_config(self, config):
        r"""
        Sets the communication LED configuration. By default the LED shows
        CAN-Bus traffic, it flickers once for every 40 transmitted or received frames.

        You can also turn the LED permanently on/off or show a heartbeat.

        If the Bricklet is in bootloader mode, the LED is off.
        """
        self.check_validity()

        config = int(config)

        self.ipcon.send_request(self, BrickletCANV2.FUNCTION_SET_COMMUNICATION_LED_CONFIG, (config,), 'B', 0, '')

    def get_communication_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Communication LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletCANV2.FUNCTION_GET_COMMUNICATION_LED_CONFIG, (), '', 9, 'B')

    def set_error_led_config(self, config):
        r"""
        Sets the error LED configuration.

        By default (show-transceiver-state) the error LED turns on if the CAN
        transceiver is passive or disabled state (see :func:`Get Error Log`). If
        the CAN transceiver is in active state the LED turns off.

        If the LED is configured as show-error then the error LED turns on if any error
        occurs. If you call this function with the show-error option again, the LED will
        turn off until the next error occurs.

        You can also turn the LED permanently on/off or show a heartbeat.

        If the Bricklet is in bootloader mode, the LED is off.
        """
        self.check_validity()

        config = int(config)

        self.ipcon.send_request(self, BrickletCANV2.FUNCTION_SET_ERROR_LED_CONFIG, (config,), 'B', 0, '')

    def get_error_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Error LED Config`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletCANV2.FUNCTION_GET_ERROR_LED_CONFIG, (), '', 9, 'B')

    def set_frame_readable_callback_configuration(self, enabled):
        r"""
        Enables and disables the :cb:`Frame Readable` callback.

        By default the callback is disabled. Enabling this callback will disable the :cb:`Frame Read` callback.

        .. versionadded:: 2.0.3$nbsp;(Plugin)
        """
        self.check_validity()

        enabled = bool(enabled)

        self.ipcon.send_request(self, BrickletCANV2.FUNCTION_SET_FRAME_READABLE_CALLBACK_CONFIGURATION, (enabled,), '!', 0, '')

    def get_frame_readable_callback_configuration(self):
        r"""
        Returns *true* if the :cb:`Frame Readable` callback is enabled, *false* otherwise.

        .. versionadded:: 2.0.3$nbsp;(Plugin)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletCANV2.FUNCTION_GET_FRAME_READABLE_CALLBACK_CONFIGURATION, (), '', 9, '!')

    def set_error_occurred_callback_configuration(self, enabled):
        r"""
        Enables and disables the :cb:`Error Occurred` callback.

        By default the callback is disabled.

        .. versionadded:: 2.0.3$nbsp;(Plugin)
        """
        self.check_validity()

        enabled = bool(enabled)

        self.ipcon.send_request(self, BrickletCANV2.FUNCTION_SET_ERROR_OCCURRED_CALLBACK_CONFIGURATION, (enabled,), '!', 0, '')

    def get_error_occurred_callback_configuration(self):
        r"""
        Returns *true* if the :cb:`Error Occurred` callback is enabled, *false* otherwise.

        .. versionadded:: 2.0.3$nbsp;(Plugin)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletCANV2.FUNCTION_GET_ERROR_OCCURRED_CALLBACK_CONFIGURATION, (), '', 9, '!')

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletCANV2.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletCANV2.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletCANV2.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletCANV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletCANV2.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletCANV2.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletCANV2.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletCANV2.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletCANV2.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletCANV2.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletCANV2.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletCANV2.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def write_frame(self, frame_type, identifier, data):
        r"""
        Writes a data or remote frame to the write queue to be transmitted over the
        CAN transceiver.

        The Bricklet supports the standard 11-bit (CAN 2.0A) and the additional extended
        29-bit (CAN 2.0B) identifiers. For standard frames the Bricklet uses bit 0 to 10
        from the ``identifier`` parameter as standard 11-bit identifier. For extended
        frames the Bricklet uses bit 0 to 28 from the ``identifier`` parameter as
        extended 29-bit identifier.

        The ``data`` parameter can be up to 15 bytes long. For data frames up to 8 bytes
        will be used as the actual data. The length (DLC) field in the data or remote
        frame will be set to the actual length of the ``data`` parameter. This allows
        to transmit data and remote frames with excess length. For remote frames only
        the length of the ``data`` parameter is used. The actual ``data`` bytes are
        ignored.

        Returns *true* if the frame was successfully added to the write queue. Returns
        *false* if the frame could not be added because write queue is already full or
        because the write buffer or the write backlog are configured with a size of
        zero (see :func:`Set Queue Configuration`).

        The write queue can overflow if frames are written to it at a higher rate
        than the Bricklet can transmitted them over the CAN transceiver. This may
        happen if the CAN transceiver is configured as read-only or is using a low baud
        rate (see :func:`Set Transceiver Configuration`). It can also happen if the CAN
        bus is congested and the frame cannot be transmitted because it constantly loses
        arbitration or because the CAN transceiver is currently disabled due to a high
        write error level (see :func:`Get Error Log`).
        """
        frame_type = int(frame_type)
        identifier = int(identifier)
        data = list(map(int, data))

        data_length = len(data)
        data_data = list(data) # make a copy so we can potentially extend it

        if data_length > 15:
            raise Error(Error.INVALID_PARAMETER, 'Data can be at most 15 items long')

        if data_length < 15:
            data_data += [0] * (15 - data_length)

        return self.write_frame_low_level(frame_type, identifier, data_length, data_data)

    def read_frame(self):
        r"""
        Tries to read the next data or remote frame from the read queue and returns it.
        If a frame was successfully read, then the ``success`` return value is set to
        *true* and the other return values contain the frame. If the read queue is
        empty and no frame could be read, then the ``success`` return value is set to
        *false* and the other return values contain invalid data.

        The ``identifier`` return value follows the identifier format described for
        :func:`Write Frame`.

        The ``data`` return value can be up to 15 bytes long. For data frames up to the
        first 8 bytes are the actual received data. All bytes after the 8th byte are
        always zero and only there to indicate the length of a data or remote frame
        with excess length. For remote frames the length of the ``data`` return value
        represents the requested length. The actual ``data`` bytes are always zero.

        A configurable read filter can be used to define which frames should be
        received by the CAN transceiver and put into the read queue (see
        :func:`Set Read Filter Configuration`).

        Instead of polling with this function, you can also use callbacks. See the
        :func:`Set Frame Read Callback Configuration` function and the :cb:`Frame Read`
        callback.
        """
        ret = self.read_frame_low_level()

        return ReadFrame(ret.success, ret.frame_type, ret.identifier, ret.data_data[:ret.data_length])

    def set_queue_configuration(self, write_buffer_size, write_buffer_timeout, write_backlog_size, read_buffer_sizes, read_backlog_size):
        r"""
        Sets the write and read queue configuration.

        The CAN transceiver has 32 buffers in total in hardware for transmitting and
        receiving frames. Additionally, the Bricklet has a backlog for 768 frames in
        total in software. The buffers and the backlog can be freely assigned to the
        write and read queues.

        :func:`Write Frame` writes a frame into the write backlog. The Bricklet moves
        the frame from the backlog into a free write buffer. The CAN transceiver then
        transmits the frame from the write buffer to the CAN bus. If there are no
        write buffers (``write_buffer_size`` is zero) or there is no write backlog
        (``write_backlog_size`` is zero) then no frames can be transmitted and
        :func:`Write Frame` returns always *false*.

        The CAN transceiver receives a frame from the CAN bus and stores it into a
        free read buffer. The Bricklet moves the frame from the read buffer into the
        read backlog. :func:`Read Frame` reads the frame from the read backlog and
        returns it. If there are no read buffers (``read_buffer_sizes`` is empty) or
        there is no read backlog (``read_backlog_size`` is zero) then no frames can be
        received and :func:`Read Frame` returns always *false*.

        There can be multiple read buffers, because the CAN transceiver cannot receive
        data and remote frames into the same read buffer. A positive read buffer size
        represents a data frame read buffer and a negative read buffer size represents
        a remote frame read buffer. A read buffer size of zero is not allowed. By
        default the first read buffer is configured for data frames and the second read
        buffer is configured for remote frame. There can be up to 32 different read
        buffers, assuming that no write buffer is used. Each read buffer has its own
        filter configuration (see :func:`Set Read Filter Configuration`).

        A valid queue configuration fulfills these conditions::

         write_buffer_size + abs(read_buffer_size_0) + abs(read_buffer_size_1) + ... + abs(read_buffer_size_31) <= 32
         write_backlog_size + read_backlog_size <= 768

        The write buffer timeout has three different modes that define how a failed
        frame transmission should be handled:

        * Single-Shot (< 0): Only one transmission attempt will be made. If the
          transmission fails then the frame is discarded.
        * Infinite (= 0): Infinite transmission attempts will be made. The frame will
          never be discarded.
        * Milliseconds (> 0): A limited number of transmission attempts will be made.
          If the frame could not be transmitted successfully after the configured
          number of milliseconds then the frame is discarded.

        The current content of the queues is lost when this function is called.
        """
        write_buffer_size = int(write_buffer_size)
        write_buffer_timeout = int(write_buffer_timeout)
        write_backlog_size = int(write_backlog_size)
        read_buffer_sizes = list(map(int, read_buffer_sizes))
        read_backlog_size = int(read_backlog_size)

        read_buffer_sizes_length = len(read_buffer_sizes)
        read_buffer_sizes_data = list(read_buffer_sizes) # make a copy so we can potentially extend it

        if read_buffer_sizes_length > 32:
            raise Error(Error.INVALID_PARAMETER, 'Read Buffer Sizes can be at most 32 items long')

        if read_buffer_sizes_length < 32:
            read_buffer_sizes_data += [0] * (32 - read_buffer_sizes_length)

        return self.set_queue_configuration_low_level(write_buffer_size, write_buffer_timeout, write_backlog_size, read_buffer_sizes_length, read_buffer_sizes_data, read_backlog_size)

    def get_queue_configuration(self):
        r"""
        Returns the queue configuration as set by :func:`Set Queue Configuration`.
        """
        ret = self.get_queue_configuration_low_level()

        return GetQueueConfiguration(ret.write_buffer_size, ret.write_buffer_timeout, ret.write_backlog_size, ret.read_buffer_sizes_data[:ret.read_buffer_sizes_length], ret.read_backlog_size)

    def get_error_log(self):
        r"""
        Returns information about different kinds of errors.

        The write and read error levels indicate the current level of stuffing, form,
        acknowledgement, bit and checksum errors during CAN bus write and read
        operations. For each of this error kinds there is also an individual counter.

        When the write error level extends 255 then the CAN transceiver gets disabled
        and no frames can be transmitted or received anymore. The CAN transceiver will
        automatically be activated again after the CAN bus is idle for a while.

        The write buffer timeout, read buffer and backlog overflow counts represents the
        number of these errors:

        * A write buffer timeout occurs if a frame could not be transmitted before the
          configured write buffer timeout expired (see :func:`Set Queue Configuration`).
        * A read buffer overflow occurs if a read buffer of the CAN transceiver
          still contains the last received frame when the next frame arrives. In this
          case the last received frame is lost. This happens if the CAN transceiver
          receives more frames than the Bricklet can handle. Using the read filter
          (see :func:`Set Read Filter Configuration`) can help to reduce the amount of
          received frames. This count is not exact, but a lower bound, because the
          Bricklet might not able detect all overflows if they occur in rapid succession.
        * A read backlog overflow occurs if the read backlog of the Bricklet is already
          full when the next frame should be read from a read buffer of the CAN
          transceiver. In this case the frame in the read buffer is lost. This
          happens if the CAN transceiver receives more frames to be added to the read
          backlog than are removed from the read backlog using the :func:`Read Frame`
          function. Using the :cb:`Frame Read` callback ensures that the read backlog
          can not overflow.

        The read buffer overflow counter counts the overflows of all configured read
        buffers. Which read buffer exactly suffered from an overflow can be figured
        out from the read buffer overflow occurrence list
        (``read_buffer_overflow_error_occurred``). Reading the error log clears the
        occurence list.
        """
        ret = self.get_error_log_low_level()

        return GetErrorLog(ret.transceiver_state, ret.transceiver_write_error_level, ret.transceiver_read_error_level, ret.transceiver_stuffing_error_count, ret.transceiver_format_error_count, ret.transceiver_ack_error_count, ret.transceiver_bit1_error_count, ret.transceiver_bit0_error_count, ret.transceiver_crc_error_count, ret.write_buffer_timeout_error_count, ret.read_buffer_overflow_error_count, ret.read_buffer_overflow_error_occurred_data[:ret.read_buffer_overflow_error_occurred_length], ret.read_backlog_overflow_error_count)

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

CANV2 = BrickletCANV2 # for backward compatibility
