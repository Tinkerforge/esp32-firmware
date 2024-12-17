# -*- coding: utf-8 -*-
#############################################################
# This file was automatically generated on 2024-12-17.      #
#                                                           #
# Python Bindings Version 2.1.31                            #
#                                                           #
# If you have a bugfix for this file and want to commit it, #
# please fix the bug in the generator. You can find a link  #
# to the generators git repository on tinkerforge.com       #
#############################################################

#### __DEVICE_IS_NOT_RELEASED__ ####

from collections import namedtuple

try:
    from .ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data
except (ValueError, ImportError):
    try:
        from ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data
    except (ValueError, ImportError):
        from tinkerforge.ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data

NormalWriteExtraOut2LowLevel = namedtuple('NormalWriteExtraOut2LowLevel', ['extra_1', 'extra_2'])
FixedWriteExtraOut2LowLevel = namedtuple('FixedWriteExtraOut2LowLevel', ['extra_1', 'extra_2'])
ShortWriteExtraOutPrefix1LowLevel = namedtuple('ShortWriteExtraOutPrefix1LowLevel', ['extra', 'message_chunk_written'])
ShortWriteExtraOutPrefix2LowLevel = namedtuple('ShortWriteExtraOutPrefix2LowLevel', ['extra_1', 'extra_2', 'message_chunk_written'])
ShortWriteExtraOutSuffix1LowLevel = namedtuple('ShortWriteExtraOutSuffix1LowLevel', ['message_chunk_written', 'extra'])
ShortWriteExtraOutSuffix2LowLevel = namedtuple('ShortWriteExtraOutSuffix2LowLevel', ['message_chunk_written', 'extra_1', 'extra_2'])
ShortWriteExtraFullLowLevel = namedtuple('ShortWriteExtraFullLowLevel', ['extra_5', 'message_chunk_written', 'extra_6'])
SingleWriteExtraOut2LowLevel = namedtuple('SingleWriteExtraOut2LowLevel', ['extra_1', 'extra_2'])
ShortSingleWriteExtraOutPrefix1LowLevel = namedtuple('ShortSingleWriteExtraOutPrefix1LowLevel', ['extra', 'message_written'])
ShortSingleWriteExtraOutPrefix2LowLevel = namedtuple('ShortSingleWriteExtraOutPrefix2LowLevel', ['extra_1', 'extra_2', 'message_written'])
ShortSingleWriteExtraOutSuffix1LowLevel = namedtuple('ShortSingleWriteExtraOutSuffix1LowLevel', ['message_written', 'extra'])
ShortSingleWriteExtraOutSuffix2LowLevel = namedtuple('ShortSingleWriteExtraOutSuffix2LowLevel', ['message_written', 'extra_1', 'extra_2'])
ShortSingleWriteExtraFullLowLevel = namedtuple('ShortSingleWriteExtraFullLowLevel', ['extra_4', 'message_written', 'extra_5'])
NormalReadLowLevel = namedtuple('NormalReadLowLevel', ['message_length', 'message_chunk_offset', 'message_chunk_data'])
NormalReadExtraIn1LowLevel = namedtuple('NormalReadExtraIn1LowLevel', ['message_length', 'message_chunk_offset', 'message_chunk_data'])
NormalReadExtraIn2LowLevel = namedtuple('NormalReadExtraIn2LowLevel', ['message_length', 'message_chunk_offset', 'message_chunk_data'])
NormalReadExtraOutPrefix1LowLevel = namedtuple('NormalReadExtraOutPrefix1LowLevel', ['extra', 'message_length', 'message_chunk_offset', 'message_chunk_data'])
NormalReadExtraOutPrefix2LowLevel = namedtuple('NormalReadExtraOutPrefix2LowLevel', ['extra_1', 'extra_2', 'message_length', 'message_chunk_offset', 'message_chunk_data'])
NormalReadExtraOutSuffix1LowLevel = namedtuple('NormalReadExtraOutSuffix1LowLevel', ['message_length', 'message_chunk_offset', 'message_chunk_data', 'extra'])
NormalReadExtraOutSuffix2LowLevel = namedtuple('NormalReadExtraOutSuffix2LowLevel', ['message_length', 'message_chunk_offset', 'message_chunk_data', 'extra_1', 'extra_2'])
NormalReadExtraOutFullLowLevel = namedtuple('NormalReadExtraOutFullLowLevel', ['extra_1', 'message_length', 'extra_2', 'message_chunk_offset', 'extra_3', 'message_chunk_data', 'extra_4'])
FixedReadLowLevel = namedtuple('FixedReadLowLevel', ['message_chunk_offset', 'message_chunk_data'])
FixedReadExtraIn1LowLevel = namedtuple('FixedReadExtraIn1LowLevel', ['message_chunk_offset', 'message_chunk_data'])
FixedReadExtraIn2LowLevel = namedtuple('FixedReadExtraIn2LowLevel', ['message_chunk_offset', 'message_chunk_data'])
FixedReadExtraOutPrefix1LowLevel = namedtuple('FixedReadExtraOutPrefix1LowLevel', ['extra', 'message_chunk_offset', 'message_chunk_data'])
FixedReadExtraOutPrefix2LowLevel = namedtuple('FixedReadExtraOutPrefix2LowLevel', ['extra_1', 'extra_2', 'message_chunk_offset', 'message_chunk_data'])
FixedReadExtraOutSuffix1LowLevel = namedtuple('FixedReadExtraOutSuffix1LowLevel', ['message_chunk_offset', 'message_chunk_data', 'extra'])
FixedReadExtraOutSuffix2LowLevel = namedtuple('FixedReadExtraOutSuffix2LowLevel', ['message_chunk_offset', 'message_chunk_data', 'extra_1', 'extra_2'])
FixedReadExtraOutFullLowLevel = namedtuple('FixedReadExtraOutFullLowLevel', ['extra_1', 'message_chunk_offset', 'extra_2', 'message_chunk_data', 'extra_3'])
SingleReadLowLevel = namedtuple('SingleReadLowLevel', ['message_length', 'message_data'])
SingleReadExtraIn1LowLevel = namedtuple('SingleReadExtraIn1LowLevel', ['message_length', 'message_data'])
SingleReadExtraIn2LowLevel = namedtuple('SingleReadExtraIn2LowLevel', ['message_length', 'message_data'])
SingleReadExtraOutPrefix1LowLevel = namedtuple('SingleReadExtraOutPrefix1LowLevel', ['extra', 'message_length', 'message_data'])
SingleReadExtraOutPrefix2LowLevel = namedtuple('SingleReadExtraOutPrefix2LowLevel', ['extra_1', 'extra_2', 'message_length', 'message_data'])
SingleReadExtraOutSuffix1LowLevel = namedtuple('SingleReadExtraOutSuffix1LowLevel', ['message_length', 'message_data', 'extra'])
SingleReadExtraOutSuffix2LowLevel = namedtuple('SingleReadExtraOutSuffix2LowLevel', ['message_length', 'message_data', 'extra_1', 'extra_2'])
SingleReadExtraOutFullLowLevel = namedtuple('SingleReadExtraOutFullLowLevel', ['extra_1', 'message_length', 'extra_2', 'message_data', 'extra_3'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])
NormalWriteExtraOut2 = namedtuple('NormalWriteExtraOut2', ['extra_1', 'extra_2'])
FixedWriteExtraOut2 = namedtuple('FixedWriteExtraOut2', ['extra_1', 'extra_2'])
ShortWriteExtraOutPrefix1 = namedtuple('ShortWriteExtraOutPrefix1', ['extra', 'message_written'])
ShortWriteExtraOutPrefix2 = namedtuple('ShortWriteExtraOutPrefix2', ['extra_1', 'extra_2', 'message_written'])
ShortWriteExtraOutSuffix1 = namedtuple('ShortWriteExtraOutSuffix1', ['message_written', 'extra'])
ShortWriteExtraOutSuffix2 = namedtuple('ShortWriteExtraOutSuffix2', ['message_written', 'extra_1', 'extra_2'])
ShortWriteExtraFull = namedtuple('ShortWriteExtraFull', ['extra_5', 'message_written', 'extra_6'])
SingleWriteExtraOut2 = namedtuple('SingleWriteExtraOut2', ['extra_1', 'extra_2'])
ShortSingleWriteExtraOutPrefix1 = namedtuple('ShortSingleWriteExtraOutPrefix1', ['extra', 'message_written'])
ShortSingleWriteExtraOutPrefix2 = namedtuple('ShortSingleWriteExtraOutPrefix2', ['extra_1', 'extra_2', 'message_written'])
ShortSingleWriteExtraOutSuffix1 = namedtuple('ShortSingleWriteExtraOutSuffix1', ['message_written', 'extra'])
ShortSingleWriteExtraOutSuffix2 = namedtuple('ShortSingleWriteExtraOutSuffix2', ['message_written', 'extra_1', 'extra_2'])
ShortSingleWriteExtraFull = namedtuple('ShortSingleWriteExtraFull', ['extra_4', 'message_written', 'extra_5'])
NormalReadExtraOutPrefix1 = namedtuple('NormalReadExtraOutPrefix1', ['extra', 'message'])
NormalReadExtraOutPrefix2 = namedtuple('NormalReadExtraOutPrefix2', ['extra_1', 'extra_2', 'message'])
NormalReadExtraOutSuffix1 = namedtuple('NormalReadExtraOutSuffix1', ['message', 'extra'])
NormalReadExtraOutSuffix2 = namedtuple('NormalReadExtraOutSuffix2', ['message', 'extra_1', 'extra_2'])
NormalReadExtraOutFull = namedtuple('NormalReadExtraOutFull', ['extra_1', 'extra_2', 'extra_3', 'message', 'extra_4'])
FixedReadExtraOutPrefix1 = namedtuple('FixedReadExtraOutPrefix1', ['extra', 'message'])
FixedReadExtraOutPrefix2 = namedtuple('FixedReadExtraOutPrefix2', ['extra_1', 'extra_2', 'message'])
FixedReadExtraOutSuffix1 = namedtuple('FixedReadExtraOutSuffix1', ['message', 'extra'])
FixedReadExtraOutSuffix2 = namedtuple('FixedReadExtraOutSuffix2', ['message', 'extra_1', 'extra_2'])
FixedReadExtraOutFull = namedtuple('FixedReadExtraOutFull', ['extra_1', 'extra_2', 'message', 'extra_3'])
SingleReadExtraOutPrefix1 = namedtuple('SingleReadExtraOutPrefix1', ['extra', 'message'])
SingleReadExtraOutPrefix2 = namedtuple('SingleReadExtraOutPrefix2', ['extra_1', 'extra_2', 'message'])
SingleReadExtraOutSuffix1 = namedtuple('SingleReadExtraOutSuffix1', ['message', 'extra'])
SingleReadExtraOutSuffix2 = namedtuple('SingleReadExtraOutSuffix2', ['message', 'extra_1', 'extra_2'])
SingleReadExtraOutFull = namedtuple('SingleReadExtraOutFull', ['extra_1', 'extra_2', 'message', 'extra_3'])

class BrickletStreamTest(Device):
    r"""

    """

    DEVICE_IDENTIFIER = 21111
    DEVICE_DISPLAY_NAME = 'Stream Test Bricklet'
    DEVICE_URL_PART = 'stream_test' # internal

    CALLBACK_NORMAL_READ_LOW_LEVEL = 70
    CALLBACK_NORMAL_READ_EXTRA_PREFIX_1_LOW_LEVEL = 71
    CALLBACK_NORMAL_READ_EXTRA_PREFIX_2_LOW_LEVEL = 72
    CALLBACK_NORMAL_READ_EXTRA_SUFFIX_1_LOW_LEVEL = 73
    CALLBACK_NORMAL_READ_EXTRA_SUFFIX_2_LOW_LEVEL = 74
    CALLBACK_NORMAL_READ_EXTRA_FULL_LOW_LEVEL = 75
    CALLBACK_FIXED_READ_LOW_LEVEL = 76
    CALLBACK_FIXED_READ_EXTRA_PREFIX_1_LOW_LEVEL = 77
    CALLBACK_FIXED_READ_EXTRA_PREFIX_2_LOW_LEVEL = 78
    CALLBACK_FIXED_READ_EXTRA_SUFFIX_1_LOW_LEVEL = 79
    CALLBACK_FIXED_READ_EXTRA_SUFFIX_2_LOW_LEVEL = 80
    CALLBACK_FIXED_READ_EXTRA_FULL_LOW_LEVEL = 81
    CALLBACK_SINGLE_READ_LOW_LEVEL = 82
    CALLBACK_SINGLE_READ_EXTRA_PREFIX_1_LOW_LEVEL = 83
    CALLBACK_SINGLE_READ_EXTRA_PREFIX_2_LOW_LEVEL = 84
    CALLBACK_SINGLE_READ_EXTRA_SUFFIX_1_LOW_LEVEL = 85
    CALLBACK_SINGLE_READ_EXTRA_SUFFIX_2_LOW_LEVEL = 86
    CALLBACK_SINGLE_READ_EXTRA_FULL_LOW_LEVEL = 87

    CALLBACK_NORMAL_READ = -70
    CALLBACK_NORMAL_READ_EXTRA_PREFIX_1 = -71
    CALLBACK_NORMAL_READ_EXTRA_PREFIX_2 = -72
    CALLBACK_NORMAL_READ_EXTRA_SUFFIX_1 = -73
    CALLBACK_NORMAL_READ_EXTRA_SUFFIX_2 = -74
    CALLBACK_NORMAL_READ_EXTRA_FULL = -75
    CALLBACK_FIXED_READ = -76
    CALLBACK_FIXED_READ_EXTRA_PREFIX_1 = -77
    CALLBACK_FIXED_READ_EXTRA_PREFIX_2 = -78
    CALLBACK_FIXED_READ_EXTRA_SUFFIX_1 = -79
    CALLBACK_FIXED_READ_EXTRA_SUFFIX_2 = -80
    CALLBACK_FIXED_READ_EXTRA_FULL = -81
    CALLBACK_SINGLE_READ = -82
    CALLBACK_SINGLE_READ_EXTRA_PREFIX_1 = -83
    CALLBACK_SINGLE_READ_EXTRA_PREFIX_2 = -84
    CALLBACK_SINGLE_READ_EXTRA_SUFFIX_1 = -85
    CALLBACK_SINGLE_READ_EXTRA_SUFFIX_2 = -86
    CALLBACK_SINGLE_READ_EXTRA_FULL = -87

    FUNCTION_NORMAL_WRITE_LOW_LEVEL = 1
    FUNCTION_NORMAL_WRITE_EXTRA_IN_PREFIX_1_LOW_LEVEL = 2
    FUNCTION_NORMAL_WRITE_EXTRA_IN_PREFIX_2_LOW_LEVEL = 3
    FUNCTION_NORMAL_WRITE_EXTRA_IN_SUFFIX_1_LOW_LEVEL = 4
    FUNCTION_NORMAL_WRITE_EXTRA_IN_SUFFIX_2_LOW_LEVEL = 5
    FUNCTION_NORMAL_WRITE_EXTRA_IN_FULL_LOW_LEVEL = 6
    FUNCTION_NORMAL_WRITE_EXTRA_OUT_1_LOW_LEVEL = 7
    FUNCTION_NORMAL_WRITE_EXTRA_OUT_2_LOW_LEVEL = 8
    FUNCTION_FIXED_WRITE_LOW_LEVEL = 9
    FUNCTION_FIXED_WRITE_EXTRA_IN_PREFIX_1_LOW_LEVEL = 10
    FUNCTION_FIXED_WRITE_EXTRA_IN_PREFIX_2_LOW_LEVEL = 11
    FUNCTION_FIXED_WRITE_EXTRA_IN_SUFFIX_1_LOW_LEVEL = 12
    FUNCTION_FIXED_WRITE_EXTRA_IN_SUFFIX_2_LOW_LEVEL = 13
    FUNCTION_FIXED_WRITE_EXTRA_IN_FULL_LOW_LEVEL = 14
    FUNCTION_FIXED_WRITE_EXTRA_OUT_1_LOW_LEVEL = 15
    FUNCTION_FIXED_WRITE_EXTRA_OUT_2_LOW_LEVEL = 16
    FUNCTION_SHORT_WRITE_LOW_LEVEL = 17
    FUNCTION_SHORT_WRITE_EXTRA_IN_PREFIX_1_LOW_LEVEL = 18
    FUNCTION_SHORT_WRITE_EXTRA_IN_PREFIX_2_LOW_LEVEL = 19
    FUNCTION_SHORT_WRITE_EXTRA_IN_SUFFIX_1_LOW_LEVEL = 20
    FUNCTION_SHORT_WRITE_EXTRA_IN_SUFFIX_2_LOW_LEVEL = 21
    FUNCTION_SHORT_WRITE_EXTRA_OUT_PREFIX_1_LOW_LEVEL = 22
    FUNCTION_SHORT_WRITE_EXTRA_OUT_PREFIX_2_LOW_LEVEL = 23
    FUNCTION_SHORT_WRITE_EXTRA_OUT_SUFFIX_1_LOW_LEVEL = 24
    FUNCTION_SHORT_WRITE_EXTRA_OUT_SUFFIX_2_LOW_LEVEL = 25
    FUNCTION_SHORT_WRITE_EXTRA_FULL_LOW_LEVEL = 26
    FUNCTION_SHORT_WRITE_BOOL_LOW_LEVEL = 27
    FUNCTION_SINGLE_WRITE_LOW_LEVEL = 28
    FUNCTION_SINGLE_WRITE_EXTRA_IN_PREFIX_1_LOW_LEVEL = 29
    FUNCTION_SINGLE_WRITE_EXTRA_IN_PREFIX_2_LOW_LEVEL = 30
    FUNCTION_SINGLE_WRITE_EXTRA_IN_SUFFIX_1_LOW_LEVEL = 31
    FUNCTION_SINGLE_WRITE_EXTRA_IN_SUFFIX_2_LOW_LEVEL = 32
    FUNCTION_SINGLE_WRITE_EXTRA_IN_FULL_LOW_LEVEL = 33
    FUNCTION_SINGLE_WRITE_EXTRA_OUT_1_LOW_LEVEL = 34
    FUNCTION_SINGLE_WRITE_EXTRA_OUT_2_LOW_LEVEL = 35
    FUNCTION_SHORT_SINGLE_WRITE_LOW_LEVEL = 36
    FUNCTION_SHORT_SINGLE_WRITE_EXTRA_IN_PREFIX_1_LOW_LEVEL = 37
    FUNCTION_SHORT_SINGLE_WRITE_EXTRA_IN_PREFIX_2_LOW_LEVEL = 38
    FUNCTION_SHORT_SINGLE_WRITE_EXTRA_IN_SUFFIX_1_LOW_LEVEL = 39
    FUNCTION_SHORT_SINGLE_WRITE_EXTRA_IN_SUFFIX_2_LOW_LEVEL = 40
    FUNCTION_SHORT_SINGLE_WRITE_EXTRA_OUT_PREFIX_1_LOW_LEVEL = 41
    FUNCTION_SHORT_SINGLE_WRITE_EXTRA_OUT_PREFIX_2_LOW_LEVEL = 42
    FUNCTION_SHORT_SINGLE_WRITE_EXTRA_OUT_SUFFIX_1_LOW_LEVEL = 43
    FUNCTION_SHORT_SINGLE_WRITE_EXTRA_OUT_SUFFIX_2_LOW_LEVEL = 44
    FUNCTION_SHORT_SINGLE_WRITE_EXTRA_FULL_LOW_LEVEL = 45
    FUNCTION_NORMAL_READ_LOW_LEVEL = 46
    FUNCTION_NORMAL_READ_EXTRA_IN_1_LOW_LEVEL = 47
    FUNCTION_NORMAL_READ_EXTRA_IN_2_LOW_LEVEL = 48
    FUNCTION_NORMAL_READ_EXTRA_OUT_PREFIX_1_LOW_LEVEL = 49
    FUNCTION_NORMAL_READ_EXTRA_OUT_PREFIX_2_LOW_LEVEL = 50
    FUNCTION_NORMAL_READ_EXTRA_OUT_SUFFIX_1_LOW_LEVEL = 51
    FUNCTION_NORMAL_READ_EXTRA_OUT_SUFFIX_2_LOW_LEVEL = 52
    FUNCTION_NORMAL_READ_EXTRA_OUT_FULL_LOW_LEVEL = 53
    FUNCTION_FIXED_READ_LOW_LEVEL = 54
    FUNCTION_FIXED_READ_EXTRA_IN_1_LOW_LEVEL = 55
    FUNCTION_FIXED_READ_EXTRA_IN_2_LOW_LEVEL = 56
    FUNCTION_FIXED_READ_EXTRA_OUT_PREFIX_1_LOW_LEVEL = 57
    FUNCTION_FIXED_READ_EXTRA_OUT_PREFIX_2_LOW_LEVEL = 58
    FUNCTION_FIXED_READ_EXTRA_OUT_SUFFIX_1_LOW_LEVEL = 59
    FUNCTION_FIXED_READ_EXTRA_OUT_SUFFIX_2_LOW_LEVEL = 60
    FUNCTION_FIXED_READ_EXTRA_OUT_FULL_LOW_LEVEL = 61
    FUNCTION_SINGLE_READ_LOW_LEVEL = 62
    FUNCTION_SINGLE_READ_EXTRA_IN_1_LOW_LEVEL = 63
    FUNCTION_SINGLE_READ_EXTRA_IN_2_LOW_LEVEL = 64
    FUNCTION_SINGLE_READ_EXTRA_OUT_PREFIX_1_LOW_LEVEL = 65
    FUNCTION_SINGLE_READ_EXTRA_OUT_PREFIX_2_LOW_LEVEL = 66
    FUNCTION_SINGLE_READ_EXTRA_OUT_SUFFIX_1_LOW_LEVEL = 67
    FUNCTION_SINGLE_READ_EXTRA_OUT_SUFFIX_2_LOW_LEVEL = 68
    FUNCTION_SINGLE_READ_EXTRA_OUT_FULL_LOW_LEVEL = 69
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
        Device.__init__(self, uid, ipcon, BrickletStreamTest.DEVICE_IDENTIFIER, BrickletStreamTest.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletStreamTest.FUNCTION_NORMAL_WRITE_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_NORMAL_WRITE_EXTRA_IN_PREFIX_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_NORMAL_WRITE_EXTRA_IN_PREFIX_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_NORMAL_WRITE_EXTRA_IN_SUFFIX_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_NORMAL_WRITE_EXTRA_IN_SUFFIX_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_NORMAL_WRITE_EXTRA_IN_FULL_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_NORMAL_WRITE_EXTRA_OUT_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_NORMAL_WRITE_EXTRA_OUT_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_FIXED_WRITE_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_FIXED_WRITE_EXTRA_IN_PREFIX_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_FIXED_WRITE_EXTRA_IN_PREFIX_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_FIXED_WRITE_EXTRA_IN_SUFFIX_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_FIXED_WRITE_EXTRA_IN_SUFFIX_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_FIXED_WRITE_EXTRA_IN_FULL_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_FIXED_WRITE_EXTRA_OUT_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_FIXED_WRITE_EXTRA_OUT_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SHORT_WRITE_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SHORT_WRITE_EXTRA_IN_PREFIX_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SHORT_WRITE_EXTRA_IN_PREFIX_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SHORT_WRITE_EXTRA_IN_SUFFIX_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SHORT_WRITE_EXTRA_IN_SUFFIX_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SHORT_WRITE_EXTRA_OUT_PREFIX_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SHORT_WRITE_EXTRA_OUT_PREFIX_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SHORT_WRITE_EXTRA_OUT_SUFFIX_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SHORT_WRITE_EXTRA_OUT_SUFFIX_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SHORT_WRITE_EXTRA_FULL_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SHORT_WRITE_BOOL_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SINGLE_WRITE_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SINGLE_WRITE_EXTRA_IN_PREFIX_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SINGLE_WRITE_EXTRA_IN_PREFIX_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SINGLE_WRITE_EXTRA_IN_SUFFIX_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SINGLE_WRITE_EXTRA_IN_SUFFIX_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SINGLE_WRITE_EXTRA_IN_FULL_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SINGLE_WRITE_EXTRA_OUT_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SINGLE_WRITE_EXTRA_OUT_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SHORT_SINGLE_WRITE_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SHORT_SINGLE_WRITE_EXTRA_IN_PREFIX_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SHORT_SINGLE_WRITE_EXTRA_IN_PREFIX_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SHORT_SINGLE_WRITE_EXTRA_IN_SUFFIX_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SHORT_SINGLE_WRITE_EXTRA_IN_SUFFIX_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SHORT_SINGLE_WRITE_EXTRA_OUT_PREFIX_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SHORT_SINGLE_WRITE_EXTRA_OUT_PREFIX_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SHORT_SINGLE_WRITE_EXTRA_OUT_SUFFIX_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SHORT_SINGLE_WRITE_EXTRA_OUT_SUFFIX_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SHORT_SINGLE_WRITE_EXTRA_FULL_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_NORMAL_READ_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_NORMAL_READ_EXTRA_IN_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_NORMAL_READ_EXTRA_IN_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_NORMAL_READ_EXTRA_OUT_PREFIX_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_NORMAL_READ_EXTRA_OUT_PREFIX_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_NORMAL_READ_EXTRA_OUT_SUFFIX_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_NORMAL_READ_EXTRA_OUT_SUFFIX_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_NORMAL_READ_EXTRA_OUT_FULL_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_FIXED_READ_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_FIXED_READ_EXTRA_IN_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_FIXED_READ_EXTRA_IN_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_FIXED_READ_EXTRA_OUT_PREFIX_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_FIXED_READ_EXTRA_OUT_PREFIX_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_FIXED_READ_EXTRA_OUT_SUFFIX_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_FIXED_READ_EXTRA_OUT_SUFFIX_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_FIXED_READ_EXTRA_OUT_FULL_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SINGLE_READ_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SINGLE_READ_EXTRA_IN_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SINGLE_READ_EXTRA_IN_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SINGLE_READ_EXTRA_OUT_PREFIX_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SINGLE_READ_EXTRA_OUT_PREFIX_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SINGLE_READ_EXTRA_OUT_SUFFIX_1_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SINGLE_READ_EXTRA_OUT_SUFFIX_2_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SINGLE_READ_EXTRA_OUT_FULL_LOW_LEVEL] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SET_BOOTLOADER_MODE] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_GET_BOOTLOADER_MODE] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletStreamTest.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletStreamTest.FUNCTION_WRITE_FIRMWARE] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletStreamTest.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletStreamTest.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_RESET] = BrickletStreamTest.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletStreamTest.FUNCTION_WRITE_UID] = BrickletStreamTest.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletStreamTest.FUNCTION_READ_UID] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletStreamTest.FUNCTION_GET_IDENTITY] = BrickletStreamTest.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletStreamTest.CALLBACK_NORMAL_READ_LOW_LEVEL] = (72, 'H H 60c')
        self.callback_formats[BrickletStreamTest.CALLBACK_NORMAL_READ_EXTRA_PREFIX_1_LOW_LEVEL] = (72, 'B H H 59c')
        self.callback_formats[BrickletStreamTest.CALLBACK_NORMAL_READ_EXTRA_PREFIX_2_LOW_LEVEL] = (72, 'B B H H 58c')
        self.callback_formats[BrickletStreamTest.CALLBACK_NORMAL_READ_EXTRA_SUFFIX_1_LOW_LEVEL] = (72, 'H H 59c B')
        self.callback_formats[BrickletStreamTest.CALLBACK_NORMAL_READ_EXTRA_SUFFIX_2_LOW_LEVEL] = (72, 'H H 58c B B')
        self.callback_formats[BrickletStreamTest.CALLBACK_NORMAL_READ_EXTRA_FULL_LOW_LEVEL] = (72, 'B H B H B 56c B')
        self.callback_formats[BrickletStreamTest.CALLBACK_FIXED_READ_LOW_LEVEL] = (72, 'H 62c')
        self.callback_formats[BrickletStreamTest.CALLBACK_FIXED_READ_EXTRA_PREFIX_1_LOW_LEVEL] = (72, 'B H 61c')
        self.callback_formats[BrickletStreamTest.CALLBACK_FIXED_READ_EXTRA_PREFIX_2_LOW_LEVEL] = (72, 'B B H 60c')
        self.callback_formats[BrickletStreamTest.CALLBACK_FIXED_READ_EXTRA_SUFFIX_1_LOW_LEVEL] = (72, 'H 61c B')
        self.callback_formats[BrickletStreamTest.CALLBACK_FIXED_READ_EXTRA_SUFFIX_2_LOW_LEVEL] = (72, 'H 60c B B')
        self.callback_formats[BrickletStreamTest.CALLBACK_FIXED_READ_EXTRA_FULL_LOW_LEVEL] = (72, 'B H B 59c B')
        self.callback_formats[BrickletStreamTest.CALLBACK_SINGLE_READ_LOW_LEVEL] = (72, 'B 63c')
        self.callback_formats[BrickletStreamTest.CALLBACK_SINGLE_READ_EXTRA_PREFIX_1_LOW_LEVEL] = (72, 'B B 62c')
        self.callback_formats[BrickletStreamTest.CALLBACK_SINGLE_READ_EXTRA_PREFIX_2_LOW_LEVEL] = (72, 'B B B 61c')
        self.callback_formats[BrickletStreamTest.CALLBACK_SINGLE_READ_EXTRA_SUFFIX_1_LOW_LEVEL] = (72, 'B 62c B')
        self.callback_formats[BrickletStreamTest.CALLBACK_SINGLE_READ_EXTRA_SUFFIX_2_LOW_LEVEL] = (72, 'B 61c B B')
        self.callback_formats[BrickletStreamTest.CALLBACK_SINGLE_READ_EXTRA_FULL_LOW_LEVEL] = (72, 'B B B 60c B')

        self.high_level_callbacks[BrickletStreamTest.CALLBACK_NORMAL_READ] = [('stream_length', 'stream_chunk_offset', 'stream_chunk_data'), {'fixed_length': None, 'single_chunk': False}, None]
        self.high_level_callbacks[BrickletStreamTest.CALLBACK_NORMAL_READ_EXTRA_PREFIX_1] = [(None, 'stream_length', 'stream_chunk_offset', 'stream_chunk_data'), {'fixed_length': None, 'single_chunk': False}, None]
        self.high_level_callbacks[BrickletStreamTest.CALLBACK_NORMAL_READ_EXTRA_PREFIX_2] = [(None, None, 'stream_length', 'stream_chunk_offset', 'stream_chunk_data'), {'fixed_length': None, 'single_chunk': False}, None]
        self.high_level_callbacks[BrickletStreamTest.CALLBACK_NORMAL_READ_EXTRA_SUFFIX_1] = [('stream_length', 'stream_chunk_offset', 'stream_chunk_data', None), {'fixed_length': None, 'single_chunk': False}, None]
        self.high_level_callbacks[BrickletStreamTest.CALLBACK_NORMAL_READ_EXTRA_SUFFIX_2] = [('stream_length', 'stream_chunk_offset', 'stream_chunk_data', None, None), {'fixed_length': None, 'single_chunk': False}, None]
        self.high_level_callbacks[BrickletStreamTest.CALLBACK_NORMAL_READ_EXTRA_FULL] = [(None, 'stream_length', None, 'stream_chunk_offset', None, 'stream_chunk_data', None), {'fixed_length': None, 'single_chunk': False}, None]
        self.high_level_callbacks[BrickletStreamTest.CALLBACK_FIXED_READ] = [('stream_chunk_offset', 'stream_chunk_data'), {'fixed_length': 1000, 'single_chunk': False}, None]
        self.high_level_callbacks[BrickletStreamTest.CALLBACK_FIXED_READ_EXTRA_PREFIX_1] = [(None, 'stream_chunk_offset', 'stream_chunk_data'), {'fixed_length': 1000, 'single_chunk': False}, None]
        self.high_level_callbacks[BrickletStreamTest.CALLBACK_FIXED_READ_EXTRA_PREFIX_2] = [(None, None, 'stream_chunk_offset', 'stream_chunk_data'), {'fixed_length': 1000, 'single_chunk': False}, None]
        self.high_level_callbacks[BrickletStreamTest.CALLBACK_FIXED_READ_EXTRA_SUFFIX_1] = [('stream_chunk_offset', 'stream_chunk_data', None), {'fixed_length': 1000, 'single_chunk': False}, None]
        self.high_level_callbacks[BrickletStreamTest.CALLBACK_FIXED_READ_EXTRA_SUFFIX_2] = [('stream_chunk_offset', 'stream_chunk_data', None, None), {'fixed_length': 1000, 'single_chunk': False}, None]
        self.high_level_callbacks[BrickletStreamTest.CALLBACK_FIXED_READ_EXTRA_FULL] = [(None, 'stream_chunk_offset', None, 'stream_chunk_data', None), {'fixed_length': 1000, 'single_chunk': False}, None]
        self.high_level_callbacks[BrickletStreamTest.CALLBACK_SINGLE_READ] = [('stream_length', 'stream_chunk_data'), {'fixed_length': None, 'single_chunk': True}, None]
        self.high_level_callbacks[BrickletStreamTest.CALLBACK_SINGLE_READ_EXTRA_PREFIX_1] = [(None, 'stream_length', 'stream_chunk_data'), {'fixed_length': None, 'single_chunk': True}, None]
        self.high_level_callbacks[BrickletStreamTest.CALLBACK_SINGLE_READ_EXTRA_PREFIX_2] = [(None, None, 'stream_length', 'stream_chunk_data'), {'fixed_length': None, 'single_chunk': True}, None]
        self.high_level_callbacks[BrickletStreamTest.CALLBACK_SINGLE_READ_EXTRA_SUFFIX_1] = [('stream_length', 'stream_chunk_data', None), {'fixed_length': None, 'single_chunk': True}, None]
        self.high_level_callbacks[BrickletStreamTest.CALLBACK_SINGLE_READ_EXTRA_SUFFIX_2] = [('stream_length', 'stream_chunk_data', None, None), {'fixed_length': None, 'single_chunk': True}, None]
        self.high_level_callbacks[BrickletStreamTest.CALLBACK_SINGLE_READ_EXTRA_FULL] = [(None, 'stream_length', None, 'stream_chunk_data', None), {'fixed_length': None, 'single_chunk': True}, None]
        ipcon.add_device(self)

    def normal_write_low_level(self, message_length, message_chunk_offset, message_chunk_data):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = create_char_list(message_chunk_data)

        self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_NORMAL_WRITE_LOW_LEVEL, (message_length, message_chunk_offset, message_chunk_data), 'H H 60c', 0, '')

    def normal_write_extra_in_prefix_1_low_level(self, extra, message_length, message_chunk_offset, message_chunk_data):
        r"""

        """
        self.check_validity()

        extra = int(extra)
        message_length = int(message_length)
        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = create_char_list(message_chunk_data)

        self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_NORMAL_WRITE_EXTRA_IN_PREFIX_1_LOW_LEVEL, (extra, message_length, message_chunk_offset, message_chunk_data), 'B H H 59c', 0, '')

    def normal_write_extra_in_prefix_2_low_level(self, extra_1, extra_2, message_length, message_chunk_offset, message_chunk_data):
        r"""

        """
        self.check_validity()

        extra_1 = int(extra_1)
        extra_2 = int(extra_2)
        message_length = int(message_length)
        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = create_char_list(message_chunk_data)

        self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_NORMAL_WRITE_EXTRA_IN_PREFIX_2_LOW_LEVEL, (extra_1, extra_2, message_length, message_chunk_offset, message_chunk_data), 'B B H H 58c', 0, '')

    def normal_write_extra_in_suffix_1_low_level(self, message_length, message_chunk_offset, message_chunk_data, extra):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = create_char_list(message_chunk_data)
        extra = int(extra)

        self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_NORMAL_WRITE_EXTRA_IN_SUFFIX_1_LOW_LEVEL, (message_length, message_chunk_offset, message_chunk_data, extra), 'H H 59c B', 0, '')

    def normal_write_extra_in_suffix_2_low_level(self, message_length, message_chunk_offset, message_chunk_data, extra_1, extra_2):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = create_char_list(message_chunk_data)
        extra_1 = int(extra_1)
        extra_2 = int(extra_2)

        self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_NORMAL_WRITE_EXTRA_IN_SUFFIX_2_LOW_LEVEL, (message_length, message_chunk_offset, message_chunk_data, extra_1, extra_2), 'H H 58c B B', 0, '')

    def normal_write_extra_in_full_low_level(self, extra_1, message_length, extra_2, message_chunk_offset, extra_3, message_chunk_data, extra_4):
        r"""

        """
        self.check_validity()

        extra_1 = int(extra_1)
        message_length = int(message_length)
        extra_2 = int(extra_2)
        message_chunk_offset = int(message_chunk_offset)
        extra_3 = int(extra_3)
        message_chunk_data = create_char_list(message_chunk_data)
        extra_4 = int(extra_4)

        self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_NORMAL_WRITE_EXTRA_IN_FULL_LOW_LEVEL, (extra_1, message_length, extra_2, message_chunk_offset, extra_3, message_chunk_data, extra_4), 'B H B H B 56c B', 0, '')

    def normal_write_extra_out_1_low_level(self, message_length, message_chunk_offset, message_chunk_data):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = create_char_list(message_chunk_data)

        return self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_NORMAL_WRITE_EXTRA_OUT_1_LOW_LEVEL, (message_length, message_chunk_offset, message_chunk_data), 'H H 60c', 9, 'B')

    def normal_write_extra_out_2_low_level(self, message_length, message_chunk_offset, message_chunk_data):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = create_char_list(message_chunk_data)

        return NormalWriteExtraOut2LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_NORMAL_WRITE_EXTRA_OUT_2_LOW_LEVEL, (message_length, message_chunk_offset, message_chunk_data), 'H H 60c', 10, 'B B'))

    def fixed_write_low_level(self, message_chunk_offset, message_chunk_data):
        r"""

        """
        self.check_validity()

        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = create_char_list(message_chunk_data)

        self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_FIXED_WRITE_LOW_LEVEL, (message_chunk_offset, message_chunk_data), 'H 62c', 0, '')

    def fixed_write_extra_in_prefix_1_low_level(self, extra, message_chunk_offset, message_chunk_data):
        r"""

        """
        self.check_validity()

        extra = int(extra)
        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = create_char_list(message_chunk_data)

        self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_FIXED_WRITE_EXTRA_IN_PREFIX_1_LOW_LEVEL, (extra, message_chunk_offset, message_chunk_data), 'B H 61c', 0, '')

    def fixed_write_extra_in_prefix_2_low_level(self, extra_1, extra_2, message_chunk_offset, message_chunk_data):
        r"""

        """
        self.check_validity()

        extra_1 = int(extra_1)
        extra_2 = int(extra_2)
        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = create_char_list(message_chunk_data)

        self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_FIXED_WRITE_EXTRA_IN_PREFIX_2_LOW_LEVEL, (extra_1, extra_2, message_chunk_offset, message_chunk_data), 'B B H 60c', 0, '')

    def fixed_write_extra_in_suffix_1_low_level(self, message_chunk_offset, message_chunk_data, extra):
        r"""

        """
        self.check_validity()

        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = create_char_list(message_chunk_data)
        extra = int(extra)

        self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_FIXED_WRITE_EXTRA_IN_SUFFIX_1_LOW_LEVEL, (message_chunk_offset, message_chunk_data, extra), 'H 61c B', 0, '')

    def fixed_write_extra_in_suffix_2_low_level(self, message_chunk_offset, message_chunk_data, extra_1, extra_2):
        r"""

        """
        self.check_validity()

        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = create_char_list(message_chunk_data)
        extra_1 = int(extra_1)
        extra_2 = int(extra_2)

        self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_FIXED_WRITE_EXTRA_IN_SUFFIX_2_LOW_LEVEL, (message_chunk_offset, message_chunk_data, extra_1, extra_2), 'H 60c B B', 0, '')

    def fixed_write_extra_in_full_low_level(self, extra_1, message_chunk_offset, extra_2, message_chunk_data, extra_3):
        r"""

        """
        self.check_validity()

        extra_1 = int(extra_1)
        message_chunk_offset = int(message_chunk_offset)
        extra_2 = int(extra_2)
        message_chunk_data = create_char_list(message_chunk_data)
        extra_3 = int(extra_3)

        self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_FIXED_WRITE_EXTRA_IN_FULL_LOW_LEVEL, (extra_1, message_chunk_offset, extra_2, message_chunk_data, extra_3), 'B H B 59c B', 0, '')

    def fixed_write_extra_out_1_low_level(self, message_chunk_offset, message_chunk_data):
        r"""

        """
        self.check_validity()

        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = create_char_list(message_chunk_data)

        return self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_FIXED_WRITE_EXTRA_OUT_1_LOW_LEVEL, (message_chunk_offset, message_chunk_data), 'H 62c', 9, 'B')

    def fixed_write_extra_out_2_low_level(self, message_chunk_offset, message_chunk_data):
        r"""

        """
        self.check_validity()

        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = create_char_list(message_chunk_data)

        return FixedWriteExtraOut2LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_FIXED_WRITE_EXTRA_OUT_2_LOW_LEVEL, (message_chunk_offset, message_chunk_data), 'H 62c', 10, 'B B'))

    def short_write_low_level(self, message_length, message_chunk_offset, message_chunk_data):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = create_char_list(message_chunk_data)

        return self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SHORT_WRITE_LOW_LEVEL, (message_length, message_chunk_offset, message_chunk_data), 'H H 60c', 9, 'B')

    def short_write_extra_in_prefix_1_low_level(self, extra, message_length, message_chunk_offset, message_chunk_data):
        r"""

        """
        self.check_validity()

        extra = int(extra)
        message_length = int(message_length)
        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = create_char_list(message_chunk_data)

        return self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SHORT_WRITE_EXTRA_IN_PREFIX_1_LOW_LEVEL, (extra, message_length, message_chunk_offset, message_chunk_data), 'B H H 59c', 9, 'B')

    def short_write_extra_in_prefix_2_low_level(self, extra_1, extra_2, message_length, message_chunk_offset, message_chunk_data):
        r"""

        """
        self.check_validity()

        extra_1 = int(extra_1)
        extra_2 = int(extra_2)
        message_length = int(message_length)
        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = create_char_list(message_chunk_data)

        return self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SHORT_WRITE_EXTRA_IN_PREFIX_2_LOW_LEVEL, (extra_1, extra_2, message_length, message_chunk_offset, message_chunk_data), 'B B H H 58c', 9, 'B')

    def short_write_extra_in_suffix_1_low_level(self, message_length, message_chunk_offset, message_chunk_data, extra):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = create_char_list(message_chunk_data)
        extra = int(extra)

        return self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SHORT_WRITE_EXTRA_IN_SUFFIX_1_LOW_LEVEL, (message_length, message_chunk_offset, message_chunk_data, extra), 'H H 59c B', 9, 'B')

    def short_write_extra_in_suffix_2_low_level(self, message_length, message_chunk_offset, message_chunk_data, extra_1, extra_2):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = create_char_list(message_chunk_data)
        extra_1 = int(extra_1)
        extra_2 = int(extra_2)

        return self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SHORT_WRITE_EXTRA_IN_SUFFIX_2_LOW_LEVEL, (message_length, message_chunk_offset, message_chunk_data, extra_1, extra_2), 'H H 58c B B', 9, 'B')

    def short_write_extra_out_prefix_1_low_level(self, message_length, message_chunk_offset, message_chunk_data):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = create_char_list(message_chunk_data)

        return ShortWriteExtraOutPrefix1LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SHORT_WRITE_EXTRA_OUT_PREFIX_1_LOW_LEVEL, (message_length, message_chunk_offset, message_chunk_data), 'H H 60c', 10, 'B B'))

    def short_write_extra_out_prefix_2_low_level(self, message_length, message_chunk_offset, message_chunk_data):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = create_char_list(message_chunk_data)

        return ShortWriteExtraOutPrefix2LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SHORT_WRITE_EXTRA_OUT_PREFIX_2_LOW_LEVEL, (message_length, message_chunk_offset, message_chunk_data), 'H H 60c', 11, 'B B B'))

    def short_write_extra_out_suffix_1_low_level(self, message_length, message_chunk_offset, message_chunk_data):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = create_char_list(message_chunk_data)

        return ShortWriteExtraOutSuffix1LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SHORT_WRITE_EXTRA_OUT_SUFFIX_1_LOW_LEVEL, (message_length, message_chunk_offset, message_chunk_data), 'H H 60c', 10, 'B B'))

    def short_write_extra_out_suffix_2_low_level(self, message_length, message_chunk_offset, message_chunk_data):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = create_char_list(message_chunk_data)

        return ShortWriteExtraOutSuffix2LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SHORT_WRITE_EXTRA_OUT_SUFFIX_2_LOW_LEVEL, (message_length, message_chunk_offset, message_chunk_data), 'H H 60c', 11, 'B B B'))

    def short_write_extra_full_low_level(self, extra_1, message_length, extra_2, message_chunk_offset, extra_3, message_chunk_data, extra_4):
        r"""

        """
        self.check_validity()

        extra_1 = int(extra_1)
        message_length = int(message_length)
        extra_2 = int(extra_2)
        message_chunk_offset = int(message_chunk_offset)
        extra_3 = int(extra_3)
        message_chunk_data = create_char_list(message_chunk_data)
        extra_4 = int(extra_4)

        return ShortWriteExtraFullLowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SHORT_WRITE_EXTRA_FULL_LOW_LEVEL, (extra_1, message_length, extra_2, message_chunk_offset, extra_3, message_chunk_data, extra_4), 'B H B H B 56c B', 11, 'B B B'))

    def short_write_bool_low_level(self, message_length, message_chunk_offset, message_chunk_data):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_chunk_offset = int(message_chunk_offset)
        message_chunk_data = list(map(bool, message_chunk_data))

        return self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SHORT_WRITE_BOOL_LOW_LEVEL, (message_length, message_chunk_offset, message_chunk_data), 'H H 480!', 10, 'H')

    def single_write_low_level(self, message_length, message_data):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_data = create_char_list(message_data)

        self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SINGLE_WRITE_LOW_LEVEL, (message_length, message_data), 'B 63c', 0, '')

    def single_write_extra_in_prefix_1_low_level(self, extra, message_length, message_data):
        r"""

        """
        self.check_validity()

        extra = int(extra)
        message_length = int(message_length)
        message_data = create_char_list(message_data)

        self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SINGLE_WRITE_EXTRA_IN_PREFIX_1_LOW_LEVEL, (extra, message_length, message_data), 'B B 62c', 0, '')

    def single_write_extra_in_prefix_2_low_level(self, extra_1, extra_2, message_length, message_data):
        r"""

        """
        self.check_validity()

        extra_1 = int(extra_1)
        extra_2 = int(extra_2)
        message_length = int(message_length)
        message_data = create_char_list(message_data)

        self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SINGLE_WRITE_EXTRA_IN_PREFIX_2_LOW_LEVEL, (extra_1, extra_2, message_length, message_data), 'B B B 61c', 0, '')

    def single_write_extra_in_suffix_1_low_level(self, message_length, message_data, extra):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_data = create_char_list(message_data)
        extra = int(extra)

        self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SINGLE_WRITE_EXTRA_IN_SUFFIX_1_LOW_LEVEL, (message_length, message_data, extra), 'B 62c B', 0, '')

    def single_write_extra_in_suffix_2_low_level(self, message_length, message_data, extra_1, extra_2):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_data = create_char_list(message_data)
        extra_1 = int(extra_1)
        extra_2 = int(extra_2)

        self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SINGLE_WRITE_EXTRA_IN_SUFFIX_2_LOW_LEVEL, (message_length, message_data, extra_1, extra_2), 'B 61c B B', 0, '')

    def single_write_extra_in_full_low_level(self, extra_1, message_length, extra_2, message_data, extra_3):
        r"""

        """
        self.check_validity()

        extra_1 = int(extra_1)
        message_length = int(message_length)
        extra_2 = int(extra_2)
        message_data = create_char_list(message_data)
        extra_3 = int(extra_3)

        self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SINGLE_WRITE_EXTRA_IN_FULL_LOW_LEVEL, (extra_1, message_length, extra_2, message_data, extra_3), 'B B B 60c B', 0, '')

    def single_write_extra_out_1_low_level(self, message_length, message_data):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_data = create_char_list(message_data)

        return self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SINGLE_WRITE_EXTRA_OUT_1_LOW_LEVEL, (message_length, message_data), 'B 63c', 9, 'B')

    def single_write_extra_out_2_low_level(self, message_length, message_data):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_data = create_char_list(message_data)

        return SingleWriteExtraOut2LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SINGLE_WRITE_EXTRA_OUT_2_LOW_LEVEL, (message_length, message_data), 'B 63c', 10, 'B B'))

    def short_single_write_low_level(self, message_length, message_data):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_data = create_char_list(message_data)

        return self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SHORT_SINGLE_WRITE_LOW_LEVEL, (message_length, message_data), 'B 63c', 9, 'B')

    def short_single_write_extra_in_prefix_1_low_level(self, extra, message_length, message_data):
        r"""

        """
        self.check_validity()

        extra = int(extra)
        message_length = int(message_length)
        message_data = create_char_list(message_data)

        return self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SHORT_SINGLE_WRITE_EXTRA_IN_PREFIX_1_LOW_LEVEL, (extra, message_length, message_data), 'B B 62c', 9, 'B')

    def short_single_write_extra_in_prefix_2_low_level(self, extra_1, extra_2, message_length, message_data):
        r"""

        """
        self.check_validity()

        extra_1 = int(extra_1)
        extra_2 = int(extra_2)
        message_length = int(message_length)
        message_data = create_char_list(message_data)

        return self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SHORT_SINGLE_WRITE_EXTRA_IN_PREFIX_2_LOW_LEVEL, (extra_1, extra_2, message_length, message_data), 'B B B 61c', 9, 'B')

    def short_single_write_extra_in_suffix_1_low_level(self, message_length, message_data, extra):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_data = create_char_list(message_data)
        extra = int(extra)

        return self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SHORT_SINGLE_WRITE_EXTRA_IN_SUFFIX_1_LOW_LEVEL, (message_length, message_data, extra), 'B 62c B', 9, 'B')

    def short_single_write_extra_in_suffix_2_low_level(self, message_length, message_data, extra_1, extra_2):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_data = create_char_list(message_data)
        extra_1 = int(extra_1)
        extra_2 = int(extra_2)

        return self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SHORT_SINGLE_WRITE_EXTRA_IN_SUFFIX_2_LOW_LEVEL, (message_length, message_data, extra_1, extra_2), 'B 61c B B', 9, 'B')

    def short_single_write_extra_out_prefix_1_low_level(self, message_length, message_data):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_data = create_char_list(message_data)

        return ShortSingleWriteExtraOutPrefix1LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SHORT_SINGLE_WRITE_EXTRA_OUT_PREFIX_1_LOW_LEVEL, (message_length, message_data), 'B 62c', 10, 'B B'))

    def short_single_write_extra_out_prefix_2_low_level(self, message_length, message_data):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_data = create_char_list(message_data)

        return ShortSingleWriteExtraOutPrefix2LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SHORT_SINGLE_WRITE_EXTRA_OUT_PREFIX_2_LOW_LEVEL, (message_length, message_data), 'B 62c', 11, 'B B B'))

    def short_single_write_extra_out_suffix_1_low_level(self, message_length, message_data):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_data = create_char_list(message_data)

        return ShortSingleWriteExtraOutSuffix1LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SHORT_SINGLE_WRITE_EXTRA_OUT_SUFFIX_1_LOW_LEVEL, (message_length, message_data), 'B 63c', 10, 'B B'))

    def short_single_write_extra_out_suffix_2_low_level(self, message_length, message_data):
        r"""

        """
        self.check_validity()

        message_length = int(message_length)
        message_data = create_char_list(message_data)

        return ShortSingleWriteExtraOutSuffix2LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SHORT_SINGLE_WRITE_EXTRA_OUT_SUFFIX_2_LOW_LEVEL, (message_length, message_data), 'B 63c', 11, 'B B B'))

    def short_single_write_extra_full_low_level(self, extra_1, message_length, extra_2, message_data, extra_3):
        r"""

        """
        self.check_validity()

        extra_1 = int(extra_1)
        message_length = int(message_length)
        extra_2 = int(extra_2)
        message_data = create_char_list(message_data)
        extra_3 = int(extra_3)

        return ShortSingleWriteExtraFullLowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SHORT_SINGLE_WRITE_EXTRA_FULL_LOW_LEVEL, (extra_1, message_length, extra_2, message_data, extra_3), 'B B B 60c B', 11, 'B B B'))

    def normal_read_low_level(self):
        r"""

        """
        self.check_validity()

        return NormalReadLowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_NORMAL_READ_LOW_LEVEL, (), '', 72, 'H H 60c'))

    def normal_read_extra_in_1_low_level(self, extra):
        r"""

        """
        self.check_validity()

        extra = int(extra)

        return NormalReadExtraIn1LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_NORMAL_READ_EXTRA_IN_1_LOW_LEVEL, (extra,), 'B', 72, 'H H 60c'))

    def normal_read_extra_in_2_low_level(self, extra_1, extra_2):
        r"""

        """
        self.check_validity()

        extra_1 = int(extra_1)
        extra_2 = int(extra_2)

        return NormalReadExtraIn2LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_NORMAL_READ_EXTRA_IN_2_LOW_LEVEL, (extra_1, extra_2), 'B B', 72, 'H H 60c'))

    def normal_read_extra_out_prefix_1_low_level(self):
        r"""

        """
        self.check_validity()

        return NormalReadExtraOutPrefix1LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_NORMAL_READ_EXTRA_OUT_PREFIX_1_LOW_LEVEL, (), '', 72, 'B H H 59c'))

    def normal_read_extra_out_prefix_2_low_level(self):
        r"""

        """
        self.check_validity()

        return NormalReadExtraOutPrefix2LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_NORMAL_READ_EXTRA_OUT_PREFIX_2_LOW_LEVEL, (), '', 72, 'B B H H 58c'))

    def normal_read_extra_out_suffix_1_low_level(self):
        r"""

        """
        self.check_validity()

        return NormalReadExtraOutSuffix1LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_NORMAL_READ_EXTRA_OUT_SUFFIX_1_LOW_LEVEL, (), '', 72, 'H H 59c B'))

    def normal_read_extra_out_suffix_2_low_level(self):
        r"""

        """
        self.check_validity()

        return NormalReadExtraOutSuffix2LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_NORMAL_READ_EXTRA_OUT_SUFFIX_2_LOW_LEVEL, (), '', 72, 'H H 58c B B'))

    def normal_read_extra_out_full_low_level(self):
        r"""

        """
        self.check_validity()

        return NormalReadExtraOutFullLowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_NORMAL_READ_EXTRA_OUT_FULL_LOW_LEVEL, (), '', 72, 'B H B H B 56c B'))

    def fixed_read_low_level(self):
        r"""

        """
        self.check_validity()

        return FixedReadLowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_FIXED_READ_LOW_LEVEL, (), '', 72, 'H 62c'))

    def fixed_read_extra_in_1_low_level(self, extra):
        r"""

        """
        self.check_validity()

        extra = int(extra)

        return FixedReadExtraIn1LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_FIXED_READ_EXTRA_IN_1_LOW_LEVEL, (extra,), 'B', 72, 'H 62c'))

    def fixed_read_extra_in_2_low_level(self, extra_1, extra_2):
        r"""

        """
        self.check_validity()

        extra_1 = int(extra_1)
        extra_2 = int(extra_2)

        return FixedReadExtraIn2LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_FIXED_READ_EXTRA_IN_2_LOW_LEVEL, (extra_1, extra_2), 'B B', 72, 'H 62c'))

    def fixed_read_extra_out_prefix_1_low_level(self):
        r"""

        """
        self.check_validity()

        return FixedReadExtraOutPrefix1LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_FIXED_READ_EXTRA_OUT_PREFIX_1_LOW_LEVEL, (), '', 72, 'B H 61c'))

    def fixed_read_extra_out_prefix_2_low_level(self):
        r"""

        """
        self.check_validity()

        return FixedReadExtraOutPrefix2LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_FIXED_READ_EXTRA_OUT_PREFIX_2_LOW_LEVEL, (), '', 72, 'B B H 60c'))

    def fixed_read_extra_out_suffix_1_low_level(self):
        r"""

        """
        self.check_validity()

        return FixedReadExtraOutSuffix1LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_FIXED_READ_EXTRA_OUT_SUFFIX_1_LOW_LEVEL, (), '', 72, 'H 61c B'))

    def fixed_read_extra_out_suffix_2_low_level(self):
        r"""

        """
        self.check_validity()

        return FixedReadExtraOutSuffix2LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_FIXED_READ_EXTRA_OUT_SUFFIX_2_LOW_LEVEL, (), '', 72, 'H 60c B B'))

    def fixed_read_extra_out_full_low_level(self):
        r"""

        """
        self.check_validity()

        return FixedReadExtraOutFullLowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_FIXED_READ_EXTRA_OUT_FULL_LOW_LEVEL, (), '', 72, 'B H B 59c B'))

    def single_read_low_level(self):
        r"""

        """
        self.check_validity()

        return SingleReadLowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SINGLE_READ_LOW_LEVEL, (), '', 72, 'B 63c'))

    def single_read_extra_in_1_low_level(self, extra):
        r"""

        """
        self.check_validity()

        extra = int(extra)

        return SingleReadExtraIn1LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SINGLE_READ_EXTRA_IN_1_LOW_LEVEL, (extra,), 'B', 71, 'B 62c'))

    def single_read_extra_in_2_low_level(self, extra_1, extra_2):
        r"""

        """
        self.check_validity()

        extra_1 = int(extra_1)
        extra_2 = int(extra_2)

        return SingleReadExtraIn2LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SINGLE_READ_EXTRA_IN_2_LOW_LEVEL, (extra_1, extra_2), 'B B', 71, 'B 62c'))

    def single_read_extra_out_prefix_1_low_level(self):
        r"""

        """
        self.check_validity()

        return SingleReadExtraOutPrefix1LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SINGLE_READ_EXTRA_OUT_PREFIX_1_LOW_LEVEL, (), '', 72, 'B B 62c'))

    def single_read_extra_out_prefix_2_low_level(self):
        r"""

        """
        self.check_validity()

        return SingleReadExtraOutPrefix2LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SINGLE_READ_EXTRA_OUT_PREFIX_2_LOW_LEVEL, (), '', 72, 'B B B 61c'))

    def single_read_extra_out_suffix_1_low_level(self):
        r"""

        """
        self.check_validity()

        return SingleReadExtraOutSuffix1LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SINGLE_READ_EXTRA_OUT_SUFFIX_1_LOW_LEVEL, (), '', 72, 'B 62c B'))

    def single_read_extra_out_suffix_2_low_level(self):
        r"""

        """
        self.check_validity()

        return SingleReadExtraOutSuffix2LowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SINGLE_READ_EXTRA_OUT_SUFFIX_2_LOW_LEVEL, (), '', 72, 'B 61c B B'))

    def single_read_extra_out_full_low_level(self):
        r"""

        """
        self.check_validity()

        return SingleReadExtraOutFullLowLevel(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SINGLE_READ_EXTRA_OUT_FULL_LOW_LEVEL, (), '', 72, 'B B B 60c B'))

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletStreamTest.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def normal_write(self, message):
        r"""

        """
        message = create_char_list(message)

        if len(message) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 65535 items long')

        message_length = len(message)
        message_chunk_offset = 0

        if message_length == 0:
            message_chunk_data = ['\0'] * 60
            ret = self.normal_write_low_level(message_length, message_chunk_offset, message_chunk_data)
        else:
            with self.stream_lock:
                while message_chunk_offset < message_length:
                    message_chunk_data = create_chunk_data(message, message_chunk_offset, 60, '\0')
                    ret = self.normal_write_low_level(message_length, message_chunk_offset, message_chunk_data)
                    message_chunk_offset += 60

        return ret

    def normal_write_extra_in_prefix_1(self, extra, message):
        r"""

        """
        extra = int(extra)
        message = create_char_list(message)

        if len(message) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 65535 items long')

        message_length = len(message)
        message_chunk_offset = 0

        if message_length == 0:
            message_chunk_data = ['\0'] * 59
            ret = self.normal_write_extra_in_prefix_1_low_level(extra, message_length, message_chunk_offset, message_chunk_data)
        else:
            with self.stream_lock:
                while message_chunk_offset < message_length:
                    message_chunk_data = create_chunk_data(message, message_chunk_offset, 59, '\0')
                    ret = self.normal_write_extra_in_prefix_1_low_level(extra, message_length, message_chunk_offset, message_chunk_data)
                    message_chunk_offset += 59

        return ret

    def normal_write_extra_in_prefix_2(self, extra_1, extra_2, message):
        r"""

        """
        extra_1 = int(extra_1)
        extra_2 = int(extra_2)
        message = create_char_list(message)

        if len(message) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 65535 items long')

        message_length = len(message)
        message_chunk_offset = 0

        if message_length == 0:
            message_chunk_data = ['\0'] * 58
            ret = self.normal_write_extra_in_prefix_2_low_level(extra_1, extra_2, message_length, message_chunk_offset, message_chunk_data)
        else:
            with self.stream_lock:
                while message_chunk_offset < message_length:
                    message_chunk_data = create_chunk_data(message, message_chunk_offset, 58, '\0')
                    ret = self.normal_write_extra_in_prefix_2_low_level(extra_1, extra_2, message_length, message_chunk_offset, message_chunk_data)
                    message_chunk_offset += 58

        return ret

    def normal_write_extra_in_suffix_1(self, message, extra):
        r"""

        """
        message = create_char_list(message)
        extra = int(extra)

        if len(message) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 65535 items long')

        message_length = len(message)
        message_chunk_offset = 0

        if message_length == 0:
            message_chunk_data = ['\0'] * 59
            ret = self.normal_write_extra_in_suffix_1_low_level(message_length, message_chunk_offset, message_chunk_data, extra)
        else:
            with self.stream_lock:
                while message_chunk_offset < message_length:
                    message_chunk_data = create_chunk_data(message, message_chunk_offset, 59, '\0')
                    ret = self.normal_write_extra_in_suffix_1_low_level(message_length, message_chunk_offset, message_chunk_data, extra)
                    message_chunk_offset += 59

        return ret

    def normal_write_extra_in_suffix_2(self, message, extra_1, extra_2):
        r"""

        """
        message = create_char_list(message)
        extra_1 = int(extra_1)
        extra_2 = int(extra_2)

        if len(message) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 65535 items long')

        message_length = len(message)
        message_chunk_offset = 0

        if message_length == 0:
            message_chunk_data = ['\0'] * 58
            ret = self.normal_write_extra_in_suffix_2_low_level(message_length, message_chunk_offset, message_chunk_data, extra_1, extra_2)
        else:
            with self.stream_lock:
                while message_chunk_offset < message_length:
                    message_chunk_data = create_chunk_data(message, message_chunk_offset, 58, '\0')
                    ret = self.normal_write_extra_in_suffix_2_low_level(message_length, message_chunk_offset, message_chunk_data, extra_1, extra_2)
                    message_chunk_offset += 58

        return ret

    def normal_write_extra_in_full(self, extra_1, extra_2, extra_3, message, extra_4):
        r"""

        """
        extra_1 = int(extra_1)
        extra_2 = int(extra_2)
        extra_3 = int(extra_3)
        message = create_char_list(message)
        extra_4 = int(extra_4)

        if len(message) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 65535 items long')

        message_length = len(message)
        message_chunk_offset = 0

        if message_length == 0:
            message_chunk_data = ['\0'] * 56
            ret = self.normal_write_extra_in_full_low_level(extra_1, message_length, extra_2, message_chunk_offset, extra_3, message_chunk_data, extra_4)
        else:
            with self.stream_lock:
                while message_chunk_offset < message_length:
                    message_chunk_data = create_chunk_data(message, message_chunk_offset, 56, '\0')
                    ret = self.normal_write_extra_in_full_low_level(extra_1, message_length, extra_2, message_chunk_offset, extra_3, message_chunk_data, extra_4)
                    message_chunk_offset += 56

        return ret

    def normal_write_extra_out_1(self, message):
        r"""

        """
        message = create_char_list(message)

        if len(message) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 65535 items long')

        message_length = len(message)
        message_chunk_offset = 0

        if message_length == 0:
            message_chunk_data = ['\0'] * 60
            ret = self.normal_write_extra_out_1_low_level(message_length, message_chunk_offset, message_chunk_data)
        else:
            with self.stream_lock:
                while message_chunk_offset < message_length:
                    message_chunk_data = create_chunk_data(message, message_chunk_offset, 60, '\0')
                    ret = self.normal_write_extra_out_1_low_level(message_length, message_chunk_offset, message_chunk_data)
                    message_chunk_offset += 60

        return ret

    def normal_write_extra_out_2(self, message):
        r"""

        """
        message = create_char_list(message)

        if len(message) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 65535 items long')

        message_length = len(message)
        message_chunk_offset = 0

        if message_length == 0:
            message_chunk_data = ['\0'] * 60
            ret = self.normal_write_extra_out_2_low_level(message_length, message_chunk_offset, message_chunk_data)
        else:
            with self.stream_lock:
                while message_chunk_offset < message_length:
                    message_chunk_data = create_chunk_data(message, message_chunk_offset, 60, '\0')
                    ret = self.normal_write_extra_out_2_low_level(message_length, message_chunk_offset, message_chunk_data)
                    message_chunk_offset += 60

        return NormalWriteExtraOut2(*ret)

    def fixed_write(self, message):
        r"""

        """
        message = create_char_list(message)

        message_length = 1000
        message_chunk_offset = 0

        if len(message) != message_length:
            raise Error(Error.INVALID_PARAMETER, 'Message has to be exactly {0} items long'.format(message_length))

        with self.stream_lock:
            while message_chunk_offset < message_length:
                message_chunk_data = create_chunk_data(message, message_chunk_offset, 62, '\0')
                ret = self.fixed_write_low_level(message_chunk_offset, message_chunk_data)
                message_chunk_offset += 62

        return ret

    def fixed_write_extra_in_prefix_1(self, extra, message):
        r"""

        """
        extra = int(extra)
        message = create_char_list(message)

        message_length = 1000
        message_chunk_offset = 0

        if len(message) != message_length:
            raise Error(Error.INVALID_PARAMETER, 'Message has to be exactly {0} items long'.format(message_length))

        with self.stream_lock:
            while message_chunk_offset < message_length:
                message_chunk_data = create_chunk_data(message, message_chunk_offset, 61, '\0')
                ret = self.fixed_write_extra_in_prefix_1_low_level(extra, message_chunk_offset, message_chunk_data)
                message_chunk_offset += 61

        return ret

    def fixed_write_extra_in_prefix_2(self, extra_1, extra_2, message):
        r"""

        """
        extra_1 = int(extra_1)
        extra_2 = int(extra_2)
        message = create_char_list(message)

        message_length = 1000
        message_chunk_offset = 0

        if len(message) != message_length:
            raise Error(Error.INVALID_PARAMETER, 'Message has to be exactly {0} items long'.format(message_length))

        with self.stream_lock:
            while message_chunk_offset < message_length:
                message_chunk_data = create_chunk_data(message, message_chunk_offset, 60, '\0')
                ret = self.fixed_write_extra_in_prefix_2_low_level(extra_1, extra_2, message_chunk_offset, message_chunk_data)
                message_chunk_offset += 60

        return ret

    def fixed_write_extra_in_suffix_1(self, message, extra):
        r"""

        """
        message = create_char_list(message)
        extra = int(extra)

        message_length = 1000
        message_chunk_offset = 0

        if len(message) != message_length:
            raise Error(Error.INVALID_PARAMETER, 'Message has to be exactly {0} items long'.format(message_length))

        with self.stream_lock:
            while message_chunk_offset < message_length:
                message_chunk_data = create_chunk_data(message, message_chunk_offset, 61, '\0')
                ret = self.fixed_write_extra_in_suffix_1_low_level(message_chunk_offset, message_chunk_data, extra)
                message_chunk_offset += 61

        return ret

    def fixed_write_extra_in_suffix_2(self, message, extra_1, extra_2):
        r"""

        """
        message = create_char_list(message)
        extra_1 = int(extra_1)
        extra_2 = int(extra_2)

        message_length = 1000
        message_chunk_offset = 0

        if len(message) != message_length:
            raise Error(Error.INVALID_PARAMETER, 'Message has to be exactly {0} items long'.format(message_length))

        with self.stream_lock:
            while message_chunk_offset < message_length:
                message_chunk_data = create_chunk_data(message, message_chunk_offset, 60, '\0')
                ret = self.fixed_write_extra_in_suffix_2_low_level(message_chunk_offset, message_chunk_data, extra_1, extra_2)
                message_chunk_offset += 60

        return ret

    def fixed_write_extra_in_full(self, extra_1, extra_2, message, extra_3):
        r"""

        """
        extra_1 = int(extra_1)
        extra_2 = int(extra_2)
        message = create_char_list(message)
        extra_3 = int(extra_3)

        message_length = 1000
        message_chunk_offset = 0

        if len(message) != message_length:
            raise Error(Error.INVALID_PARAMETER, 'Message has to be exactly {0} items long'.format(message_length))

        with self.stream_lock:
            while message_chunk_offset < message_length:
                message_chunk_data = create_chunk_data(message, message_chunk_offset, 59, '\0')
                ret = self.fixed_write_extra_in_full_low_level(extra_1, message_chunk_offset, extra_2, message_chunk_data, extra_3)
                message_chunk_offset += 59

        return ret

    def fixed_write_extra_out_1(self, message):
        r"""

        """
        message = create_char_list(message)

        message_length = 1000
        message_chunk_offset = 0

        if len(message) != message_length:
            raise Error(Error.INVALID_PARAMETER, 'Message has to be exactly {0} items long'.format(message_length))

        with self.stream_lock:
            while message_chunk_offset < message_length:
                message_chunk_data = create_chunk_data(message, message_chunk_offset, 62, '\0')
                ret = self.fixed_write_extra_out_1_low_level(message_chunk_offset, message_chunk_data)
                message_chunk_offset += 62

        return ret

    def fixed_write_extra_out_2(self, message):
        r"""

        """
        message = create_char_list(message)

        message_length = 1000
        message_chunk_offset = 0

        if len(message) != message_length:
            raise Error(Error.INVALID_PARAMETER, 'Message has to be exactly {0} items long'.format(message_length))

        with self.stream_lock:
            while message_chunk_offset < message_length:
                message_chunk_data = create_chunk_data(message, message_chunk_offset, 62, '\0')
                ret = self.fixed_write_extra_out_2_low_level(message_chunk_offset, message_chunk_data)
                message_chunk_offset += 62

        return FixedWriteExtraOut2(*ret)

    def short_write(self, message):
        r"""

        """
        message = create_char_list(message)

        if len(message) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 65535 items long')

        message_length = len(message)
        message_chunk_offset = 0

        if message_length == 0:
            message_chunk_data = ['\0'] * 60
            ret = self.short_write_low_level(message_length, message_chunk_offset, message_chunk_data)
            message_written = ret
        else:
            message_written = 0

            with self.stream_lock:
                while message_chunk_offset < message_length:
                    message_chunk_data = create_chunk_data(message, message_chunk_offset, 60, '\0')
                    ret = self.short_write_low_level(message_length, message_chunk_offset, message_chunk_data)
                    message_written += ret

                    if ret < 60:
                        break # either last chunk or short write

                    message_chunk_offset += 60

        return message_written

    def short_write_extra_in_prefix_1(self, extra, message):
        r"""

        """
        extra = int(extra)
        message = create_char_list(message)

        if len(message) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 65535 items long')

        message_length = len(message)
        message_chunk_offset = 0

        if message_length == 0:
            message_chunk_data = ['\0'] * 59
            ret = self.short_write_extra_in_prefix_1_low_level(extra, message_length, message_chunk_offset, message_chunk_data)
            message_written = ret
        else:
            message_written = 0

            with self.stream_lock:
                while message_chunk_offset < message_length:
                    message_chunk_data = create_chunk_data(message, message_chunk_offset, 59, '\0')
                    ret = self.short_write_extra_in_prefix_1_low_level(extra, message_length, message_chunk_offset, message_chunk_data)
                    message_written += ret

                    if ret < 59:
                        break # either last chunk or short write

                    message_chunk_offset += 59

        return message_written

    def short_write_extra_in_prefix_2(self, extra_1, extra_2, message):
        r"""

        """
        extra_1 = int(extra_1)
        extra_2 = int(extra_2)
        message = create_char_list(message)

        if len(message) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 65535 items long')

        message_length = len(message)
        message_chunk_offset = 0

        if message_length == 0:
            message_chunk_data = ['\0'] * 58
            ret = self.short_write_extra_in_prefix_2_low_level(extra_1, extra_2, message_length, message_chunk_offset, message_chunk_data)
            message_written = ret
        else:
            message_written = 0

            with self.stream_lock:
                while message_chunk_offset < message_length:
                    message_chunk_data = create_chunk_data(message, message_chunk_offset, 58, '\0')
                    ret = self.short_write_extra_in_prefix_2_low_level(extra_1, extra_2, message_length, message_chunk_offset, message_chunk_data)
                    message_written += ret

                    if ret < 58:
                        break # either last chunk or short write

                    message_chunk_offset += 58

        return message_written

    def short_write_extra_in_suffix_1(self, message, extra):
        r"""

        """
        message = create_char_list(message)
        extra = int(extra)

        if len(message) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 65535 items long')

        message_length = len(message)
        message_chunk_offset = 0

        if message_length == 0:
            message_chunk_data = ['\0'] * 59
            ret = self.short_write_extra_in_suffix_1_low_level(message_length, message_chunk_offset, message_chunk_data, extra)
            message_written = ret
        else:
            message_written = 0

            with self.stream_lock:
                while message_chunk_offset < message_length:
                    message_chunk_data = create_chunk_data(message, message_chunk_offset, 59, '\0')
                    ret = self.short_write_extra_in_suffix_1_low_level(message_length, message_chunk_offset, message_chunk_data, extra)
                    message_written += ret

                    if ret < 59:
                        break # either last chunk or short write

                    message_chunk_offset += 59

        return message_written

    def short_write_extra_in_suffix_2(self, message, extra_1, extra_2):
        r"""

        """
        message = create_char_list(message)
        extra_1 = int(extra_1)
        extra_2 = int(extra_2)

        if len(message) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 65535 items long')

        message_length = len(message)
        message_chunk_offset = 0

        if message_length == 0:
            message_chunk_data = ['\0'] * 58
            ret = self.short_write_extra_in_suffix_2_low_level(message_length, message_chunk_offset, message_chunk_data, extra_1, extra_2)
            message_written = ret
        else:
            message_written = 0

            with self.stream_lock:
                while message_chunk_offset < message_length:
                    message_chunk_data = create_chunk_data(message, message_chunk_offset, 58, '\0')
                    ret = self.short_write_extra_in_suffix_2_low_level(message_length, message_chunk_offset, message_chunk_data, extra_1, extra_2)
                    message_written += ret

                    if ret < 58:
                        break # either last chunk or short write

                    message_chunk_offset += 58

        return message_written

    def short_write_extra_out_prefix_1(self, message):
        r"""

        """
        message = create_char_list(message)

        if len(message) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 65535 items long')

        message_length = len(message)
        message_chunk_offset = 0

        if message_length == 0:
            message_chunk_data = ['\0'] * 60
            ret = self.short_write_extra_out_prefix_1_low_level(message_length, message_chunk_offset, message_chunk_data)
            message_written = ret.message_chunk_written
        else:
            message_written = 0

            with self.stream_lock:
                while message_chunk_offset < message_length:
                    message_chunk_data = create_chunk_data(message, message_chunk_offset, 60, '\0')
                    ret = self.short_write_extra_out_prefix_1_low_level(message_length, message_chunk_offset, message_chunk_data)
                    message_written += ret.message_chunk_written

                    if ret.message_chunk_written < 60:
                        break # either last chunk or short write

                    message_chunk_offset += 60

        return ShortWriteExtraOutPrefix1(ret.extra, message_written)

    def short_write_extra_out_prefix_2(self, message):
        r"""

        """
        message = create_char_list(message)

        if len(message) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 65535 items long')

        message_length = len(message)
        message_chunk_offset = 0

        if message_length == 0:
            message_chunk_data = ['\0'] * 60
            ret = self.short_write_extra_out_prefix_2_low_level(message_length, message_chunk_offset, message_chunk_data)
            message_written = ret.message_chunk_written
        else:
            message_written = 0

            with self.stream_lock:
                while message_chunk_offset < message_length:
                    message_chunk_data = create_chunk_data(message, message_chunk_offset, 60, '\0')
                    ret = self.short_write_extra_out_prefix_2_low_level(message_length, message_chunk_offset, message_chunk_data)
                    message_written += ret.message_chunk_written

                    if ret.message_chunk_written < 60:
                        break # either last chunk or short write

                    message_chunk_offset += 60

        return ShortWriteExtraOutPrefix2(ret.extra_1, ret.extra_2, message_written)

    def short_write_extra_out_suffix_1(self, message):
        r"""

        """
        message = create_char_list(message)

        if len(message) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 65535 items long')

        message_length = len(message)
        message_chunk_offset = 0

        if message_length == 0:
            message_chunk_data = ['\0'] * 60
            ret = self.short_write_extra_out_suffix_1_low_level(message_length, message_chunk_offset, message_chunk_data)
            message_written = ret.message_chunk_written
        else:
            message_written = 0

            with self.stream_lock:
                while message_chunk_offset < message_length:
                    message_chunk_data = create_chunk_data(message, message_chunk_offset, 60, '\0')
                    ret = self.short_write_extra_out_suffix_1_low_level(message_length, message_chunk_offset, message_chunk_data)
                    message_written += ret.message_chunk_written

                    if ret.message_chunk_written < 60:
                        break # either last chunk or short write

                    message_chunk_offset += 60

        return ShortWriteExtraOutSuffix1(message_written, ret.extra)

    def short_write_extra_out_suffix_2(self, message):
        r"""

        """
        message = create_char_list(message)

        if len(message) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 65535 items long')

        message_length = len(message)
        message_chunk_offset = 0

        if message_length == 0:
            message_chunk_data = ['\0'] * 60
            ret = self.short_write_extra_out_suffix_2_low_level(message_length, message_chunk_offset, message_chunk_data)
            message_written = ret.message_chunk_written
        else:
            message_written = 0

            with self.stream_lock:
                while message_chunk_offset < message_length:
                    message_chunk_data = create_chunk_data(message, message_chunk_offset, 60, '\0')
                    ret = self.short_write_extra_out_suffix_2_low_level(message_length, message_chunk_offset, message_chunk_data)
                    message_written += ret.message_chunk_written

                    if ret.message_chunk_written < 60:
                        break # either last chunk or short write

                    message_chunk_offset += 60

        return ShortWriteExtraOutSuffix2(message_written, ret.extra_1, ret.extra_2)

    def short_write_extra_full(self, extra_1, extra_2, extra_3, message, extra_4):
        r"""

        """
        extra_1 = int(extra_1)
        extra_2 = int(extra_2)
        extra_3 = int(extra_3)
        message = create_char_list(message)
        extra_4 = int(extra_4)

        if len(message) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 65535 items long')

        message_length = len(message)
        message_chunk_offset = 0

        if message_length == 0:
            message_chunk_data = ['\0'] * 56
            ret = self.short_write_extra_full_low_level(extra_1, message_length, extra_2, message_chunk_offset, extra_3, message_chunk_data, extra_4)
            message_written = ret.message_chunk_written
        else:
            message_written = 0

            with self.stream_lock:
                while message_chunk_offset < message_length:
                    message_chunk_data = create_chunk_data(message, message_chunk_offset, 56, '\0')
                    ret = self.short_write_extra_full_low_level(extra_1, message_length, extra_2, message_chunk_offset, extra_3, message_chunk_data, extra_4)
                    message_written += ret.message_chunk_written

                    if ret.message_chunk_written < 56:
                        break # either last chunk or short write

                    message_chunk_offset += 56

        return ShortWriteExtraFull(ret.extra_5, message_written, ret.extra_6)

    def short_write_bool(self, message):
        r"""

        """
        message = list(map(bool, message))

        if len(message) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 65535 items long')

        message_length = len(message)
        message_chunk_offset = 0

        if message_length == 0:
            message_chunk_data = [False] * 480
            ret = self.short_write_bool_low_level(message_length, message_chunk_offset, message_chunk_data)
            message_written = ret
        else:
            message_written = 0

            with self.stream_lock:
                while message_chunk_offset < message_length:
                    message_chunk_data = create_chunk_data(message, message_chunk_offset, 480, False)
                    ret = self.short_write_bool_low_level(message_length, message_chunk_offset, message_chunk_data)
                    message_written += ret

                    if ret < 480:
                        break # either last chunk or short write

                    message_chunk_offset += 480

        return message_written

    def single_write(self, message):
        r"""

        """
        message = create_char_list(message)

        message_length = len(message)
        message_data = list(message) # make a copy so we can potentially extend it

        if message_length > 63:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 63 items long')

        if message_length < 63:
            message_data += ['\0'] * (63 - message_length)

        return self.single_write_low_level(message_length, message_data)

    def single_write_extra_in_prefix_1(self, extra, message):
        r"""

        """
        extra = int(extra)
        message = create_char_list(message)

        message_length = len(message)
        message_data = list(message) # make a copy so we can potentially extend it

        if message_length > 62:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 62 items long')

        if message_length < 62:
            message_data += ['\0'] * (62 - message_length)

        return self.single_write_extra_in_prefix_1_low_level(extra, message_length, message_data)

    def single_write_extra_in_prefix_2(self, extra_1, extra_2, message):
        r"""

        """
        extra_1 = int(extra_1)
        extra_2 = int(extra_2)
        message = create_char_list(message)

        message_length = len(message)
        message_data = list(message) # make a copy so we can potentially extend it

        if message_length > 61:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 61 items long')

        if message_length < 61:
            message_data += ['\0'] * (61 - message_length)

        return self.single_write_extra_in_prefix_2_low_level(extra_1, extra_2, message_length, message_data)

    def single_write_extra_in_suffix_1(self, message, extra):
        r"""

        """
        message = create_char_list(message)
        extra = int(extra)

        message_length = len(message)
        message_data = list(message) # make a copy so we can potentially extend it

        if message_length > 62:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 62 items long')

        if message_length < 62:
            message_data += ['\0'] * (62 - message_length)

        return self.single_write_extra_in_suffix_1_low_level(message_length, message_data, extra)

    def single_write_extra_in_suffix_2(self, message, extra_1, extra_2):
        r"""

        """
        message = create_char_list(message)
        extra_1 = int(extra_1)
        extra_2 = int(extra_2)

        message_length = len(message)
        message_data = list(message) # make a copy so we can potentially extend it

        if message_length > 61:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 61 items long')

        if message_length < 61:
            message_data += ['\0'] * (61 - message_length)

        return self.single_write_extra_in_suffix_2_low_level(message_length, message_data, extra_1, extra_2)

    def single_write_extra_in_full(self, extra_1, extra_2, message, extra_3):
        r"""

        """
        extra_1 = int(extra_1)
        extra_2 = int(extra_2)
        message = create_char_list(message)
        extra_3 = int(extra_3)

        message_length = len(message)
        message_data = list(message) # make a copy so we can potentially extend it

        if message_length > 60:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 60 items long')

        if message_length < 60:
            message_data += ['\0'] * (60 - message_length)

        return self.single_write_extra_in_full_low_level(extra_1, message_length, extra_2, message_data, extra_3)

    def single_write_extra_out_1(self, message):
        r"""

        """
        message = create_char_list(message)

        message_length = len(message)
        message_data = list(message) # make a copy so we can potentially extend it

        if message_length > 63:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 63 items long')

        if message_length < 63:
            message_data += ['\0'] * (63 - message_length)

        return self.single_write_extra_out_1_low_level(message_length, message_data)

    def single_write_extra_out_2(self, message):
        r"""

        """
        message = create_char_list(message)

        message_length = len(message)
        message_data = list(message) # make a copy so we can potentially extend it

        if message_length > 63:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 63 items long')

        if message_length < 63:
            message_data += ['\0'] * (63 - message_length)

        return SingleWriteExtraOut2(*self.single_write_extra_out_2_low_level(message_length, message_data))

    def short_single_write(self, message):
        r"""

        """
        message = create_char_list(message)

        message_length = len(message)
        message_data = list(message) # make a copy so we can potentially extend it

        if message_length > 63:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 63 items long')

        if message_length < 63:
            message_data += ['\0'] * (63 - message_length)

        return self.short_single_write_low_level(message_length, message_data)

    def short_single_write_extra_in_prefix_1(self, extra, message):
        r"""

        """
        extra = int(extra)
        message = create_char_list(message)

        message_length = len(message)
        message_data = list(message) # make a copy so we can potentially extend it

        if message_length > 62:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 62 items long')

        if message_length < 62:
            message_data += ['\0'] * (62 - message_length)

        return self.short_single_write_extra_in_prefix_1_low_level(extra, message_length, message_data)

    def short_single_write_extra_in_prefix_2(self, extra_1, extra_2, message):
        r"""

        """
        extra_1 = int(extra_1)
        extra_2 = int(extra_2)
        message = create_char_list(message)

        message_length = len(message)
        message_data = list(message) # make a copy so we can potentially extend it

        if message_length > 61:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 61 items long')

        if message_length < 61:
            message_data += ['\0'] * (61 - message_length)

        return self.short_single_write_extra_in_prefix_2_low_level(extra_1, extra_2, message_length, message_data)

    def short_single_write_extra_in_suffix_1(self, message, extra):
        r"""

        """
        message = create_char_list(message)
        extra = int(extra)

        message_length = len(message)
        message_data = list(message) # make a copy so we can potentially extend it

        if message_length > 62:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 62 items long')

        if message_length < 62:
            message_data += ['\0'] * (62 - message_length)

        return self.short_single_write_extra_in_suffix_1_low_level(message_length, message_data, extra)

    def short_single_write_extra_in_suffix_2(self, message, extra_1, extra_2):
        r"""

        """
        message = create_char_list(message)
        extra_1 = int(extra_1)
        extra_2 = int(extra_2)

        message_length = len(message)
        message_data = list(message) # make a copy so we can potentially extend it

        if message_length > 61:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 61 items long')

        if message_length < 61:
            message_data += ['\0'] * (61 - message_length)

        return self.short_single_write_extra_in_suffix_2_low_level(message_length, message_data, extra_1, extra_2)

    def short_single_write_extra_out_prefix_1(self, message):
        r"""

        """
        message = create_char_list(message)

        message_length = len(message)
        message_data = list(message) # make a copy so we can potentially extend it

        if message_length > 62:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 62 items long')

        if message_length < 62:
            message_data += ['\0'] * (62 - message_length)

        return ShortSingleWriteExtraOutPrefix1(*self.short_single_write_extra_out_prefix_1_low_level(message_length, message_data))

    def short_single_write_extra_out_prefix_2(self, message):
        r"""

        """
        message = create_char_list(message)

        message_length = len(message)
        message_data = list(message) # make a copy so we can potentially extend it

        if message_length > 62:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 62 items long')

        if message_length < 62:
            message_data += ['\0'] * (62 - message_length)

        return ShortSingleWriteExtraOutPrefix2(*self.short_single_write_extra_out_prefix_2_low_level(message_length, message_data))

    def short_single_write_extra_out_suffix_1(self, message):
        r"""

        """
        message = create_char_list(message)

        message_length = len(message)
        message_data = list(message) # make a copy so we can potentially extend it

        if message_length > 63:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 63 items long')

        if message_length < 63:
            message_data += ['\0'] * (63 - message_length)

        return ShortSingleWriteExtraOutSuffix1(*self.short_single_write_extra_out_suffix_1_low_level(message_length, message_data))

    def short_single_write_extra_out_suffix_2(self, message):
        r"""

        """
        message = create_char_list(message)

        message_length = len(message)
        message_data = list(message) # make a copy so we can potentially extend it

        if message_length > 63:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 63 items long')

        if message_length < 63:
            message_data += ['\0'] * (63 - message_length)

        return ShortSingleWriteExtraOutSuffix2(*self.short_single_write_extra_out_suffix_2_low_level(message_length, message_data))

    def short_single_write_extra_full(self, extra_1, extra_2, message, extra_3):
        r"""

        """
        extra_1 = int(extra_1)
        extra_2 = int(extra_2)
        message = create_char_list(message)
        extra_3 = int(extra_3)

        message_length = len(message)
        message_data = list(message) # make a copy so we can potentially extend it

        if message_length > 60:
            raise Error(Error.INVALID_PARAMETER, 'Message can be at most 60 items long')

        if message_length < 60:
            message_data += ['\0'] * (60 - message_length)

        return ShortSingleWriteExtraFull(*self.short_single_write_extra_full_low_level(extra_1, message_length, extra_2, message_data, extra_3))

    def normal_read(self):
        r"""

        """
        with self.stream_lock:
            ret = self.normal_read_low_level()
            message_length = ret.message_length
            message_out_of_sync = ret.message_chunk_offset != 0
            message_data = ret.message_chunk_data

            while not message_out_of_sync and len(message_data) < message_length:
                ret = self.normal_read_low_level()
                message_length = ret.message_length
                message_out_of_sync = ret.message_chunk_offset != len(message_data)
                message_data += ret.message_chunk_data

            if message_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.message_chunk_offset + 60 < message_length:
                    ret = self.normal_read_low_level()
                    message_length = ret.message_length

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Message stream is out-of-sync')

        return message_data[:message_length]

    def normal_read_extra_in_1(self, extra):
        r"""

        """
        extra = int(extra)

        with self.stream_lock:
            ret = self.normal_read_extra_in_1_low_level(extra)
            message_length = ret.message_length
            message_out_of_sync = ret.message_chunk_offset != 0
            message_data = ret.message_chunk_data

            while not message_out_of_sync and len(message_data) < message_length:
                ret = self.normal_read_extra_in_1_low_level(extra)
                message_length = ret.message_length
                message_out_of_sync = ret.message_chunk_offset != len(message_data)
                message_data += ret.message_chunk_data

            if message_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.message_chunk_offset + 60 < message_length:
                    ret = self.normal_read_extra_in_1_low_level(extra)
                    message_length = ret.message_length

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Message stream is out-of-sync')

        return message_data[:message_length]

    def normal_read_extra_in_2(self, extra_1, extra_2):
        r"""

        """
        extra_1 = int(extra_1)
        extra_2 = int(extra_2)

        with self.stream_lock:
            ret = self.normal_read_extra_in_2_low_level(extra_1, extra_2)
            message_length = ret.message_length
            message_out_of_sync = ret.message_chunk_offset != 0
            message_data = ret.message_chunk_data

            while not message_out_of_sync and len(message_data) < message_length:
                ret = self.normal_read_extra_in_2_low_level(extra_1, extra_2)
                message_length = ret.message_length
                message_out_of_sync = ret.message_chunk_offset != len(message_data)
                message_data += ret.message_chunk_data

            if message_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.message_chunk_offset + 60 < message_length:
                    ret = self.normal_read_extra_in_2_low_level(extra_1, extra_2)
                    message_length = ret.message_length

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Message stream is out-of-sync')

        return message_data[:message_length]

    def normal_read_extra_out_prefix_1(self):
        r"""

        """
        with self.stream_lock:
            ret = self.normal_read_extra_out_prefix_1_low_level()
            message_length = ret.message_length
            message_out_of_sync = ret.message_chunk_offset != 0
            message_data = ret.message_chunk_data

            while not message_out_of_sync and len(message_data) < message_length:
                ret = self.normal_read_extra_out_prefix_1_low_level()
                message_length = ret.message_length
                message_out_of_sync = ret.message_chunk_offset != len(message_data)
                message_data += ret.message_chunk_data

            if message_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.message_chunk_offset + 59 < message_length:
                    ret = self.normal_read_extra_out_prefix_1_low_level()
                    message_length = ret.message_length

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Message stream is out-of-sync')

        return NormalReadExtraOutPrefix1(ret.extra, message_data[:message_length])

    def normal_read_extra_out_prefix_2(self):
        r"""

        """
        with self.stream_lock:
            ret = self.normal_read_extra_out_prefix_2_low_level()
            message_length = ret.message_length
            message_out_of_sync = ret.message_chunk_offset != 0
            message_data = ret.message_chunk_data

            while not message_out_of_sync and len(message_data) < message_length:
                ret = self.normal_read_extra_out_prefix_2_low_level()
                message_length = ret.message_length
                message_out_of_sync = ret.message_chunk_offset != len(message_data)
                message_data += ret.message_chunk_data

            if message_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.message_chunk_offset + 58 < message_length:
                    ret = self.normal_read_extra_out_prefix_2_low_level()
                    message_length = ret.message_length

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Message stream is out-of-sync')

        return NormalReadExtraOutPrefix2(ret.extra_1, ret.extra_2, message_data[:message_length])

    def normal_read_extra_out_suffix_1(self):
        r"""

        """
        with self.stream_lock:
            ret = self.normal_read_extra_out_suffix_1_low_level()
            message_length = ret.message_length
            message_out_of_sync = ret.message_chunk_offset != 0
            message_data = ret.message_chunk_data

            while not message_out_of_sync and len(message_data) < message_length:
                ret = self.normal_read_extra_out_suffix_1_low_level()
                message_length = ret.message_length
                message_out_of_sync = ret.message_chunk_offset != len(message_data)
                message_data += ret.message_chunk_data

            if message_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.message_chunk_offset + 59 < message_length:
                    ret = self.normal_read_extra_out_suffix_1_low_level()
                    message_length = ret.message_length

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Message stream is out-of-sync')

        return NormalReadExtraOutSuffix1(message_data[:message_length], ret.extra)

    def normal_read_extra_out_suffix_2(self):
        r"""

        """
        with self.stream_lock:
            ret = self.normal_read_extra_out_suffix_2_low_level()
            message_length = ret.message_length
            message_out_of_sync = ret.message_chunk_offset != 0
            message_data = ret.message_chunk_data

            while not message_out_of_sync and len(message_data) < message_length:
                ret = self.normal_read_extra_out_suffix_2_low_level()
                message_length = ret.message_length
                message_out_of_sync = ret.message_chunk_offset != len(message_data)
                message_data += ret.message_chunk_data

            if message_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.message_chunk_offset + 58 < message_length:
                    ret = self.normal_read_extra_out_suffix_2_low_level()
                    message_length = ret.message_length

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Message stream is out-of-sync')

        return NormalReadExtraOutSuffix2(message_data[:message_length], ret.extra_1, ret.extra_2)

    def normal_read_extra_out_full(self):
        r"""

        """
        with self.stream_lock:
            ret = self.normal_read_extra_out_full_low_level()
            message_length = ret.message_length
            message_out_of_sync = ret.message_chunk_offset != 0
            message_data = ret.message_chunk_data

            while not message_out_of_sync and len(message_data) < message_length:
                ret = self.normal_read_extra_out_full_low_level()
                message_length = ret.message_length
                message_out_of_sync = ret.message_chunk_offset != len(message_data)
                message_data += ret.message_chunk_data

            if message_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.message_chunk_offset + 56 < message_length:
                    ret = self.normal_read_extra_out_full_low_level()
                    message_length = ret.message_length

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Message stream is out-of-sync')

        return NormalReadExtraOutFull(ret.extra_1, ret.extra_2, ret.extra_3, message_data[:message_length], ret.extra_4)

    def fixed_read(self):
        r"""

        """
        message_length = 1000

        with self.stream_lock:
            ret = self.fixed_read_low_level()

            if ret.message_chunk_offset == (1 << 16) - 1: # maximum chunk offset -> stream has no data
                message_length = 0
                message_out_of_sync = False
                message_data = ()
            else:
                message_out_of_sync = ret.message_chunk_offset != 0
                message_data = ret.message_chunk_data

            while not message_out_of_sync and len(message_data) < message_length:
                ret = self.fixed_read_low_level()
                message_out_of_sync = ret.message_chunk_offset != len(message_data)
                message_data += ret.message_chunk_data

            if message_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.message_chunk_offset + 62 < message_length:
                    ret = self.fixed_read_low_level()

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Message stream is out-of-sync')

        return message_data[:message_length]

    def fixed_read_extra_in_1(self, extra):
        r"""

        """
        extra = int(extra)

        message_length = 1000

        with self.stream_lock:
            ret = self.fixed_read_extra_in_1_low_level(extra)

            if ret.message_chunk_offset == (1 << 16) - 1: # maximum chunk offset -> stream has no data
                message_length = 0
                message_out_of_sync = False
                message_data = ()
            else:
                message_out_of_sync = ret.message_chunk_offset != 0
                message_data = ret.message_chunk_data

            while not message_out_of_sync and len(message_data) < message_length:
                ret = self.fixed_read_extra_in_1_low_level(extra)
                message_out_of_sync = ret.message_chunk_offset != len(message_data)
                message_data += ret.message_chunk_data

            if message_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.message_chunk_offset + 62 < message_length:
                    ret = self.fixed_read_extra_in_1_low_level(extra)

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Message stream is out-of-sync')

        return message_data[:message_length]

    def fixed_read_extra_in_2(self, extra_1, extra_2):
        r"""

        """
        extra_1 = int(extra_1)
        extra_2 = int(extra_2)

        message_length = 1000

        with self.stream_lock:
            ret = self.fixed_read_extra_in_2_low_level(extra_1, extra_2)

            if ret.message_chunk_offset == (1 << 16) - 1: # maximum chunk offset -> stream has no data
                message_length = 0
                message_out_of_sync = False
                message_data = ()
            else:
                message_out_of_sync = ret.message_chunk_offset != 0
                message_data = ret.message_chunk_data

            while not message_out_of_sync and len(message_data) < message_length:
                ret = self.fixed_read_extra_in_2_low_level(extra_1, extra_2)
                message_out_of_sync = ret.message_chunk_offset != len(message_data)
                message_data += ret.message_chunk_data

            if message_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.message_chunk_offset + 62 < message_length:
                    ret = self.fixed_read_extra_in_2_low_level(extra_1, extra_2)

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Message stream is out-of-sync')

        return message_data[:message_length]

    def fixed_read_extra_out_prefix_1(self):
        r"""

        """
        message_length = 1000

        with self.stream_lock:
            ret = self.fixed_read_extra_out_prefix_1_low_level()

            if ret.message_chunk_offset == (1 << 16) - 1: # maximum chunk offset -> stream has no data
                message_length = 0
                message_out_of_sync = False
                message_data = ()
            else:
                message_out_of_sync = ret.message_chunk_offset != 0
                message_data = ret.message_chunk_data

            while not message_out_of_sync and len(message_data) < message_length:
                ret = self.fixed_read_extra_out_prefix_1_low_level()
                message_out_of_sync = ret.message_chunk_offset != len(message_data)
                message_data += ret.message_chunk_data

            if message_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.message_chunk_offset + 61 < message_length:
                    ret = self.fixed_read_extra_out_prefix_1_low_level()

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Message stream is out-of-sync')

        return FixedReadExtraOutPrefix1(ret.extra, message_data[:message_length])

    def fixed_read_extra_out_prefix_2(self):
        r"""

        """
        message_length = 1000

        with self.stream_lock:
            ret = self.fixed_read_extra_out_prefix_2_low_level()

            if ret.message_chunk_offset == (1 << 16) - 1: # maximum chunk offset -> stream has no data
                message_length = 0
                message_out_of_sync = False
                message_data = ()
            else:
                message_out_of_sync = ret.message_chunk_offset != 0
                message_data = ret.message_chunk_data

            while not message_out_of_sync and len(message_data) < message_length:
                ret = self.fixed_read_extra_out_prefix_2_low_level()
                message_out_of_sync = ret.message_chunk_offset != len(message_data)
                message_data += ret.message_chunk_data

            if message_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.message_chunk_offset + 60 < message_length:
                    ret = self.fixed_read_extra_out_prefix_2_low_level()

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Message stream is out-of-sync')

        return FixedReadExtraOutPrefix2(ret.extra_1, ret.extra_2, message_data[:message_length])

    def fixed_read_extra_out_suffix_1(self):
        r"""

        """
        message_length = 1000

        with self.stream_lock:
            ret = self.fixed_read_extra_out_suffix_1_low_level()

            if ret.message_chunk_offset == (1 << 16) - 1: # maximum chunk offset -> stream has no data
                message_length = 0
                message_out_of_sync = False
                message_data = ()
            else:
                message_out_of_sync = ret.message_chunk_offset != 0
                message_data = ret.message_chunk_data

            while not message_out_of_sync and len(message_data) < message_length:
                ret = self.fixed_read_extra_out_suffix_1_low_level()
                message_out_of_sync = ret.message_chunk_offset != len(message_data)
                message_data += ret.message_chunk_data

            if message_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.message_chunk_offset + 61 < message_length:
                    ret = self.fixed_read_extra_out_suffix_1_low_level()

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Message stream is out-of-sync')

        return FixedReadExtraOutSuffix1(message_data[:message_length], ret.extra)

    def fixed_read_extra_out_suffix_2(self):
        r"""

        """
        message_length = 1000

        with self.stream_lock:
            ret = self.fixed_read_extra_out_suffix_2_low_level()

            if ret.message_chunk_offset == (1 << 16) - 1: # maximum chunk offset -> stream has no data
                message_length = 0
                message_out_of_sync = False
                message_data = ()
            else:
                message_out_of_sync = ret.message_chunk_offset != 0
                message_data = ret.message_chunk_data

            while not message_out_of_sync and len(message_data) < message_length:
                ret = self.fixed_read_extra_out_suffix_2_low_level()
                message_out_of_sync = ret.message_chunk_offset != len(message_data)
                message_data += ret.message_chunk_data

            if message_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.message_chunk_offset + 60 < message_length:
                    ret = self.fixed_read_extra_out_suffix_2_low_level()

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Message stream is out-of-sync')

        return FixedReadExtraOutSuffix2(message_data[:message_length], ret.extra_1, ret.extra_2)

    def fixed_read_extra_out_full(self):
        r"""

        """
        message_length = 1000

        with self.stream_lock:
            ret = self.fixed_read_extra_out_full_low_level()

            if ret.message_chunk_offset == (1 << 16) - 1: # maximum chunk offset -> stream has no data
                message_length = 0
                message_out_of_sync = False
                message_data = ()
            else:
                message_out_of_sync = ret.message_chunk_offset != 0
                message_data = ret.message_chunk_data

            while not message_out_of_sync and len(message_data) < message_length:
                ret = self.fixed_read_extra_out_full_low_level()
                message_out_of_sync = ret.message_chunk_offset != len(message_data)
                message_data += ret.message_chunk_data

            if message_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.message_chunk_offset + 59 < message_length:
                    ret = self.fixed_read_extra_out_full_low_level()

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Message stream is out-of-sync')

        return FixedReadExtraOutFull(ret.extra_1, ret.extra_2, message_data[:message_length], ret.extra_3)

    def single_read(self):
        r"""

        """
        ret = self.single_read_low_level()

        return ret.message_data[:ret.message_length]

    def single_read_extra_in_1(self, extra):
        r"""

        """
        extra = int(extra)

        ret = self.single_read_extra_in_1_low_level(extra)

        return ret.message_data[:ret.message_length]

    def single_read_extra_in_2(self, extra_1, extra_2):
        r"""

        """
        extra_1 = int(extra_1)
        extra_2 = int(extra_2)

        ret = self.single_read_extra_in_2_low_level(extra_1, extra_2)

        return ret.message_data[:ret.message_length]

    def single_read_extra_out_prefix_1(self):
        r"""

        """
        ret = self.single_read_extra_out_prefix_1_low_level()

        return SingleReadExtraOutPrefix1(ret.extra, ret.message_data[:ret.message_length])

    def single_read_extra_out_prefix_2(self):
        r"""

        """
        ret = self.single_read_extra_out_prefix_2_low_level()

        return SingleReadExtraOutPrefix2(ret.extra_1, ret.extra_2, ret.message_data[:ret.message_length])

    def single_read_extra_out_suffix_1(self):
        r"""

        """
        ret = self.single_read_extra_out_suffix_1_low_level()

        return SingleReadExtraOutSuffix1(ret.message_data[:ret.message_length], ret.extra)

    def single_read_extra_out_suffix_2(self):
        r"""

        """
        ret = self.single_read_extra_out_suffix_2_low_level()

        return SingleReadExtraOutSuffix2(ret.message_data[:ret.message_length], ret.extra_1, ret.extra_2)

    def single_read_extra_out_full(self):
        r"""

        """
        ret = self.single_read_extra_out_full_low_level()

        return SingleReadExtraOutFull(ret.extra_1, ret.extra_2, ret.message_data[:ret.message_length], ret.extra_3)

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

StreamTest = BrickletStreamTest # for backward compatibility
