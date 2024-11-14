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

ReadLowLevel = namedtuple('ReadLowLevel', ['message_length', 'message_chunk_offset', 'message_chunk_data'])
GetRS485Configuration = namedtuple('RS485Configuration', ['baudrate', 'parity', 'stopbits', 'wordlength', 'duplex'])
GetModbusConfiguration = namedtuple('ModbusConfiguration', ['slave_address', 'master_request_timeout'])
GetBufferConfig = namedtuple('BufferConfig', ['send_buffer_size', 'receive_buffer_size'])
GetBufferStatus = namedtuple('BufferStatus', ['send_buffer_used', 'receive_buffer_used'])
GetErrorCount = namedtuple('ErrorCount', ['overrun_error_count', 'parity_error_count'])
GetModbusCommonErrorCount = namedtuple('ModbusCommonErrorCount', ['timeout_error_count', 'checksum_error_count', 'frame_too_big_error_count', 'illegal_function_error_count', 'illegal_data_address_error_count', 'illegal_data_value_error_count', 'slave_device_failure_error_count'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletRS485(Device):
    r"""
    Communicates with RS485/Modbus devices with full- or half-duplex
    """

    DEVICE_IDENTIFIER = 277
    DEVICE_DISPLAY_NAME = 'RS485 Bricklet'
    DEVICE_URL_PART = 'rs485' # internal

    CALLBACK_READ_LOW_LEVEL = 41
    CALLBACK_ERROR_COUNT = 42
    CALLBACK_MODBUS_SLAVE_READ_COILS_REQUEST = 43
    CALLBACK_MODBUS_MASTER_READ_COILS_RESPONSE_LOW_LEVEL = 44
    CALLBACK_MODBUS_SLAVE_READ_HOLDING_REGISTERS_REQUEST = 45
    CALLBACK_MODBUS_MASTER_READ_HOLDING_REGISTERS_RESPONSE_LOW_LEVEL = 46
    CALLBACK_MODBUS_SLAVE_WRITE_SINGLE_COIL_REQUEST = 47
    CALLBACK_MODBUS_MASTER_WRITE_SINGLE_COIL_RESPONSE = 48
    CALLBACK_MODBUS_SLAVE_WRITE_SINGLE_REGISTER_REQUEST = 49
    CALLBACK_MODBUS_MASTER_WRITE_SINGLE_REGISTER_RESPONSE = 50
    CALLBACK_MODBUS_SLAVE_WRITE_MULTIPLE_COILS_REQUEST_LOW_LEVEL = 51
    CALLBACK_MODBUS_MASTER_WRITE_MULTIPLE_COILS_RESPONSE = 52
    CALLBACK_MODBUS_SLAVE_WRITE_MULTIPLE_REGISTERS_REQUEST_LOW_LEVEL = 53
    CALLBACK_MODBUS_MASTER_WRITE_MULTIPLE_REGISTERS_RESPONSE = 54
    CALLBACK_MODBUS_SLAVE_READ_DISCRETE_INPUTS_REQUEST = 55
    CALLBACK_MODBUS_MASTER_READ_DISCRETE_INPUTS_RESPONSE_LOW_LEVEL = 56
    CALLBACK_MODBUS_SLAVE_READ_INPUT_REGISTERS_REQUEST = 57
    CALLBACK_MODBUS_MASTER_READ_INPUT_REGISTERS_RESPONSE_LOW_LEVEL = 58
    CALLBACK_FRAME_READABLE = 61

    CALLBACK_READ = -41
    CALLBACK_MODBUS_MASTER_READ_COILS_RESPONSE = -44
    CALLBACK_MODBUS_MASTER_READ_HOLDING_REGISTERS_RESPONSE = -46
    CALLBACK_MODBUS_SLAVE_WRITE_MULTIPLE_COILS_REQUEST = -51
    CALLBACK_MODBUS_SLAVE_WRITE_MULTIPLE_REGISTERS_REQUEST = -53
    CALLBACK_MODBUS_MASTER_READ_DISCRETE_INPUTS_RESPONSE = -56
    CALLBACK_MODBUS_MASTER_READ_INPUT_REGISTERS_RESPONSE = -58

    FUNCTION_WRITE_LOW_LEVEL = 1
    FUNCTION_READ_LOW_LEVEL = 2
    FUNCTION_ENABLE_READ_CALLBACK = 3
    FUNCTION_DISABLE_READ_CALLBACK = 4
    FUNCTION_IS_READ_CALLBACK_ENABLED = 5
    FUNCTION_SET_RS485_CONFIGURATION = 6
    FUNCTION_GET_RS485_CONFIGURATION = 7
    FUNCTION_SET_MODBUS_CONFIGURATION = 8
    FUNCTION_GET_MODBUS_CONFIGURATION = 9
    FUNCTION_SET_MODE = 10
    FUNCTION_GET_MODE = 11
    FUNCTION_SET_COMMUNICATION_LED_CONFIG = 12
    FUNCTION_GET_COMMUNICATION_LED_CONFIG = 13
    FUNCTION_SET_ERROR_LED_CONFIG = 14
    FUNCTION_GET_ERROR_LED_CONFIG = 15
    FUNCTION_SET_BUFFER_CONFIG = 16
    FUNCTION_GET_BUFFER_CONFIG = 17
    FUNCTION_GET_BUFFER_STATUS = 18
    FUNCTION_ENABLE_ERROR_COUNT_CALLBACK = 19
    FUNCTION_DISABLE_ERROR_COUNT_CALLBACK = 20
    FUNCTION_IS_ERROR_COUNT_CALLBACK_ENABLED = 21
    FUNCTION_GET_ERROR_COUNT = 22
    FUNCTION_GET_MODBUS_COMMON_ERROR_COUNT = 23
    FUNCTION_MODBUS_SLAVE_REPORT_EXCEPTION = 24
    FUNCTION_MODBUS_SLAVE_ANSWER_READ_COILS_REQUEST_LOW_LEVEL = 25
    FUNCTION_MODBUS_MASTER_READ_COILS = 26
    FUNCTION_MODBUS_SLAVE_ANSWER_READ_HOLDING_REGISTERS_REQUEST_LOW_LEVEL = 27
    FUNCTION_MODBUS_MASTER_READ_HOLDING_REGISTERS = 28
    FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_SINGLE_COIL_REQUEST = 29
    FUNCTION_MODBUS_MASTER_WRITE_SINGLE_COIL = 30
    FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_SINGLE_REGISTER_REQUEST = 31
    FUNCTION_MODBUS_MASTER_WRITE_SINGLE_REGISTER = 32
    FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_MULTIPLE_COILS_REQUEST = 33
    FUNCTION_MODBUS_MASTER_WRITE_MULTIPLE_COILS_LOW_LEVEL = 34
    FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_MULTIPLE_REGISTERS_REQUEST = 35
    FUNCTION_MODBUS_MASTER_WRITE_MULTIPLE_REGISTERS_LOW_LEVEL = 36
    FUNCTION_MODBUS_SLAVE_ANSWER_READ_DISCRETE_INPUTS_REQUEST_LOW_LEVEL = 37
    FUNCTION_MODBUS_MASTER_READ_DISCRETE_INPUTS = 38
    FUNCTION_MODBUS_SLAVE_ANSWER_READ_INPUT_REGISTERS_REQUEST_LOW_LEVEL = 39
    FUNCTION_MODBUS_MASTER_READ_INPUT_REGISTERS = 40
    FUNCTION_SET_FRAME_READABLE_CALLBACK_CONFIGURATION = 59
    FUNCTION_GET_FRAME_READABLE_CALLBACK_CONFIGURATION = 60
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

    PARITY_NONE = 0
    PARITY_ODD = 1
    PARITY_EVEN = 2
    STOPBITS_1 = 1
    STOPBITS_2 = 2
    WORDLENGTH_5 = 5
    WORDLENGTH_6 = 6
    WORDLENGTH_7 = 7
    WORDLENGTH_8 = 8
    DUPLEX_HALF = 0
    DUPLEX_FULL = 1
    MODE_RS485 = 0
    MODE_MODBUS_MASTER_RTU = 1
    MODE_MODBUS_SLAVE_RTU = 2
    COMMUNICATION_LED_CONFIG_OFF = 0
    COMMUNICATION_LED_CONFIG_ON = 1
    COMMUNICATION_LED_CONFIG_SHOW_HEARTBEAT = 2
    COMMUNICATION_LED_CONFIG_SHOW_COMMUNICATION = 3
    ERROR_LED_CONFIG_OFF = 0
    ERROR_LED_CONFIG_ON = 1
    ERROR_LED_CONFIG_SHOW_HEARTBEAT = 2
    ERROR_LED_CONFIG_SHOW_ERROR = 3
    EXCEPTION_CODE_TIMEOUT = -1
    EXCEPTION_CODE_SUCCESS = 0
    EXCEPTION_CODE_ILLEGAL_FUNCTION = 1
    EXCEPTION_CODE_ILLEGAL_DATA_ADDRESS = 2
    EXCEPTION_CODE_ILLEGAL_DATA_VALUE = 3
    EXCEPTION_CODE_SLAVE_DEVICE_FAILURE = 4
    EXCEPTION_CODE_ACKNOWLEDGE = 5
    EXCEPTION_CODE_SLAVE_DEVICE_BUSY = 6
    EXCEPTION_CODE_MEMORY_PARITY_ERROR = 8
    EXCEPTION_CODE_GATEWAY_PATH_UNAVAILABLE = 10
    EXCEPTION_CODE_GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND = 11
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
        Device.__init__(self, uid, ipcon, BrickletRS485.DEVICE_IDENTIFIER, BrickletRS485.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 1)

        self.response_expected[BrickletRS485.FUNCTION_WRITE_LOW_LEVEL] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_READ_LOW_LEVEL] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_ENABLE_READ_CALLBACK] = BrickletRS485.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletRS485.FUNCTION_DISABLE_READ_CALLBACK] = BrickletRS485.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletRS485.FUNCTION_IS_READ_CALLBACK_ENABLED] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_SET_RS485_CONFIGURATION] = BrickletRS485.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRS485.FUNCTION_GET_RS485_CONFIGURATION] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_SET_MODBUS_CONFIGURATION] = BrickletRS485.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRS485.FUNCTION_GET_MODBUS_CONFIGURATION] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_SET_MODE] = BrickletRS485.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRS485.FUNCTION_GET_MODE] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_SET_COMMUNICATION_LED_CONFIG] = BrickletRS485.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRS485.FUNCTION_GET_COMMUNICATION_LED_CONFIG] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_SET_ERROR_LED_CONFIG] = BrickletRS485.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRS485.FUNCTION_GET_ERROR_LED_CONFIG] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_SET_BUFFER_CONFIG] = BrickletRS485.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRS485.FUNCTION_GET_BUFFER_CONFIG] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_GET_BUFFER_STATUS] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_ENABLE_ERROR_COUNT_CALLBACK] = BrickletRS485.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletRS485.FUNCTION_DISABLE_ERROR_COUNT_CALLBACK] = BrickletRS485.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletRS485.FUNCTION_IS_ERROR_COUNT_CALLBACK_ENABLED] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_GET_ERROR_COUNT] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_GET_MODBUS_COMMON_ERROR_COUNT] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_MODBUS_SLAVE_REPORT_EXCEPTION] = BrickletRS485.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRS485.FUNCTION_MODBUS_SLAVE_ANSWER_READ_COILS_REQUEST_LOW_LEVEL] = BrickletRS485.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletRS485.FUNCTION_MODBUS_MASTER_READ_COILS] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_MODBUS_SLAVE_ANSWER_READ_HOLDING_REGISTERS_REQUEST_LOW_LEVEL] = BrickletRS485.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletRS485.FUNCTION_MODBUS_MASTER_READ_HOLDING_REGISTERS] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_SINGLE_COIL_REQUEST] = BrickletRS485.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRS485.FUNCTION_MODBUS_MASTER_WRITE_SINGLE_COIL] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_SINGLE_REGISTER_REQUEST] = BrickletRS485.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRS485.FUNCTION_MODBUS_MASTER_WRITE_SINGLE_REGISTER] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_MULTIPLE_COILS_REQUEST] = BrickletRS485.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRS485.FUNCTION_MODBUS_MASTER_WRITE_MULTIPLE_COILS_LOW_LEVEL] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_MULTIPLE_REGISTERS_REQUEST] = BrickletRS485.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRS485.FUNCTION_MODBUS_MASTER_WRITE_MULTIPLE_REGISTERS_LOW_LEVEL] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_MODBUS_SLAVE_ANSWER_READ_DISCRETE_INPUTS_REQUEST_LOW_LEVEL] = BrickletRS485.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletRS485.FUNCTION_MODBUS_MASTER_READ_DISCRETE_INPUTS] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_MODBUS_SLAVE_ANSWER_READ_INPUT_REGISTERS_REQUEST_LOW_LEVEL] = BrickletRS485.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletRS485.FUNCTION_MODBUS_MASTER_READ_INPUT_REGISTERS] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_SET_FRAME_READABLE_CALLBACK_CONFIGURATION] = BrickletRS485.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletRS485.FUNCTION_GET_FRAME_READABLE_CALLBACK_CONFIGURATION] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_SET_BOOTLOADER_MODE] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_GET_BOOTLOADER_MODE] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletRS485.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRS485.FUNCTION_WRITE_FIRMWARE] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletRS485.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRS485.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_RESET] = BrickletRS485.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRS485.FUNCTION_WRITE_UID] = BrickletRS485.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRS485.FUNCTION_READ_UID] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRS485.FUNCTION_GET_IDENTITY] = BrickletRS485.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletRS485.CALLBACK_READ_LOW_LEVEL] = (72, 'H H 60c')
        self.callback_formats[BrickletRS485.CALLBACK_ERROR_COUNT] = (16, 'I I')
        self.callback_formats[BrickletRS485.CALLBACK_MODBUS_SLAVE_READ_COILS_REQUEST] = (15, 'B I H')
        self.callback_formats[BrickletRS485.CALLBACK_MODBUS_MASTER_READ_COILS_RESPONSE_LOW_LEVEL] = (72, 'B b H H 464!')
        self.callback_formats[BrickletRS485.CALLBACK_MODBUS_SLAVE_READ_HOLDING_REGISTERS_REQUEST] = (15, 'B I H')
        self.callback_formats[BrickletRS485.CALLBACK_MODBUS_MASTER_READ_HOLDING_REGISTERS_RESPONSE_LOW_LEVEL] = (72, 'B b H H 29H')
        self.callback_formats[BrickletRS485.CALLBACK_MODBUS_SLAVE_WRITE_SINGLE_COIL_REQUEST] = (14, 'B I !')
        self.callback_formats[BrickletRS485.CALLBACK_MODBUS_MASTER_WRITE_SINGLE_COIL_RESPONSE] = (10, 'B b')
        self.callback_formats[BrickletRS485.CALLBACK_MODBUS_SLAVE_WRITE_SINGLE_REGISTER_REQUEST] = (15, 'B I H')
        self.callback_formats[BrickletRS485.CALLBACK_MODBUS_MASTER_WRITE_SINGLE_REGISTER_RESPONSE] = (10, 'B b')
        self.callback_formats[BrickletRS485.CALLBACK_MODBUS_SLAVE_WRITE_MULTIPLE_COILS_REQUEST_LOW_LEVEL] = (72, 'B I H H 440!')
        self.callback_formats[BrickletRS485.CALLBACK_MODBUS_MASTER_WRITE_MULTIPLE_COILS_RESPONSE] = (10, 'B b')
        self.callback_formats[BrickletRS485.CALLBACK_MODBUS_SLAVE_WRITE_MULTIPLE_REGISTERS_REQUEST_LOW_LEVEL] = (71, 'B I H H 27H')
        self.callback_formats[BrickletRS485.CALLBACK_MODBUS_MASTER_WRITE_MULTIPLE_REGISTERS_RESPONSE] = (10, 'B b')
        self.callback_formats[BrickletRS485.CALLBACK_MODBUS_SLAVE_READ_DISCRETE_INPUTS_REQUEST] = (15, 'B I H')
        self.callback_formats[BrickletRS485.CALLBACK_MODBUS_MASTER_READ_DISCRETE_INPUTS_RESPONSE_LOW_LEVEL] = (72, 'B b H H 464!')
        self.callback_formats[BrickletRS485.CALLBACK_MODBUS_SLAVE_READ_INPUT_REGISTERS_REQUEST] = (15, 'B I H')
        self.callback_formats[BrickletRS485.CALLBACK_MODBUS_MASTER_READ_INPUT_REGISTERS_RESPONSE_LOW_LEVEL] = (72, 'B b H H 29H')
        self.callback_formats[BrickletRS485.CALLBACK_FRAME_READABLE] = (10, 'H')

        self.high_level_callbacks[BrickletRS485.CALLBACK_READ] = [('stream_length', 'stream_chunk_offset', 'stream_chunk_data'), {'fixed_length': None, 'single_chunk': False}, None]
        self.high_level_callbacks[BrickletRS485.CALLBACK_MODBUS_MASTER_READ_COILS_RESPONSE] = [(None, None, 'stream_length', 'stream_chunk_offset', 'stream_chunk_data'), {'fixed_length': None, 'single_chunk': False}, None]
        self.high_level_callbacks[BrickletRS485.CALLBACK_MODBUS_MASTER_READ_HOLDING_REGISTERS_RESPONSE] = [(None, None, 'stream_length', 'stream_chunk_offset', 'stream_chunk_data'), {'fixed_length': None, 'single_chunk': False}, None]
        self.high_level_callbacks[BrickletRS485.CALLBACK_MODBUS_SLAVE_WRITE_MULTIPLE_COILS_REQUEST] = [(None, None, 'stream_length', 'stream_chunk_offset', 'stream_chunk_data'), {'fixed_length': None, 'single_chunk': False}, None]
        self.high_level_callbacks[BrickletRS485.CALLBACK_MODBUS_SLAVE_WRITE_MULTIPLE_REGISTERS_REQUEST] = [(None, None, 'stream_length', 'stream_chunk_offset', 'stream_chunk_data'), {'fixed_length': None, 'single_chunk': False}, None]
        self.high_level_callbacks[BrickletRS485.CALLBACK_MODBUS_MASTER_READ_DISCRETE_INPUTS_RESPONSE] = [(None, None, 'stream_length', 'stream_chunk_offset', 'stream_chunk_data'), {'fixed_length': None, 'single_chunk': False}, None]
        self.high_level_callbacks[BrickletRS485.CALLBACK_MODBUS_MASTER_READ_INPUT_REGISTERS_RESPONSE] = [(None, None, 'stream_length', 'stream_chunk_offset', 'stream_chunk_data'), {'fixed_length': None, 'single_chunk': False}, None]
        ipcon.add_device(self)

    def write_low_level(self, message_length, message_chunk_offset, message_chunk_data):
        r"""
        Writes characters to the RS485 interface. The characters can be binary data,
        ASCII or similar is not necessary.

        The return value is the number of characters that were written.

        See :func:`Set RS485 Configuration` for configuration possibilities
        regarding baudrate, parity and so on.
        """
        self.check_validity()

        message_length = int(message_length)
        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = create_char_list(message_chunk_data)

        return self.ipcon.send_request(self, BrickletRS485.FUNCTION_WRITE_LOW_LEVEL, (message_length, message_chunk_offset, message_chunk_data), 'H H 60c', 9, 'B')

    def read_low_level(self, length):
        r"""
        Returns up to *length* characters from receive buffer.

        Instead of polling with this function, you can also use
        callbacks. But note that this function will return available
        data only when the read callback is disabled.
        See :func:`Enable Read Callback` and :cb:`Read` callback.
        """
        self.check_validity()

        length = int(length)

        return ReadLowLevel(*self.ipcon.send_request(self, BrickletRS485.FUNCTION_READ_LOW_LEVEL, (length,), 'H', 72, 'H H 60c'))

    def enable_read_callback(self):
        r"""
        Enables the :cb:`Read` callback. This will disable the :cb:`Frame Readable` callback.

        By default the callback is disabled.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletRS485.FUNCTION_ENABLE_READ_CALLBACK, (), '', 0, '')

    def disable_read_callback(self):
        r"""
        Disables the :cb:`Read` callback.

        By default the callback is disabled.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletRS485.FUNCTION_DISABLE_READ_CALLBACK, (), '', 0, '')

    def is_read_callback_enabled(self):
        r"""
        Returns *true* if the :cb:`Read` callback is enabled,
        *false* otherwise.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRS485.FUNCTION_IS_READ_CALLBACK_ENABLED, (), '', 9, '!')

    def set_rs485_configuration(self, baudrate, parity, stopbits, wordlength, duplex):
        r"""
        Sets the configuration for the RS485 communication.
        """
        self.check_validity()

        baudrate = int(baudrate)
        parity = int(parity)
        stopbits = int(stopbits)
        wordlength = int(wordlength)
        duplex = int(duplex)

        self.ipcon.send_request(self, BrickletRS485.FUNCTION_SET_RS485_CONFIGURATION, (baudrate, parity, stopbits, wordlength, duplex), 'I B B B B', 0, '')

    def get_rs485_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set RS485 Configuration`.
        """
        self.check_validity()

        return GetRS485Configuration(*self.ipcon.send_request(self, BrickletRS485.FUNCTION_GET_RS485_CONFIGURATION, (), '', 16, 'I B B B B'))

    def set_modbus_configuration(self, slave_address, master_request_timeout):
        r"""
        Sets the configuration for the RS485 Modbus communication. Available options:

        * Slave Address: Address to be used as the Modbus slave address in Modbus slave mode. Valid Modbus slave address range is 1 to 247.
        * Master Request Timeout: Specifies how long the master should wait for a response from a slave when in Modbus master mode.
        """
        self.check_validity()

        slave_address = int(slave_address)
        master_request_timeout = int(master_request_timeout)

        self.ipcon.send_request(self, BrickletRS485.FUNCTION_SET_MODBUS_CONFIGURATION, (slave_address, master_request_timeout), 'B I', 0, '')

    def get_modbus_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Modbus Configuration`.
        """
        self.check_validity()

        return GetModbusConfiguration(*self.ipcon.send_request(self, BrickletRS485.FUNCTION_GET_MODBUS_CONFIGURATION, (), '', 13, 'B I'))

    def set_mode(self, mode):
        r"""
        Sets the mode of the Bricklet in which it operates. Available options are

        * RS485,
        * Modbus Master RTU and
        * Modbus Slave RTU.
        """
        self.check_validity()

        mode = int(mode)

        self.ipcon.send_request(self, BrickletRS485.FUNCTION_SET_MODE, (mode,), 'B', 0, '')

    def get_mode(self):
        r"""
        Returns the configuration as set by :func:`Set Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRS485.FUNCTION_GET_MODE, (), '', 9, 'B')

    def set_communication_led_config(self, config):
        r"""
        Sets the communication LED configuration. By default the LED shows RS485
        communication traffic by flickering.

        You can also turn the LED permanently on/off or show a heartbeat.

        If the Bricklet is in bootloader mode, the LED is off.
        """
        self.check_validity()

        config = int(config)

        self.ipcon.send_request(self, BrickletRS485.FUNCTION_SET_COMMUNICATION_LED_CONFIG, (config,), 'B', 0, '')

    def get_communication_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Communication LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRS485.FUNCTION_GET_COMMUNICATION_LED_CONFIG, (), '', 9, 'B')

    def set_error_led_config(self, config):
        r"""
        Sets the error LED configuration.

        By default the error LED turns on if there is any error (see :cb:`Error Count`
        callback). If you call this function with the SHOW ERROR option again, the LED
        will turn off until the next error occurs.

        You can also turn the LED permanently on/off or show a heartbeat.

        If the Bricklet is in bootloader mode, the LED is off.
        """
        self.check_validity()

        config = int(config)

        self.ipcon.send_request(self, BrickletRS485.FUNCTION_SET_ERROR_LED_CONFIG, (config,), 'B', 0, '')

    def get_error_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Error LED Config`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRS485.FUNCTION_GET_ERROR_LED_CONFIG, (), '', 9, 'B')

    def set_buffer_config(self, send_buffer_size, receive_buffer_size):
        r"""
        Sets the send and receive buffer size in byte. In sum there is
        10240 byte (10KiB) buffer available and the minimum buffer size
        is 1024 byte (1KiB) for both.

        The current buffer content is lost if this function is called.

        The send buffer holds data that was given by :func:`Write` and
        could not be written yet. The receive buffer holds data that is
        received through RS485 but could not yet be send to the
        user, either by :func:`Read` or through :cb:`Read` callback.
        """
        self.check_validity()

        send_buffer_size = int(send_buffer_size)
        receive_buffer_size = int(receive_buffer_size)

        self.ipcon.send_request(self, BrickletRS485.FUNCTION_SET_BUFFER_CONFIG, (send_buffer_size, receive_buffer_size), 'H H', 0, '')

    def get_buffer_config(self):
        r"""
        Returns the buffer configuration as set by :func:`Set Buffer Config`.
        """
        self.check_validity()

        return GetBufferConfig(*self.ipcon.send_request(self, BrickletRS485.FUNCTION_GET_BUFFER_CONFIG, (), '', 12, 'H H'))

    def get_buffer_status(self):
        r"""
        Returns the currently used bytes for the send and received buffer.

        See :func:`Set Buffer Config` for buffer size configuration.
        """
        self.check_validity()

        return GetBufferStatus(*self.ipcon.send_request(self, BrickletRS485.FUNCTION_GET_BUFFER_STATUS, (), '', 12, 'H H'))

    def enable_error_count_callback(self):
        r"""
        Enables the :cb:`Error Count` callback.

        By default the callback is disabled.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletRS485.FUNCTION_ENABLE_ERROR_COUNT_CALLBACK, (), '', 0, '')

    def disable_error_count_callback(self):
        r"""
        Disables the :cb:`Error Count` callback.

        By default the callback is disabled.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletRS485.FUNCTION_DISABLE_ERROR_COUNT_CALLBACK, (), '', 0, '')

    def is_error_count_callback_enabled(self):
        r"""
        Returns *true* if the :cb:`Error Count` callback is enabled,
        *false* otherwise.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRS485.FUNCTION_IS_ERROR_COUNT_CALLBACK_ENABLED, (), '', 9, '!')

    def get_error_count(self):
        r"""
        Returns the current number of overrun and parity errors.
        """
        self.check_validity()

        return GetErrorCount(*self.ipcon.send_request(self, BrickletRS485.FUNCTION_GET_ERROR_COUNT, (), '', 16, 'I I'))

    def get_modbus_common_error_count(self):
        r"""
        Returns the current number of errors occurred in Modbus mode.

        * Timeout Error Count: Number of timeouts occurred.
        * Checksum Error Count: Number of failures due to Modbus frame CRC16 checksum mismatch.
        * Frame Too Big Error Count: Number of times frames were rejected because they exceeded maximum Modbus frame size which is 256 bytes.
        * Illegal Function Error Count: Number of errors when an unimplemented or illegal function is requested. This corresponds to Modbus exception code 1.
        * Illegal Data Address Error Count: Number of errors due to invalid data address. This corresponds to Modbus exception code 2.
        * Illegal Data Value Error Count: Number of errors due to invalid data value. This corresponds to Modbus exception code 3.
        * Slave Device Failure Error Count: Number of errors occurred on the slave device which were unrecoverable. This corresponds to Modbus exception code 4.
        """
        self.check_validity()

        return GetModbusCommonErrorCount(*self.ipcon.send_request(self, BrickletRS485.FUNCTION_GET_MODBUS_COMMON_ERROR_COUNT, (), '', 36, 'I I I I I I I'))

    def modbus_slave_report_exception(self, request_id, exception_code):
        r"""
        In Modbus slave mode this function can be used to report a Modbus exception for
        a Modbus master request.

        * Request ID: Request ID of the request received by the slave.
        * Exception Code: Modbus exception code to report to the Modbus master.
        """
        self.check_validity()

        request_id = int(request_id)
        exception_code = int(exception_code)

        self.ipcon.send_request(self, BrickletRS485.FUNCTION_MODBUS_SLAVE_REPORT_EXCEPTION, (request_id, exception_code), 'B b', 0, '')

    def modbus_slave_answer_read_coils_request_low_level(self, request_id, coils_length, coils_chunk_offset, coils_chunk_data):
        r"""
        In Modbus slave mode this function can be used to answer a master request to
        read coils.

        * Request ID: Request ID of the corresponding request that is being answered.
        * Coils: Data that is to be sent to the Modbus master for the corresponding request.

        This function must be called from the :cb:`Modbus Slave Read Coils Request` callback
        with the Request ID as provided by the argument of the callback.
        """
        self.check_validity()

        request_id = int(request_id)
        coils_length = int(coils_length)
        coils_chunk_offset = int(coils_chunk_offset)
        coils_chunk_data = list(map(bool, coils_chunk_data))

        self.ipcon.send_request(self, BrickletRS485.FUNCTION_MODBUS_SLAVE_ANSWER_READ_COILS_REQUEST_LOW_LEVEL, (request_id, coils_length, coils_chunk_offset, coils_chunk_data), 'B H H 472!', 0, '')

    def modbus_master_read_coils(self, slave_address, starting_address, count):
        r"""
        In Modbus master mode this function can be used to read coils from a slave. This
        function creates a Modbus function code 1 request.

        * Slave Address: Address of the target Modbus slave.
        * Starting Address: Number of the first coil to read. For backwards compatibility reasons this parameter is called Starting Address. It is not an address, but instead a coil number in the range of 1 to 65536.
        * Count: Number of coils to read.

        Upon success the function will return a non-zero request ID which will represent
        the current request initiated by the Modbus master. In case of failure the returned
        request ID will be 0.

        When successful this function will also invoke the :cb:`Modbus Master Read Coils Response`
        callback. In this callback the Request ID provided by the callback argument must be
        matched with the Request ID returned from this function to verify that the callback
        is indeed for a particular request.
        """
        self.check_validity()

        slave_address = int(slave_address)
        starting_address = int(starting_address)
        count = int(count)

        return self.ipcon.send_request(self, BrickletRS485.FUNCTION_MODBUS_MASTER_READ_COILS, (slave_address, starting_address, count), 'B I H', 9, 'B')

    def modbus_slave_answer_read_holding_registers_request_low_level(self, request_id, holding_registers_length, holding_registers_chunk_offset, holding_registers_chunk_data):
        r"""
        In Modbus slave mode this function can be used to answer a master request to
        read holding registers.

        * Request ID: Request ID of the corresponding request that is being answered.
        * Holding Registers: Data that is to be sent to the Modbus master for the corresponding request.

        This function must be called from the :cb:`Modbus Slave Read Holding Registers Request`
        callback with the Request ID as provided by the argument of the callback.
        """
        self.check_validity()

        request_id = int(request_id)
        holding_registers_length = int(holding_registers_length)
        holding_registers_chunk_offset = int(holding_registers_chunk_offset)
        holding_registers_chunk_data = list(map(int, holding_registers_chunk_data))

        self.ipcon.send_request(self, BrickletRS485.FUNCTION_MODBUS_SLAVE_ANSWER_READ_HOLDING_REGISTERS_REQUEST_LOW_LEVEL, (request_id, holding_registers_length, holding_registers_chunk_offset, holding_registers_chunk_data), 'B H H 29H', 0, '')

    def modbus_master_read_holding_registers(self, slave_address, starting_address, count):
        r"""
        In Modbus master mode this function can be used to read holding registers from a slave.
        This function creates a Modbus function code 3 request.

        * Slave Address: Address of the target Modbus slave.
        * Starting Address: Number of the first holding register to read. For backwards compatibility reasons this parameter is called Starting Address. It is not an address, but instead a holding register number in the range of 1 to 65536. The prefix digit 4 (for holding register) is implicit and must be omitted.
        * Count: Number of holding registers to read.

        Upon success the function will return a non-zero request ID which will represent
        the current request initiated by the Modbus master. In case of failure the returned
        request ID will be 0.

        When successful this function will also invoke the :cb:`Modbus Master Read Holding Registers Response`
        callback. In this callback the Request ID provided by the callback argument must be matched
        with the Request ID returned from this function to verify that the callback is indeed for a
        particular request.
        """
        self.check_validity()

        slave_address = int(slave_address)
        starting_address = int(starting_address)
        count = int(count)

        return self.ipcon.send_request(self, BrickletRS485.FUNCTION_MODBUS_MASTER_READ_HOLDING_REGISTERS, (slave_address, starting_address, count), 'B I H', 9, 'B')

    def modbus_slave_answer_write_single_coil_request(self, request_id):
        r"""
        In Modbus slave mode this function can be used to answer a master request to
        write a single coil.

        * Request ID: Request ID of the corresponding request that is being answered.

        This function must be called from the :cb:`Modbus Slave Write Single Coil Request`
        callback with the Request ID as provided by the arguments of the callback.
        """
        self.check_validity()

        request_id = int(request_id)

        self.ipcon.send_request(self, BrickletRS485.FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_SINGLE_COIL_REQUEST, (request_id,), 'B', 0, '')

    def modbus_master_write_single_coil(self, slave_address, coil_address, coil_value):
        r"""
        In Modbus master mode this function can be used to write a single coil of a slave.
        This function creates a Modbus function code 5 request.

        * Slave Address: Address of the target Modbus slave.
        * Coil Address: Number of the coil to be written. For backwards compatibility reasons, this parameter is called Starting Address. It is not an address, but instead a coil number in the range of 1 to 65536.
        * Coil Value: Value to be written.

        Upon success the function will return a non-zero request ID which will represent
        the current request initiated by the Modbus master. In case of failure the returned
        request ID will be 0.

        When successful this function will also invoke the :cb:`Modbus Master Write Single Coil Response`
        callback. In this callback the Request ID provided by the callback argument must be matched
        with the Request ID returned from this function to verify that the callback is indeed for a
        particular request.
        """
        self.check_validity()

        slave_address = int(slave_address)
        coil_address = int(coil_address)
        coil_value = bool(coil_value)

        return self.ipcon.send_request(self, BrickletRS485.FUNCTION_MODBUS_MASTER_WRITE_SINGLE_COIL, (slave_address, coil_address, coil_value), 'B I !', 9, 'B')

    def modbus_slave_answer_write_single_register_request(self, request_id):
        r"""
        In Modbus slave mode this function can be used to answer a master request to
        write a single register.

        * Request ID: Request ID of the corresponding request that is being answered.

        This function must be called from the :cb:`Modbus Slave Write Single Register Request`
        callback with the Request ID, Register Address and Register Value as provided by
        the arguments of the callback.
        """
        self.check_validity()

        request_id = int(request_id)

        self.ipcon.send_request(self, BrickletRS485.FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_SINGLE_REGISTER_REQUEST, (request_id,), 'B', 0, '')

    def modbus_master_write_single_register(self, slave_address, register_address, register_value):
        r"""
        In Modbus master mode this function can be used to write a single holding register of a
        slave. This function creates a Modbus function code 6 request.

        * Slave Address: Address of the target Modbus slave.
        * Register Address: Number of the holding register to be written. For backwards compatibility reasons, this parameter is called Starting Address. It is not an address, but instead a holding register number in the range of 1 to 65536. The prefix digit 4 (for holding register) is implicit and must be omitted.
        * Register Value: Value to be written.

        Upon success the function will return a non-zero request ID which will represent
        the current request initiated by the Modbus master. In case of failure the returned
        request ID will be 0.

        When successful this function will also invoke the :cb:`Modbus Master Write Single Register Response`
        callback. In this callback the Request ID provided by the callback argument must be matched
        with the Request ID returned from this function to verify that the callback is indeed for a
        particular request.
        """
        self.check_validity()

        slave_address = int(slave_address)
        register_address = int(register_address)
        register_value = int(register_value)

        return self.ipcon.send_request(self, BrickletRS485.FUNCTION_MODBUS_MASTER_WRITE_SINGLE_REGISTER, (slave_address, register_address, register_value), 'B I H', 9, 'B')

    def modbus_slave_answer_write_multiple_coils_request(self, request_id):
        r"""
        In Modbus slave mode this function can be used to answer a master request to
        write multiple coils.

        * Request ID: Request ID of the corresponding request that is being answered.

        This function must be called from the :cb:`Modbus Slave Write Multiple Coils Request`
        callback with the Request ID of the callback.
        """
        self.check_validity()

        request_id = int(request_id)

        self.ipcon.send_request(self, BrickletRS485.FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_MULTIPLE_COILS_REQUEST, (request_id,), 'B', 0, '')

    def modbus_master_write_multiple_coils_low_level(self, slave_address, starting_address, coils_length, coils_chunk_offset, coils_chunk_data):
        r"""
        In Modbus master mode this function can be used to write multiple coils of a slave.
        This function creates a Modbus function code 15 request.

        * Slave Address: Address of the target Modbus slave.
        * Starting Address: Number of the first coil to write. For backwards compatibility reasons, this parameter is called Starting Address.It is not an address, but instead a coil number in the range of 1 to 65536.

        Upon success the function will return a non-zero request ID which will represent
        the current request initiated by the Modbus master. In case of failure the returned
        request ID will be 0.

        When successful this function will also invoke the :cb:`Modbus Master Write Multiple Coils Response`
        callback. In this callback the Request ID provided by the callback argument must be matched
        with the Request ID returned from this function to verify that the callback is indeed for a
        particular request.
        """
        self.check_validity()

        slave_address = int(slave_address)
        starting_address = int(starting_address)
        coils_length = int(coils_length)
        coils_chunk_offset = int(coils_chunk_offset)
        coils_chunk_data = list(map(bool, coils_chunk_data))

        return self.ipcon.send_request(self, BrickletRS485.FUNCTION_MODBUS_MASTER_WRITE_MULTIPLE_COILS_LOW_LEVEL, (slave_address, starting_address, coils_length, coils_chunk_offset, coils_chunk_data), 'B I H H 440!', 9, 'B')

    def modbus_slave_answer_write_multiple_registers_request(self, request_id):
        r"""
        In Modbus slave mode this function can be used to answer a master request to
        write multiple registers.

        * Request ID: Request ID of the corresponding request that is being answered.

        This function must be called from the :cb:`Modbus Slave Write Multiple Registers Request`
        callback with the Request ID of the callback.
        """
        self.check_validity()

        request_id = int(request_id)

        self.ipcon.send_request(self, BrickletRS485.FUNCTION_MODBUS_SLAVE_ANSWER_WRITE_MULTIPLE_REGISTERS_REQUEST, (request_id,), 'B', 0, '')

    def modbus_master_write_multiple_registers_low_level(self, slave_address, starting_address, registers_length, registers_chunk_offset, registers_chunk_data):
        r"""
        In Modbus master mode this function can be used to write multiple registers of a slave.
        This function creates a Modbus function code 16 request.

        * Slave Address: Address of the target Modbus slave.
        * Starting Address: Number of the first holding register to write. For backwards compatibility reasons, this parameter is called Starting Address. It is not an address, but instead a holding register number in the range of 1 to 65536. The prefix digit 4 (for holding register) is implicit and must be omitted.

        Upon success the function will return a non-zero request ID which will represent
        the current request initiated by the Modbus master. In case of failure the returned
        request ID will be 0.

        When successful this function will also invoke the :cb:`Modbus Master Write Multiple Registers Response`
        callback. In this callback the Request ID provided by the callback argument must be matched
        with the Request ID returned from this function to verify that the callback is indeed for a
        particular request.
        """
        self.check_validity()

        slave_address = int(slave_address)
        starting_address = int(starting_address)
        registers_length = int(registers_length)
        registers_chunk_offset = int(registers_chunk_offset)
        registers_chunk_data = list(map(int, registers_chunk_data))

        return self.ipcon.send_request(self, BrickletRS485.FUNCTION_MODBUS_MASTER_WRITE_MULTIPLE_REGISTERS_LOW_LEVEL, (slave_address, starting_address, registers_length, registers_chunk_offset, registers_chunk_data), 'B I H H 27H', 9, 'B')

    def modbus_slave_answer_read_discrete_inputs_request_low_level(self, request_id, discrete_inputs_length, discrete_inputs_chunk_offset, discrete_inputs_chunk_data):
        r"""
        In Modbus slave mode this function can be used to answer a master request to
        read discrete inputs.

        * Request ID: Request ID of the corresponding request that is being answered.
        * Discrete Inputs: Data that is to be sent to the Modbus master for the corresponding request.

        This function must be called from the :cb:`Modbus Slave Read Discrete Inputs Request`
        callback with the Request ID as provided by the argument of the callback.
        """
        self.check_validity()

        request_id = int(request_id)
        discrete_inputs_length = int(discrete_inputs_length)
        discrete_inputs_chunk_offset = int(discrete_inputs_chunk_offset)
        discrete_inputs_chunk_data = list(map(bool, discrete_inputs_chunk_data))

        self.ipcon.send_request(self, BrickletRS485.FUNCTION_MODBUS_SLAVE_ANSWER_READ_DISCRETE_INPUTS_REQUEST_LOW_LEVEL, (request_id, discrete_inputs_length, discrete_inputs_chunk_offset, discrete_inputs_chunk_data), 'B H H 472!', 0, '')

    def modbus_master_read_discrete_inputs(self, slave_address, starting_address, count):
        r"""
        In Modbus master mode this function can be used to read discrete inputs from a slave.
        This function creates a Modbus function code 2 request.

        * Slave Address: Address of the target Modbus slave.
        * Starting Address: Number of the first discrete input to read. For backwards compatibility reasons, this parameter is called Starting Address. It is not an address, but instead a discrete input number in the range of 1 to 65536. The prefix digit 1 (for discrete input) is implicit and must be omitted.
        * Count: Number of discrete inputs to read.

        Upon success the function will return a non-zero request ID which will represent
        the current request initiated by the Modbus master. In case of failure the returned
        request ID will be 0.

        When successful this function will also invoke the :cb:`Modbus Master Read Discrete Inputs Response`
        callback. In this callback the Request ID provided by the callback argument must be matched
        with the Request ID returned from this function to verify that the callback is indeed for a
        particular request.
        """
        self.check_validity()

        slave_address = int(slave_address)
        starting_address = int(starting_address)
        count = int(count)

        return self.ipcon.send_request(self, BrickletRS485.FUNCTION_MODBUS_MASTER_READ_DISCRETE_INPUTS, (slave_address, starting_address, count), 'B I H', 9, 'B')

    def modbus_slave_answer_read_input_registers_request_low_level(self, request_id, input_registers_length, input_registers_chunk_offset, input_registers_chunk_data):
        r"""
        In Modbus slave mode this function can be used to answer a master request to
        read input registers.

        * Request ID: Request ID of the corresponding request that is being answered.
        * Input Registers: Data that is to be sent to the Modbus master for the corresponding request.

        This function must be called from the :cb:`Modbus Slave Read Input Registers Request` callback
        with the Request ID as provided by the argument of the callback.
        """
        self.check_validity()

        request_id = int(request_id)
        input_registers_length = int(input_registers_length)
        input_registers_chunk_offset = int(input_registers_chunk_offset)
        input_registers_chunk_data = list(map(int, input_registers_chunk_data))

        self.ipcon.send_request(self, BrickletRS485.FUNCTION_MODBUS_SLAVE_ANSWER_READ_INPUT_REGISTERS_REQUEST_LOW_LEVEL, (request_id, input_registers_length, input_registers_chunk_offset, input_registers_chunk_data), 'B H H 29H', 0, '')

    def modbus_master_read_input_registers(self, slave_address, starting_address, count):
        r"""
        In Modbus master mode this function can be used to read input registers from a slave.
        This function creates a Modbus function code 4 request.

        * Slave Address: Address of the target Modbus slave.
        * Starting Address: Number of the first input register to read. For backwards compatibility reasons, this parameter is called Starting Address. It is not an address, but instead an input register number in the range of 1 to 65536. The prefix digit 3 (for input register) is implicit and must be omitted.
        * Count: Number of input registers to read.

        Upon success the function will return a non-zero request ID which will represent
        the current request initiated by the Modbus master. In case of failure the returned
        request ID will be 0.

        When successful this function will also invoke the :cb:`Modbus Master Read Input Registers Response`
        callback. In this callback the Request ID provided by the callback argument must be matched
        with the Request ID returned from this function to verify that the callback is indeed for a
        particular request.
        """
        self.check_validity()

        slave_address = int(slave_address)
        starting_address = int(starting_address)
        count = int(count)

        return self.ipcon.send_request(self, BrickletRS485.FUNCTION_MODBUS_MASTER_READ_INPUT_REGISTERS, (slave_address, starting_address, count), 'B I H', 9, 'B')

    def set_frame_readable_callback_configuration(self, frame_size):
        r"""
        Configures the :cb:`Frame Readable` callback. The frame size is the number of bytes, that have to be readable to trigger the callback.
        A frame size of 0 disables the callback. A frame size greater than 0 enables the callback and disables the :cb:`Read` callback.

        By default the callback is disabled.

        .. versionadded:: 2.0.5$nbsp;(Plugin)
        """
        self.check_validity()

        frame_size = int(frame_size)

        self.ipcon.send_request(self, BrickletRS485.FUNCTION_SET_FRAME_READABLE_CALLBACK_CONFIGURATION, (frame_size,), 'H', 0, '')

    def get_frame_readable_callback_configuration(self):
        r"""
        Returns the callback configuration as set by :func:`Set Frame Readable Callback Configuration`.

        .. versionadded:: 2.0.5$nbsp;(Plugin)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRS485.FUNCTION_GET_FRAME_READABLE_CALLBACK_CONFIGURATION, (), '', 10, 'H')

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletRS485.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletRS485.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRS485.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletRS485.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletRS485.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletRS485.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRS485.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRS485.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletRS485.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletRS485.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRS485.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletRS485.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def write(self, message):
        r"""
        Writes characters to the RS485 interface. The characters can be binary data,
        ASCII or similar is not necessary.

        The return value is the number of characters that were written.

        See :func:`Set RS485 Configuration` for configuration possibilities
        regarding baudrate, parity and so on.
        """
        message = create_char_list(message)

        if len(message) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 65535 items long')

        message_length = len(message)
        message_chunk_offset = 0

        if message_length == 0:
            message_chunk_data = ['\0'] * 60
            ret = self.write_low_level(message_length, message_chunk_offset, message_chunk_data)
            message_written = ret
        else:
            message_written = 0

            with self.stream_lock:
                while message_chunk_offset < message_length:
                    message_chunk_data = create_chunk_data(message, message_chunk_offset, 60, '\0')
                    ret = self.write_low_level(message_length, message_chunk_offset, message_chunk_data)
                    message_written += ret

                    if ret < 60:
                        break # either last chunk or short write

                    message_chunk_offset += 60

        return message_written

    def read(self, length):
        r"""
        Returns up to *length* characters from receive buffer.

        Instead of polling with this function, you can also use
        callbacks. But note that this function will return available
        data only when the read callback is disabled.
        See :func:`Enable Read Callback` and :cb:`Read` callback.
        """
        length = int(length)

        with self.stream_lock:
            ret = self.read_low_level(length)
            message_length = ret.message_length
            message_out_of_sync = ret.message_chunk_offset != 0
            message_data = ret.message_chunk_data

            while not message_out_of_sync and len(message_data) < message_length:
                ret = self.read_low_level(length)
                message_length = ret.message_length
                message_out_of_sync = ret.message_chunk_offset != len(message_data)
                message_data += ret.message_chunk_data

            if message_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.message_chunk_offset + 60 < message_length:
                    ret = self.read_low_level(length)
                    message_length = ret.message_length

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Message stream is out-of-sync')

        return message_data[:message_length]

    def modbus_slave_answer_read_coils_request(self, request_id, coils):
        r"""
        In Modbus slave mode this function can be used to answer a master request to
        read coils.

        * Request ID: Request ID of the corresponding request that is being answered.
        * Coils: Data that is to be sent to the Modbus master for the corresponding request.

        This function must be called from the :cb:`Modbus Slave Read Coils Request` callback
        with the Request ID as provided by the argument of the callback.
        """
        request_id = int(request_id)
        coils = list(map(bool, coils))

        if len(coils) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Coils can be at most 65535 items long')

        coils_length = len(coils)
        coils_chunk_offset = 0

        if coils_length == 0:
            coils_chunk_data = [False] * 472
            ret = self.modbus_slave_answer_read_coils_request_low_level(request_id, coils_length, coils_chunk_offset, coils_chunk_data)
        else:
            with self.stream_lock:
                while coils_chunk_offset < coils_length:
                    coils_chunk_data = create_chunk_data(coils, coils_chunk_offset, 472, False)
                    ret = self.modbus_slave_answer_read_coils_request_low_level(request_id, coils_length, coils_chunk_offset, coils_chunk_data)
                    coils_chunk_offset += 472

        return ret

    def modbus_slave_answer_read_holding_registers_request(self, request_id, holding_registers):
        r"""
        In Modbus slave mode this function can be used to answer a master request to
        read holding registers.

        * Request ID: Request ID of the corresponding request that is being answered.
        * Holding Registers: Data that is to be sent to the Modbus master for the corresponding request.

        This function must be called from the :cb:`Modbus Slave Read Holding Registers Request`
        callback with the Request ID as provided by the argument of the callback.
        """
        request_id = int(request_id)
        holding_registers = list(map(int, holding_registers))

        if len(holding_registers) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Holding Registers can be at most 65535 items long')

        holding_registers_length = len(holding_registers)
        holding_registers_chunk_offset = 0

        if holding_registers_length == 0:
            holding_registers_chunk_data = [0] * 29
            ret = self.modbus_slave_answer_read_holding_registers_request_low_level(request_id, holding_registers_length, holding_registers_chunk_offset, holding_registers_chunk_data)
        else:
            with self.stream_lock:
                while holding_registers_chunk_offset < holding_registers_length:
                    holding_registers_chunk_data = create_chunk_data(holding_registers, holding_registers_chunk_offset, 29, 0)
                    ret = self.modbus_slave_answer_read_holding_registers_request_low_level(request_id, holding_registers_length, holding_registers_chunk_offset, holding_registers_chunk_data)
                    holding_registers_chunk_offset += 29

        return ret

    def modbus_master_write_multiple_coils(self, slave_address, starting_address, coils):
        r"""
        In Modbus master mode this function can be used to write multiple coils of a slave.
        This function creates a Modbus function code 15 request.

        * Slave Address: Address of the target Modbus slave.
        * Starting Address: Number of the first coil to write. For backwards compatibility reasons, this parameter is called Starting Address.It is not an address, but instead a coil number in the range of 1 to 65536.

        Upon success the function will return a non-zero request ID which will represent
        the current request initiated by the Modbus master. In case of failure the returned
        request ID will be 0.

        When successful this function will also invoke the :cb:`Modbus Master Write Multiple Coils Response`
        callback. In this callback the Request ID provided by the callback argument must be matched
        with the Request ID returned from this function to verify that the callback is indeed for a
        particular request.
        """
        slave_address = int(slave_address)
        starting_address = int(starting_address)
        coils = list(map(bool, coils))

        if len(coils) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Coils can be at most 65535 items long')

        coils_length = len(coils)
        coils_chunk_offset = 0

        if coils_length == 0:
            coils_chunk_data = [False] * 440
            ret = self.modbus_master_write_multiple_coils_low_level(slave_address, starting_address, coils_length, coils_chunk_offset, coils_chunk_data)
        else:
            with self.stream_lock:
                while coils_chunk_offset < coils_length:
                    coils_chunk_data = create_chunk_data(coils, coils_chunk_offset, 440, False)
                    ret = self.modbus_master_write_multiple_coils_low_level(slave_address, starting_address, coils_length, coils_chunk_offset, coils_chunk_data)
                    coils_chunk_offset += 440

        return ret

    def modbus_master_write_multiple_registers(self, slave_address, starting_address, registers):
        r"""
        In Modbus master mode this function can be used to write multiple registers of a slave.
        This function creates a Modbus function code 16 request.

        * Slave Address: Address of the target Modbus slave.
        * Starting Address: Number of the first holding register to write. For backwards compatibility reasons, this parameter is called Starting Address. It is not an address, but instead a holding register number in the range of 1 to 65536. The prefix digit 4 (for holding register) is implicit and must be omitted.

        Upon success the function will return a non-zero request ID which will represent
        the current request initiated by the Modbus master. In case of failure the returned
        request ID will be 0.

        When successful this function will also invoke the :cb:`Modbus Master Write Multiple Registers Response`
        callback. In this callback the Request ID provided by the callback argument must be matched
        with the Request ID returned from this function to verify that the callback is indeed for a
        particular request.
        """
        slave_address = int(slave_address)
        starting_address = int(starting_address)
        registers = list(map(int, registers))

        if len(registers) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Registers can be at most 65535 items long')

        registers_length = len(registers)
        registers_chunk_offset = 0

        if registers_length == 0:
            registers_chunk_data = [0] * 27
            ret = self.modbus_master_write_multiple_registers_low_level(slave_address, starting_address, registers_length, registers_chunk_offset, registers_chunk_data)
        else:
            with self.stream_lock:
                while registers_chunk_offset < registers_length:
                    registers_chunk_data = create_chunk_data(registers, registers_chunk_offset, 27, 0)
                    ret = self.modbus_master_write_multiple_registers_low_level(slave_address, starting_address, registers_length, registers_chunk_offset, registers_chunk_data)
                    registers_chunk_offset += 27

        return ret

    def modbus_slave_answer_read_discrete_inputs_request(self, request_id, discrete_inputs):
        r"""
        In Modbus slave mode this function can be used to answer a master request to
        read discrete inputs.

        * Request ID: Request ID of the corresponding request that is being answered.
        * Discrete Inputs: Data that is to be sent to the Modbus master for the corresponding request.

        This function must be called from the :cb:`Modbus Slave Read Discrete Inputs Request`
        callback with the Request ID as provided by the argument of the callback.
        """
        request_id = int(request_id)
        discrete_inputs = list(map(bool, discrete_inputs))

        if len(discrete_inputs) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Discrete Inputs can be at most 65535 items long')

        discrete_inputs_length = len(discrete_inputs)
        discrete_inputs_chunk_offset = 0

        if discrete_inputs_length == 0:
            discrete_inputs_chunk_data = [False] * 472
            ret = self.modbus_slave_answer_read_discrete_inputs_request_low_level(request_id, discrete_inputs_length, discrete_inputs_chunk_offset, discrete_inputs_chunk_data)
        else:
            with self.stream_lock:
                while discrete_inputs_chunk_offset < discrete_inputs_length:
                    discrete_inputs_chunk_data = create_chunk_data(discrete_inputs, discrete_inputs_chunk_offset, 472, False)
                    ret = self.modbus_slave_answer_read_discrete_inputs_request_low_level(request_id, discrete_inputs_length, discrete_inputs_chunk_offset, discrete_inputs_chunk_data)
                    discrete_inputs_chunk_offset += 472

        return ret

    def modbus_slave_answer_read_input_registers_request(self, request_id, input_registers):
        r"""
        In Modbus slave mode this function can be used to answer a master request to
        read input registers.

        * Request ID: Request ID of the corresponding request that is being answered.
        * Input Registers: Data that is to be sent to the Modbus master for the corresponding request.

        This function must be called from the :cb:`Modbus Slave Read Input Registers Request` callback
        with the Request ID as provided by the argument of the callback.
        """
        request_id = int(request_id)
        input_registers = list(map(int, input_registers))

        if len(input_registers) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Input Registers can be at most 65535 items long')

        input_registers_length = len(input_registers)
        input_registers_chunk_offset = 0

        if input_registers_length == 0:
            input_registers_chunk_data = [0] * 29
            ret = self.modbus_slave_answer_read_input_registers_request_low_level(request_id, input_registers_length, input_registers_chunk_offset, input_registers_chunk_data)
        else:
            with self.stream_lock:
                while input_registers_chunk_offset < input_registers_length:
                    input_registers_chunk_data = create_chunk_data(input_registers, input_registers_chunk_offset, 29, 0)
                    ret = self.modbus_slave_answer_read_input_registers_request_low_level(request_id, input_registers_length, input_registers_chunk_offset, input_registers_chunk_data)
                    input_registers_chunk_offset += 29

        return ret

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

RS485 = BrickletRS485 # for backward compatibility
