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

GetRemoteConfiguration = namedtuple('RemoteConfiguration', ['remote_type', 'minimum_repeats', 'callback_enabled'])
GetRemoteStatusA = namedtuple('RemoteStatusA', ['house_code', 'receiver_code', 'switch_to', 'repeats'])
GetRemoteStatusB = namedtuple('RemoteStatusB', ['address', 'unit', 'switch_to', 'dim_value', 'repeats'])
GetRemoteStatusC = namedtuple('RemoteStatusC', ['system_code', 'device_code', 'switch_to', 'repeats'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletRemoteSwitchV2(Device):
    r"""
    Controls remote mains switches and receives signals from remotes
    """

    DEVICE_IDENTIFIER = 289
    DEVICE_DISPLAY_NAME = 'Remote Switch Bricklet 2.0'
    DEVICE_URL_PART = 'remote_switch_v2' # internal

    CALLBACK_SWITCHING_DONE = 2
    CALLBACK_REMOTE_STATUS_A = 14
    CALLBACK_REMOTE_STATUS_B = 15
    CALLBACK_REMOTE_STATUS_C = 16


    FUNCTION_GET_SWITCHING_STATE = 1
    FUNCTION_SET_REPEATS = 3
    FUNCTION_GET_REPEATS = 4
    FUNCTION_SWITCH_SOCKET_A = 5
    FUNCTION_SWITCH_SOCKET_B = 6
    FUNCTION_DIM_SOCKET_B = 7
    FUNCTION_SWITCH_SOCKET_C = 8
    FUNCTION_SET_REMOTE_CONFIGURATION = 9
    FUNCTION_GET_REMOTE_CONFIGURATION = 10
    FUNCTION_GET_REMOTE_STATUS_A = 11
    FUNCTION_GET_REMOTE_STATUS_B = 12
    FUNCTION_GET_REMOTE_STATUS_C = 13
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

    SWITCHING_STATE_READY = 0
    SWITCHING_STATE_BUSY = 1
    SWITCH_TO_OFF = 0
    SWITCH_TO_ON = 1
    REMOTE_TYPE_A = 0
    REMOTE_TYPE_B = 1
    REMOTE_TYPE_C = 2
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
        Device.__init__(self, uid, ipcon, BrickletRemoteSwitchV2.DEVICE_IDENTIFIER, BrickletRemoteSwitchV2.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletRemoteSwitchV2.FUNCTION_GET_SWITCHING_STATE] = BrickletRemoteSwitchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRemoteSwitchV2.FUNCTION_SET_REPEATS] = BrickletRemoteSwitchV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRemoteSwitchV2.FUNCTION_GET_REPEATS] = BrickletRemoteSwitchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRemoteSwitchV2.FUNCTION_SWITCH_SOCKET_A] = BrickletRemoteSwitchV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRemoteSwitchV2.FUNCTION_SWITCH_SOCKET_B] = BrickletRemoteSwitchV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRemoteSwitchV2.FUNCTION_DIM_SOCKET_B] = BrickletRemoteSwitchV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRemoteSwitchV2.FUNCTION_SWITCH_SOCKET_C] = BrickletRemoteSwitchV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRemoteSwitchV2.FUNCTION_SET_REMOTE_CONFIGURATION] = BrickletRemoteSwitchV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRemoteSwitchV2.FUNCTION_GET_REMOTE_CONFIGURATION] = BrickletRemoteSwitchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRemoteSwitchV2.FUNCTION_GET_REMOTE_STATUS_A] = BrickletRemoteSwitchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRemoteSwitchV2.FUNCTION_GET_REMOTE_STATUS_B] = BrickletRemoteSwitchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRemoteSwitchV2.FUNCTION_GET_REMOTE_STATUS_C] = BrickletRemoteSwitchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRemoteSwitchV2.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletRemoteSwitchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRemoteSwitchV2.FUNCTION_SET_BOOTLOADER_MODE] = BrickletRemoteSwitchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRemoteSwitchV2.FUNCTION_GET_BOOTLOADER_MODE] = BrickletRemoteSwitchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRemoteSwitchV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletRemoteSwitchV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRemoteSwitchV2.FUNCTION_WRITE_FIRMWARE] = BrickletRemoteSwitchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRemoteSwitchV2.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletRemoteSwitchV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRemoteSwitchV2.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletRemoteSwitchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRemoteSwitchV2.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletRemoteSwitchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRemoteSwitchV2.FUNCTION_RESET] = BrickletRemoteSwitchV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRemoteSwitchV2.FUNCTION_WRITE_UID] = BrickletRemoteSwitchV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletRemoteSwitchV2.FUNCTION_READ_UID] = BrickletRemoteSwitchV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletRemoteSwitchV2.FUNCTION_GET_IDENTITY] = BrickletRemoteSwitchV2.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletRemoteSwitchV2.CALLBACK_SWITCHING_DONE] = (8, '')
        self.callback_formats[BrickletRemoteSwitchV2.CALLBACK_REMOTE_STATUS_A] = (13, 'B B B H')
        self.callback_formats[BrickletRemoteSwitchV2.CALLBACK_REMOTE_STATUS_B] = (17, 'I B B B H')
        self.callback_formats[BrickletRemoteSwitchV2.CALLBACK_REMOTE_STATUS_C] = (13, 'c B B H')

        ipcon.add_device(self)

    def get_switching_state(self):
        r"""
        Returns the current switching state. If the current state is busy, the
        Bricklet is currently sending a code to switch a socket. It will not
        accept any calls of switch socket functions until the state changes to ready.

        How long the switching takes is dependent on the number of repeats, see
        :func:`Set Repeats`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRemoteSwitchV2.FUNCTION_GET_SWITCHING_STATE, (), '', 9, 'B')

    def set_repeats(self, repeats):
        r"""
        Sets the number of times the code is sent when one of the Switch Socket
        functions is called. The repeats basically correspond to the amount of time
        that a button of the remote is pressed.

        Some dimmers are controlled by the length of a button pressed,
        this can be simulated by increasing the repeats.
        """
        self.check_validity()

        repeats = int(repeats)

        self.ipcon.send_request(self, BrickletRemoteSwitchV2.FUNCTION_SET_REPEATS, (repeats,), 'B', 0, '')

    def get_repeats(self):
        r"""
        Returns the number of repeats as set by :func:`Set Repeats`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRemoteSwitchV2.FUNCTION_GET_REPEATS, (), '', 9, 'B')

    def switch_socket_a(self, house_code, receiver_code, switch_to):
        r"""
        To switch a type A socket you have to give the house code, receiver code and the
        state (on or off) you want to switch to.

        A detailed description on how you can figure out the house and receiver code
        can be found :ref:`here <remote_switch_bricklet_type_a_house_and_receiver_code>`.
        """
        self.check_validity()

        house_code = int(house_code)
        receiver_code = int(receiver_code)
        switch_to = int(switch_to)

        self.ipcon.send_request(self, BrickletRemoteSwitchV2.FUNCTION_SWITCH_SOCKET_A, (house_code, receiver_code, switch_to), 'B B B', 0, '')

    def switch_socket_b(self, address, unit, switch_to):
        r"""
        To switch a type B socket you have to give the address, unit and the state
        (on or off) you want to switch to.

        To switch all devices with the same address use 255 for the unit.

        A detailed description on how you can teach a socket the address and unit can
        be found :ref:`here <remote_switch_bricklet_type_b_address_and_unit>`.
        """
        self.check_validity()

        address = int(address)
        unit = int(unit)
        switch_to = int(switch_to)

        self.ipcon.send_request(self, BrickletRemoteSwitchV2.FUNCTION_SWITCH_SOCKET_B, (address, unit, switch_to), 'I B B', 0, '')

    def dim_socket_b(self, address, unit, dim_value):
        r"""
        To control a type B dimmer you have to give the address, unit and the
        dim value you want to set the dimmer to.

        A detailed description on how you can teach a dimmer the address and unit can
        be found :ref:`here <remote_switch_bricklet_type_b_address_and_unit>`.
        """
        self.check_validity()

        address = int(address)
        unit = int(unit)
        dim_value = int(dim_value)

        self.ipcon.send_request(self, BrickletRemoteSwitchV2.FUNCTION_DIM_SOCKET_B, (address, unit, dim_value), 'I B B', 0, '')

    def switch_socket_c(self, system_code, device_code, switch_to):
        r"""
        To switch a type C socket you have to give the system code, device code and the
        state (on or off) you want to switch to.

        A detailed description on how you can figure out the system and device code
        can be found :ref:`here <remote_switch_bricklet_type_c_system_and_device_code>`.
        """
        self.check_validity()

        system_code = create_char(system_code)
        device_code = int(device_code)
        switch_to = int(switch_to)

        self.ipcon.send_request(self, BrickletRemoteSwitchV2.FUNCTION_SWITCH_SOCKET_C, (system_code, device_code, switch_to), 'c B B', 0, '')

    def set_remote_configuration(self, remote_type, minimum_repeats, callback_enabled):
        r"""
        Sets the configuration for **receiving** data from a remote of type A, B or C.

        * Remote Type: A, B or C depending on the type of remote you want to receive.
        * Minimum Repeats: The minimum number of repeated data packets until the callback
          is triggered (if enabled).
        * Callback Enabled: Enable or disable callback (see :cb:`Remote Status A` callback,
          :cb:`Remote Status B` callback and :cb:`Remote Status C` callback).
        """
        self.check_validity()

        remote_type = int(remote_type)
        minimum_repeats = int(minimum_repeats)
        callback_enabled = bool(callback_enabled)

        self.ipcon.send_request(self, BrickletRemoteSwitchV2.FUNCTION_SET_REMOTE_CONFIGURATION, (remote_type, minimum_repeats, callback_enabled), 'B H !', 0, '')

    def get_remote_configuration(self):
        r"""
        Returns the remote configuration as set by :func:`Set Remote Configuration`
        """
        self.check_validity()

        return GetRemoteConfiguration(*self.ipcon.send_request(self, BrickletRemoteSwitchV2.FUNCTION_GET_REMOTE_CONFIGURATION, (), '', 12, 'B H !'))

    def get_remote_status_a(self):
        r"""
        Returns the house code, receiver code, switch state (on/off) and number of
        repeats for remote type A.

        Repeats == 0 means there was no button press. Repeats >= 1 means there
        was a button press with the specified house/receiver code. The repeats are the
        number of received identical data packets. The longer the button is pressed,
        the higher the repeat number.

        Use the callback to get this data automatically when a button is pressed,
        see :func:`Set Remote Configuration` and :cb:`Remote Status A` callback.
        """
        self.check_validity()

        return GetRemoteStatusA(*self.ipcon.send_request(self, BrickletRemoteSwitchV2.FUNCTION_GET_REMOTE_STATUS_A, (), '', 13, 'B B B H'))

    def get_remote_status_b(self):
        r"""
        Returns the address (unique per remote), unit (button number), switch state
        (on/off) and number of repeats for remote type B.

        If the remote supports dimming the dim value is used instead of the switch state.

        If repeats=0 there was no button press. If repeats >= 1 there
        was a button press with the specified address/unit. The repeats are the number of received
        identical data packets. The longer the button is pressed, the higher the repeat number.

        Use the callback to get this data automatically when a button is pressed,
        see :func:`Set Remote Configuration` and :cb:`Remote Status B` callback.
        """
        self.check_validity()

        return GetRemoteStatusB(*self.ipcon.send_request(self, BrickletRemoteSwitchV2.FUNCTION_GET_REMOTE_STATUS_B, (), '', 17, 'I B B B H'))

    def get_remote_status_c(self):
        r"""
        Returns the system code, device code, switch state (on/off) and number of repeats for
        remote type C.

        If repeats=0 there was no button press. If repeats >= 1 there
        was a button press with the specified system/device code. The repeats are the number of received
        identical data packets. The longer the button is pressed, the higher the repeat number.

        Use the callback to get this data automatically when a button is pressed,
        see :func:`Set Remote Configuration` and :cb:`Remote Status C` callback.
        """
        self.check_validity()

        return GetRemoteStatusC(*self.ipcon.send_request(self, BrickletRemoteSwitchV2.FUNCTION_GET_REMOTE_STATUS_C, (), '', 13, 'c B B H'))

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletRemoteSwitchV2.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletRemoteSwitchV2.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRemoteSwitchV2.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletRemoteSwitchV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletRemoteSwitchV2.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletRemoteSwitchV2.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRemoteSwitchV2.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRemoteSwitchV2.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletRemoteSwitchV2.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletRemoteSwitchV2.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletRemoteSwitchV2.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletRemoteSwitchV2.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

RemoteSwitchV2 = BrickletRemoteSwitchV2 # for backward compatibility
