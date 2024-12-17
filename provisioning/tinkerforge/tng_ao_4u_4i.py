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

GetValues = namedtuple('Values', ['timestamp', 'voltages', 'currents'])
GetVoltages = namedtuple('Voltages', ['timestamp', 'voltages'])
GetCurrents = namedtuple('Currents', ['timestamp', 'currents'])
GetSelectedVoltage = namedtuple('SelectedVoltage', ['timestamp', 'voltage'])
GetSelectedCurrent = namedtuple('SelectedCurrent', ['timestamp', 'current'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class TNGAO4U4I(Device):
    r"""
    TBD
    """

    DEVICE_IDENTIFIER = 206
    DEVICE_DISPLAY_NAME = 'TNG AO 4U 4I'
    DEVICE_URL_PART = 'ao_4u_4i' # internal



    FUNCTION_SET_VALUES = 1
    FUNCTION_GET_VALUES = 2
    FUNCTION_SET_VOLTAGES = 3
    FUNCTION_GET_VOLTAGES = 4
    FUNCTION_SET_CURRENTS = 5
    FUNCTION_GET_CURRENTS = 6
    FUNCTION_SET_SELECTED_VOLTAGE = 7
    FUNCTION_GET_SELECTED_VOLTAGE = 8
    FUNCTION_SET_SELECTED_CURRENT = 9
    FUNCTION_GET_SELECTED_CURRENT = 10
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
        Device.__init__(self, uid, ipcon, TNGAO4U4I.DEVICE_IDENTIFIER, TNGAO4U4I.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[TNGAO4U4I.FUNCTION_SET_VALUES] = TNGAO4U4I.RESPONSE_EXPECTED_FALSE
        self.response_expected[TNGAO4U4I.FUNCTION_GET_VALUES] = TNGAO4U4I.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[TNGAO4U4I.FUNCTION_SET_VOLTAGES] = TNGAO4U4I.RESPONSE_EXPECTED_FALSE
        self.response_expected[TNGAO4U4I.FUNCTION_GET_VOLTAGES] = TNGAO4U4I.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[TNGAO4U4I.FUNCTION_SET_CURRENTS] = TNGAO4U4I.RESPONSE_EXPECTED_FALSE
        self.response_expected[TNGAO4U4I.FUNCTION_GET_CURRENTS] = TNGAO4U4I.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[TNGAO4U4I.FUNCTION_SET_SELECTED_VOLTAGE] = TNGAO4U4I.RESPONSE_EXPECTED_FALSE
        self.response_expected[TNGAO4U4I.FUNCTION_GET_SELECTED_VOLTAGE] = TNGAO4U4I.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[TNGAO4U4I.FUNCTION_SET_SELECTED_CURRENT] = TNGAO4U4I.RESPONSE_EXPECTED_FALSE
        self.response_expected[TNGAO4U4I.FUNCTION_GET_SELECTED_CURRENT] = TNGAO4U4I.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[TNGAO4U4I.FUNCTION_GET_TIMESTAMP] = TNGAO4U4I.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[TNGAO4U4I.FUNCTION_COPY_FIRMWARE] = TNGAO4U4I.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[TNGAO4U4I.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = TNGAO4U4I.RESPONSE_EXPECTED_FALSE
        self.response_expected[TNGAO4U4I.FUNCTION_WRITE_FIRMWARE] = TNGAO4U4I.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[TNGAO4U4I.FUNCTION_RESET] = TNGAO4U4I.RESPONSE_EXPECTED_FALSE
        self.response_expected[TNGAO4U4I.FUNCTION_WRITE_UID] = TNGAO4U4I.RESPONSE_EXPECTED_FALSE
        self.response_expected[TNGAO4U4I.FUNCTION_READ_UID] = TNGAO4U4I.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[TNGAO4U4I.FUNCTION_GET_IDENTITY] = TNGAO4U4I.RESPONSE_EXPECTED_ALWAYS_TRUE


        ipcon.add_device(self)

    def set_values(self, timestamp, voltages, currents):
        r"""

        """
        self.check_validity()

        timestamp = int(timestamp)
        voltages = list(map(int, voltages))
        currents = list(map(int, currents))

        self.ipcon.send_request(self, TNGAO4U4I.FUNCTION_SET_VALUES, (timestamp, voltages, currents), 'Q 4i 4i', 0, '')

    def get_values(self):
        r"""

        """
        self.check_validity()

        return GetValues(*self.ipcon.send_request(self, TNGAO4U4I.FUNCTION_GET_VALUES, (), '', 48, 'Q 4i 4i'))

    def set_voltages(self, timestamp, voltages):
        r"""

        """
        self.check_validity()

        timestamp = int(timestamp)
        voltages = list(map(int, voltages))

        self.ipcon.send_request(self, TNGAO4U4I.FUNCTION_SET_VOLTAGES, (timestamp, voltages), 'Q 4i', 0, '')

    def get_voltages(self):
        r"""

        """
        self.check_validity()

        return GetVoltages(*self.ipcon.send_request(self, TNGAO4U4I.FUNCTION_GET_VOLTAGES, (), '', 32, 'Q 4i'))

    def set_currents(self, timestamp, currents):
        r"""

        """
        self.check_validity()

        timestamp = int(timestamp)
        currents = list(map(int, currents))

        self.ipcon.send_request(self, TNGAO4U4I.FUNCTION_SET_CURRENTS, (timestamp, currents), 'Q 4i', 0, '')

    def get_currents(self):
        r"""

        """
        self.check_validity()

        return GetCurrents(*self.ipcon.send_request(self, TNGAO4U4I.FUNCTION_GET_CURRENTS, (), '', 32, 'Q 4i'))

    def set_selected_voltage(self, channel, timestamp, voltage):
        r"""

        """
        self.check_validity()

        channel = int(channel)
        timestamp = int(timestamp)
        voltage = int(voltage)

        self.ipcon.send_request(self, TNGAO4U4I.FUNCTION_SET_SELECTED_VOLTAGE, (channel, timestamp, voltage), 'B Q i', 0, '')

    def get_selected_voltage(self, channel):
        r"""

        """
        self.check_validity()

        channel = int(channel)

        return GetSelectedVoltage(*self.ipcon.send_request(self, TNGAO4U4I.FUNCTION_GET_SELECTED_VOLTAGE, (channel,), 'B', 20, 'Q i'))

    def set_selected_current(self, channel, timestamp, current):
        r"""

        """
        self.check_validity()

        channel = int(channel)
        timestamp = int(timestamp)
        current = int(current)

        self.ipcon.send_request(self, TNGAO4U4I.FUNCTION_SET_SELECTED_CURRENT, (channel, timestamp, current), 'B Q i', 0, '')

    def get_selected_current(self, channel):
        r"""

        """
        self.check_validity()

        channel = int(channel)

        return GetSelectedCurrent(*self.ipcon.send_request(self, TNGAO4U4I.FUNCTION_GET_SELECTED_CURRENT, (channel,), 'B', 20, 'Q i'))

    def get_timestamp(self):
        r"""
        TODO
        """
        self.check_validity()

        return self.ipcon.send_request(self, TNGAO4U4I.FUNCTION_GET_TIMESTAMP, (), '', 16, 'Q')

    def copy_firmware(self):
        r"""
        TODO
        """
        self.check_validity()

        return self.ipcon.send_request(self, TNGAO4U4I.FUNCTION_COPY_FIRMWARE, (), '', 9, 'B')

    def set_write_firmware_pointer(self, pointer):
        r"""
        TODO
        """
        self.check_validity()

        pointer = int(pointer)

        self.ipcon.send_request(self, TNGAO4U4I.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

    def write_firmware(self, data):
        r"""
        TODO
        """
        self.check_validity()

        data = list(map(int, data))

        return self.ipcon.send_request(self, TNGAO4U4I.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

    def reset(self):
        r"""
        Calling this function will reset the TNG module. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, TNGAO4U4I.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, TNGAO4U4I.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, TNGAO4U4I.FUNCTION_READ_UID, (), '', 12, 'I')

    def get_identity(self):
        r"""
        Returns the UID, the UID where the Brick is connected to,
        the position, the hardware and firmware version as well as the
        device identifier.

        The position is the position in the stack from '0' (bottom) to '8' (top).

        The device identifier numbers can be found :ref:`here <device_identifier>`.
        |device_identifier_constant|
        """
        return GetIdentity(*self.ipcon.send_request(self, TNGAO4U4I.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))
