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

GetValues = namedtuple('Values', ['timestamp', 'values'])
GetSelectedValue = namedtuple('SelectedValue', ['timestamp', 'value'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class TNGDO8(Device):
    r"""
    TBD
    """

    DEVICE_IDENTIFIER = 205
    DEVICE_DISPLAY_NAME = 'TNG DO8'
    DEVICE_URL_PART = 'do8' # internal



    FUNCTION_SET_VALUES = 1
    FUNCTION_GET_VALUES = 2
    FUNCTION_SET_SELECTED_VALUE = 3
    FUNCTION_GET_SELECTED_VALUE = 4
    FUNCTION_GET_TIMESTAMP = 234
    FUNCTION_COPY_FIRMWARE = 235
    FUNCTION_SET_WRITE_FIRMWARE_POINTER = 237
    FUNCTION_WRITE_FIRMWARE = 238
    FUNCTION_RESET = 243
    FUNCTION_WRITE_UID = 248
    FUNCTION_READ_UID = 249
    FUNCTION_GET_IDENTITY = 255

    COPY_STATUS_OK = 0
    COPY_STATUS_DEVICE_IDENTIFIER_INCORRECT = 1
    COPY_STATUS_MAGIC_NUMBER_INCORRECT = 2
    COPY_STATUS_LENGTH_MALFORMED = 3
    COPY_STATUS_CRC_MISMATCH = 4

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, TNGDO8.DEVICE_IDENTIFIER, TNGDO8.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[TNGDO8.FUNCTION_SET_VALUES] = TNGDO8.RESPONSE_EXPECTED_FALSE
        self.response_expected[TNGDO8.FUNCTION_GET_VALUES] = TNGDO8.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[TNGDO8.FUNCTION_SET_SELECTED_VALUE] = TNGDO8.RESPONSE_EXPECTED_FALSE
        self.response_expected[TNGDO8.FUNCTION_GET_SELECTED_VALUE] = TNGDO8.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[TNGDO8.FUNCTION_GET_TIMESTAMP] = TNGDO8.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[TNGDO8.FUNCTION_COPY_FIRMWARE] = TNGDO8.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[TNGDO8.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = TNGDO8.RESPONSE_EXPECTED_FALSE
        self.response_expected[TNGDO8.FUNCTION_WRITE_FIRMWARE] = TNGDO8.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[TNGDO8.FUNCTION_RESET] = TNGDO8.RESPONSE_EXPECTED_FALSE
        self.response_expected[TNGDO8.FUNCTION_WRITE_UID] = TNGDO8.RESPONSE_EXPECTED_FALSE
        self.response_expected[TNGDO8.FUNCTION_READ_UID] = TNGDO8.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[TNGDO8.FUNCTION_GET_IDENTITY] = TNGDO8.RESPONSE_EXPECTED_ALWAYS_TRUE


        ipcon.add_device(self)

    def set_values(self, timestamp, values):
        r"""
        timestamp = 0 => now
        """
        self.check_validity()

        timestamp = int(timestamp)
        values = list(map(bool, values))

        self.ipcon.send_request(self, TNGDO8.FUNCTION_SET_VALUES, (timestamp, values), 'Q 8!', 0, '')

    def get_values(self):
        r"""

        """
        self.check_validity()

        return GetValues(*self.ipcon.send_request(self, TNGDO8.FUNCTION_GET_VALUES, (), '', 17, 'Q 8!'))

    def set_selected_value(self, channel, timestamp, value):
        r"""
        timestamp = 0 => now
        """
        self.check_validity()

        channel = int(channel)
        timestamp = int(timestamp)
        value = list(map(bool, value))

        self.ipcon.send_request(self, TNGDO8.FUNCTION_SET_SELECTED_VALUE, (channel, timestamp, value), 'B Q 8!', 0, '')

    def get_selected_value(self, channel):
        r"""

        """
        self.check_validity()

        channel = int(channel)

        return GetSelectedValue(*self.ipcon.send_request(self, TNGDO8.FUNCTION_GET_SELECTED_VALUE, (channel,), 'B', 17, 'Q !'))

    def get_timestamp(self):
        r"""
        TODO
        """
        self.check_validity()

        return self.ipcon.send_request(self, TNGDO8.FUNCTION_GET_TIMESTAMP, (), '', 16, 'Q')

    def copy_firmware(self):
        r"""
        TODO
        """
        self.check_validity()

        return self.ipcon.send_request(self, TNGDO8.FUNCTION_COPY_FIRMWARE, (), '', 9, 'B')

    def set_write_firmware_pointer(self, pointer):
        r"""
        TODO
        """
        self.check_validity()

        pointer = int(pointer)

        self.ipcon.send_request(self, TNGDO8.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

    def write_firmware(self, data):
        r"""
        TODO
        """
        self.check_validity()

        data = list(map(int, data))

        return self.ipcon.send_request(self, TNGDO8.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

    def reset(self):
        r"""
        Calling this function will reset the TNG module. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, TNGDO8.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, TNGDO8.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, TNGDO8.FUNCTION_READ_UID, (), '', 12, 'I')

    def get_identity(self):
        r"""
        Returns the UID, the UID where the Brick is connected to,
        the position, the hardware and firmware version as well as the
        device identifier.

        The position is the position in the stack from '0' (bottom) to '8' (top).

        The device identifier numbers can be found :ref:`here <device_identifier>`.
        |device_identifier_constant|
        """
        return GetIdentity(*self.ipcon.send_request(self, TNGDO8.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))
