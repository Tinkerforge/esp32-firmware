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

SearchBusLowLevel = namedtuple('SearchBusLowLevel', ['identifier_length', 'identifier_chunk_offset', 'identifier_chunk_data', 'status'])
Read = namedtuple('Read', ['data', 'status'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])
SearchBus = namedtuple('SearchBus', ['identifier', 'status'])

class BrickletOneWire(Device):
    r"""
    Communicates with up 64 1-Wire devices
    """

    DEVICE_IDENTIFIER = 2123
    DEVICE_DISPLAY_NAME = 'One Wire Bricklet'
    DEVICE_URL_PART = 'one_wire' # internal



    FUNCTION_SEARCH_BUS_LOW_LEVEL = 1
    FUNCTION_RESET_BUS = 2
    FUNCTION_WRITE = 3
    FUNCTION_READ = 4
    FUNCTION_WRITE_COMMAND = 5
    FUNCTION_SET_COMMUNICATION_LED_CONFIG = 6
    FUNCTION_GET_COMMUNICATION_LED_CONFIG = 7
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

    STATUS_OK = 0
    STATUS_BUSY = 1
    STATUS_NO_PRESENCE = 2
    STATUS_TIMEOUT = 3
    STATUS_ERROR = 4
    COMMUNICATION_LED_CONFIG_OFF = 0
    COMMUNICATION_LED_CONFIG_ON = 1
    COMMUNICATION_LED_CONFIG_SHOW_HEARTBEAT = 2
    COMMUNICATION_LED_CONFIG_SHOW_COMMUNICATION = 3
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
        Device.__init__(self, uid, ipcon, BrickletOneWire.DEVICE_IDENTIFIER, BrickletOneWire.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletOneWire.FUNCTION_SEARCH_BUS_LOW_LEVEL] = BrickletOneWire.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletOneWire.FUNCTION_RESET_BUS] = BrickletOneWire.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletOneWire.FUNCTION_WRITE] = BrickletOneWire.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletOneWire.FUNCTION_READ] = BrickletOneWire.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletOneWire.FUNCTION_WRITE_COMMAND] = BrickletOneWire.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletOneWire.FUNCTION_SET_COMMUNICATION_LED_CONFIG] = BrickletOneWire.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletOneWire.FUNCTION_GET_COMMUNICATION_LED_CONFIG] = BrickletOneWire.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletOneWire.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletOneWire.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletOneWire.FUNCTION_SET_BOOTLOADER_MODE] = BrickletOneWire.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletOneWire.FUNCTION_GET_BOOTLOADER_MODE] = BrickletOneWire.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletOneWire.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletOneWire.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletOneWire.FUNCTION_WRITE_FIRMWARE] = BrickletOneWire.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletOneWire.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletOneWire.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletOneWire.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletOneWire.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletOneWire.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletOneWire.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletOneWire.FUNCTION_RESET] = BrickletOneWire.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletOneWire.FUNCTION_WRITE_UID] = BrickletOneWire.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletOneWire.FUNCTION_READ_UID] = BrickletOneWire.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletOneWire.FUNCTION_GET_IDENTITY] = BrickletOneWire.RESPONSE_EXPECTED_ALWAYS_TRUE


        ipcon.add_device(self)

    def search_bus_low_level(self):
        r"""
        Returns a list of up to 64 identifiers of the connected 1-Wire devices.
        Each identifier is 64-bit and consists of 8-bit family code, 48-bit ID and
        8-bit CRC.

        To get these identifiers the Bricklet runs the
        `SEARCH ROM algorithm <https://www.maximintegrated.com/en/app-notes/index.mvp/id/187>`__,
        as defined by Maxim.
        """
        self.check_validity()

        return SearchBusLowLevel(*self.ipcon.send_request(self, BrickletOneWire.FUNCTION_SEARCH_BUS_LOW_LEVEL, (), '', 69, 'H H 7Q B'))

    def reset_bus(self):
        r"""
        Resets the bus with the 1-Wire reset operation.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletOneWire.FUNCTION_RESET_BUS, (), '', 9, 'B')

    def write(self, data):
        r"""
        Writes a byte of data to the 1-Wire bus.
        """
        self.check_validity()

        data = int(data)

        return self.ipcon.send_request(self, BrickletOneWire.FUNCTION_WRITE, (data,), 'B', 9, 'B')

    def read(self):
        r"""
        Reads a byte of data from the 1-Wire bus.
        """
        self.check_validity()

        return Read(*self.ipcon.send_request(self, BrickletOneWire.FUNCTION_READ, (), '', 10, 'B B'))

    def write_command(self, identifier, command):
        r"""
        Writes a command to the 1-Wire device with the given identifier. You can obtain
        the identifier by calling :func:`Search Bus`. The MATCH ROM operation is used to
        write the command.

        If you only have one device connected or want to broadcast to all devices
        you can set the identifier to 0. In this case the SKIP ROM operation is used to
        write the command.
        """
        self.check_validity()

        identifier = int(identifier)
        command = int(command)

        return self.ipcon.send_request(self, BrickletOneWire.FUNCTION_WRITE_COMMAND, (identifier, command), 'Q B', 9, 'B')

    def set_communication_led_config(self, config):
        r"""
        Sets the communication LED configuration. By default the LED shows 1-wire
        communication traffic by flickering.

        You can also turn the LED permanently on/off or show a heartbeat.

        If the Bricklet is in bootloader mode, the LED is off.
        """
        self.check_validity()

        config = int(config)

        self.ipcon.send_request(self, BrickletOneWire.FUNCTION_SET_COMMUNICATION_LED_CONFIG, (config,), 'B', 0, '')

    def get_communication_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Communication LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletOneWire.FUNCTION_GET_COMMUNICATION_LED_CONFIG, (), '', 9, 'B')

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletOneWire.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletOneWire.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletOneWire.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletOneWire.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletOneWire.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletOneWire.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletOneWire.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletOneWire.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletOneWire.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletOneWire.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletOneWire.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletOneWire.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def search_bus(self):
        r"""
        Returns a list of up to 64 identifiers of the connected 1-Wire devices.
        Each identifier is 64-bit and consists of 8-bit family code, 48-bit ID and
        8-bit CRC.

        To get these identifiers the Bricklet runs the
        `SEARCH ROM algorithm <https://www.maximintegrated.com/en/app-notes/index.mvp/id/187>`__,
        as defined by Maxim.
        """
        with self.stream_lock:
            ret = self.search_bus_low_level()
            identifier_length = ret.identifier_length
            identifier_out_of_sync = ret.identifier_chunk_offset != 0
            identifier_data = ret.identifier_chunk_data

            while not identifier_out_of_sync and len(identifier_data) < identifier_length:
                ret = self.search_bus_low_level()
                identifier_length = ret.identifier_length
                identifier_out_of_sync = ret.identifier_chunk_offset != len(identifier_data)
                identifier_data += ret.identifier_chunk_data

            if identifier_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.identifier_chunk_offset + 7 < identifier_length:
                    ret = self.search_bus_low_level()
                    identifier_length = ret.identifier_length

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Identifier stream is out-of-sync')

        return SearchBus(identifier_data[:identifier_length], ret.status)

OneWire = BrickletOneWire # for backward compatibility
