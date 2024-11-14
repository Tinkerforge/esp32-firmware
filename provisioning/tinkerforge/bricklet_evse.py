# -*- coding: utf-8 -*-
#############################################################
# This file was automatically generated on 2024-11-14.      #
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

GetState = namedtuple('State', ['iec61851_state', 'charger_state', 'contactor_state', 'contactor_error', 'allowed_charging_current', 'error_state', 'lock_state'])
GetHardwareConfiguration = namedtuple('HardwareConfiguration', ['jumper_configuration', 'has_lock_switch', 'evse_version'])
GetLowLevelState = namedtuple('LowLevelState', ['led_state', 'cp_pwm_duty_cycle', 'adc_values', 'voltages', 'resistances', 'gpio', 'charging_time', 'time_since_state_change', 'uptime'])
GetChargingSlot = namedtuple('ChargingSlot', ['max_current', 'active', 'clear_on_disconnect'])
GetAllChargingSlots = namedtuple('AllChargingSlots', ['max_current', 'active_and_clear_on_disconnect'])
GetChargingSlotDefault = namedtuple('ChargingSlotDefault', ['max_current', 'active', 'clear_on_disconnect'])
GetUserCalibration = namedtuple('UserCalibration', ['user_calibration_active', 'voltage_diff', 'voltage_mul', 'voltage_div', 'resistance_2700', 'resistance_880'])
GetIndicatorLED = namedtuple('IndicatorLED', ['indication', 'duration'])
GetButtonState = namedtuple('ButtonState', ['button_press_time', 'button_release_time', 'button_pressed'])
GetAllData1 = namedtuple('AllData1', ['iec61851_state', 'charger_state', 'contactor_state', 'contactor_error', 'allowed_charging_current', 'error_state', 'lock_state', 'jumper_configuration', 'has_lock_switch', 'evse_version', 'led_state', 'cp_pwm_duty_cycle', 'adc_values', 'voltages', 'resistances', 'gpio', 'charging_time', 'time_since_state_change', 'uptime', 'indication', 'duration', 'button_press_time', 'button_release_time', 'button_pressed', 'boost_mode_enabled'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletEVSE(Device):
    r"""
    TBD
    """

    DEVICE_IDENTIFIER = 2159
    DEVICE_DISPLAY_NAME = 'EVSE Bricklet'
    DEVICE_URL_PART = 'evse' # internal



    FUNCTION_GET_STATE = 1
    FUNCTION_GET_HARDWARE_CONFIGURATION = 2
    FUNCTION_GET_LOW_LEVEL_STATE = 3
    FUNCTION_SET_CHARGING_SLOT = 4
    FUNCTION_SET_CHARGING_SLOT_MAX_CURRENT = 5
    FUNCTION_SET_CHARGING_SLOT_ACTIVE = 6
    FUNCTION_SET_CHARGING_SLOT_CLEAR_ON_DISCONNECT = 7
    FUNCTION_GET_CHARGING_SLOT = 8
    FUNCTION_GET_ALL_CHARGING_SLOTS = 9
    FUNCTION_SET_CHARGING_SLOT_DEFAULT = 10
    FUNCTION_GET_CHARGING_SLOT_DEFAULT = 11
    FUNCTION_CALIBRATE = 12
    FUNCTION_GET_USER_CALIBRATION = 13
    FUNCTION_SET_USER_CALIBRATION = 14
    FUNCTION_GET_DATA_STORAGE = 15
    FUNCTION_SET_DATA_STORAGE = 16
    FUNCTION_GET_INDICATOR_LED = 17
    FUNCTION_SET_INDICATOR_LED = 18
    FUNCTION_GET_BUTTON_STATE = 19
    FUNCTION_GET_ALL_DATA_1 = 20
    FUNCTION_FACTORY_RESET = 21
    FUNCTION_SET_BOOST_MODE = 22
    FUNCTION_GET_BOOST_MODE = 23
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

    IEC61851_STATE_A = 0
    IEC61851_STATE_B = 1
    IEC61851_STATE_C = 2
    IEC61851_STATE_D = 3
    IEC61851_STATE_EF = 4
    LED_STATE_OFF = 0
    LED_STATE_ON = 1
    LED_STATE_BLINKING = 2
    LED_STATE_FLICKER = 3
    LED_STATE_BREATHING = 4
    CHARGER_STATE_NOT_CONNECTED = 0
    CHARGER_STATE_WAITING_FOR_CHARGE_RELEASE = 1
    CHARGER_STATE_READY_TO_CHARGE = 2
    CHARGER_STATE_CHARGING = 3
    CHARGER_STATE_ERROR = 4
    CONTACTOR_STATE_AC1_NLIVE_AC2_NLIVE = 0
    CONTACTOR_STATE_AC1_LIVE_AC2_NLIVE = 1
    CONTACTOR_STATE_AC1_NLIVE_AC2_LIVE = 2
    CONTACTOR_STATE_AC1_LIVE_AC2_LIVE = 3
    LOCK_STATE_INIT = 0
    LOCK_STATE_OPEN = 1
    LOCK_STATE_CLOSING = 2
    LOCK_STATE_CLOSE = 3
    LOCK_STATE_OPENING = 4
    LOCK_STATE_ERROR = 5
    ERROR_STATE_OK = 0
    ERROR_STATE_SWITCH = 2
    ERROR_STATE_CALIBRATION = 3
    ERROR_STATE_CONTACTOR = 4
    ERROR_STATE_COMMUNICATION = 5
    JUMPER_CONFIGURATION_6A = 0
    JUMPER_CONFIGURATION_10A = 1
    JUMPER_CONFIGURATION_13A = 2
    JUMPER_CONFIGURATION_16A = 3
    JUMPER_CONFIGURATION_20A = 4
    JUMPER_CONFIGURATION_25A = 5
    JUMPER_CONFIGURATION_32A = 6
    JUMPER_CONFIGURATION_SOFTWARE = 7
    JUMPER_CONFIGURATION_UNCONFIGURED = 8
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
        Device.__init__(self, uid, ipcon, BrickletEVSE.DEVICE_IDENTIFIER, BrickletEVSE.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 2)

        self.response_expected[BrickletEVSE.FUNCTION_GET_STATE] = BrickletEVSE.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSE.FUNCTION_GET_HARDWARE_CONFIGURATION] = BrickletEVSE.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSE.FUNCTION_GET_LOW_LEVEL_STATE] = BrickletEVSE.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSE.FUNCTION_SET_CHARGING_SLOT] = BrickletEVSE.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSE.FUNCTION_SET_CHARGING_SLOT_MAX_CURRENT] = BrickletEVSE.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSE.FUNCTION_SET_CHARGING_SLOT_ACTIVE] = BrickletEVSE.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSE.FUNCTION_SET_CHARGING_SLOT_CLEAR_ON_DISCONNECT] = BrickletEVSE.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSE.FUNCTION_GET_CHARGING_SLOT] = BrickletEVSE.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSE.FUNCTION_GET_ALL_CHARGING_SLOTS] = BrickletEVSE.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSE.FUNCTION_SET_CHARGING_SLOT_DEFAULT] = BrickletEVSE.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSE.FUNCTION_GET_CHARGING_SLOT_DEFAULT] = BrickletEVSE.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSE.FUNCTION_CALIBRATE] = BrickletEVSE.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSE.FUNCTION_GET_USER_CALIBRATION] = BrickletEVSE.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSE.FUNCTION_SET_USER_CALIBRATION] = BrickletEVSE.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletEVSE.FUNCTION_GET_DATA_STORAGE] = BrickletEVSE.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSE.FUNCTION_SET_DATA_STORAGE] = BrickletEVSE.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSE.FUNCTION_GET_INDICATOR_LED] = BrickletEVSE.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSE.FUNCTION_SET_INDICATOR_LED] = BrickletEVSE.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSE.FUNCTION_GET_BUTTON_STATE] = BrickletEVSE.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSE.FUNCTION_GET_ALL_DATA_1] = BrickletEVSE.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSE.FUNCTION_FACTORY_RESET] = BrickletEVSE.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletEVSE.FUNCTION_SET_BOOST_MODE] = BrickletEVSE.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSE.FUNCTION_GET_BOOST_MODE] = BrickletEVSE.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSE.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletEVSE.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSE.FUNCTION_SET_BOOTLOADER_MODE] = BrickletEVSE.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSE.FUNCTION_GET_BOOTLOADER_MODE] = BrickletEVSE.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSE.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletEVSE.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSE.FUNCTION_WRITE_FIRMWARE] = BrickletEVSE.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSE.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletEVSE.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSE.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletEVSE.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSE.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletEVSE.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSE.FUNCTION_RESET] = BrickletEVSE.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSE.FUNCTION_WRITE_UID] = BrickletEVSE.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSE.FUNCTION_READ_UID] = BrickletEVSE.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSE.FUNCTION_GET_IDENTITY] = BrickletEVSE.RESPONSE_EXPECTED_ALWAYS_TRUE


        ipcon.add_device(self)

    def get_state(self):
        r"""
        TODO

        .. versionadded:: 2.0.5$nbsp;(Plugin)
        """
        self.check_validity()

        return GetState(*self.ipcon.send_request(self, BrickletEVSE.FUNCTION_GET_STATE, (), '', 16, 'B B B B H B B'))

    def get_hardware_configuration(self):
        r"""
        TODO
        """
        self.check_validity()

        return GetHardwareConfiguration(*self.ipcon.send_request(self, BrickletEVSE.FUNCTION_GET_HARDWARE_CONFIGURATION, (), '', 11, 'B ! B'))

    def get_low_level_state(self):
        r"""
        TODO
        """
        self.check_validity()

        return GetLowLevelState(*self.ipcon.send_request(self, BrickletEVSE.FUNCTION_GET_LOW_LEVEL_STATE, (), '', 42, 'B H 2H 3h 2I 5! I I I'))

    def set_charging_slot(self, slot, max_current, active, clear_on_disconnect):
        r"""
        fixed slots:
        0: incoming cable (read-only, configured through slide switch)
        1: outgoing cable (read-only, configured through resistor)
        2: gpio input 0 (shutdown input)
        3: gpio input 1 (input)
        4: button (0A <-> 32A, can be controlled from web interface with start button and physical button if configured)


        .. versionadded:: 2.1.0$nbsp;(Plugin)
        """
        self.check_validity()

        slot = int(slot)
        max_current = int(max_current)
        active = bool(active)
        clear_on_disconnect = bool(clear_on_disconnect)

        self.ipcon.send_request(self, BrickletEVSE.FUNCTION_SET_CHARGING_SLOT, (slot, max_current, active, clear_on_disconnect), 'B H ! !', 0, '')

    def set_charging_slot_max_current(self, slot, max_current):
        r"""
        .. versionadded:: 2.1.0$nbsp;(Plugin)
        """
        self.check_validity()

        slot = int(slot)
        max_current = int(max_current)

        self.ipcon.send_request(self, BrickletEVSE.FUNCTION_SET_CHARGING_SLOT_MAX_CURRENT, (slot, max_current), 'B H', 0, '')

    def set_charging_slot_active(self, slot, active):
        r"""
        .. versionadded:: 2.1.0$nbsp;(Plugin)
        """
        self.check_validity()

        slot = int(slot)
        active = bool(active)

        self.ipcon.send_request(self, BrickletEVSE.FUNCTION_SET_CHARGING_SLOT_ACTIVE, (slot, active), 'B !', 0, '')

    def set_charging_slot_clear_on_disconnect(self, slot, clear_on_disconnect):
        r"""
        .. versionadded:: 2.1.0$nbsp;(Plugin)
        """
        self.check_validity()

        slot = int(slot)
        clear_on_disconnect = bool(clear_on_disconnect)

        self.ipcon.send_request(self, BrickletEVSE.FUNCTION_SET_CHARGING_SLOT_CLEAR_ON_DISCONNECT, (slot, clear_on_disconnect), 'B !', 0, '')

    def get_charging_slot(self, slot):
        r"""
        .. versionadded:: 2.1.0$nbsp;(Plugin)
        """
        self.check_validity()

        slot = int(slot)

        return GetChargingSlot(*self.ipcon.send_request(self, BrickletEVSE.FUNCTION_GET_CHARGING_SLOT, (slot,), 'B', 12, 'H ! !'))

    def get_all_charging_slots(self):
        r"""
        packed getter


        .. versionadded:: 2.1.0$nbsp;(Plugin)
        """
        self.check_validity()

        return GetAllChargingSlots(*self.ipcon.send_request(self, BrickletEVSE.FUNCTION_GET_ALL_CHARGING_SLOTS, (), '', 68, '20H 20B'))

    def set_charging_slot_default(self, slot, max_current, active, clear_on_disconnect):
        r"""
        fixed slots:
        0: incoming cable (read-only, configured through slide switch)
        1: outgoing cable (read-only, configured through resistor)
        2: gpio input 0 (shutdown input)
        3: gpio input 1 (input)


        .. versionadded:: 2.1.0$nbsp;(Plugin)
        """
        self.check_validity()

        slot = int(slot)
        max_current = int(max_current)
        active = bool(active)
        clear_on_disconnect = bool(clear_on_disconnect)

        self.ipcon.send_request(self, BrickletEVSE.FUNCTION_SET_CHARGING_SLOT_DEFAULT, (slot, max_current, active, clear_on_disconnect), 'B H ! !', 0, '')

    def get_charging_slot_default(self, slot):
        r"""
        .. versionadded:: 2.1.0$nbsp;(Plugin)
        """
        self.check_validity()

        slot = int(slot)

        return GetChargingSlotDefault(*self.ipcon.send_request(self, BrickletEVSE.FUNCTION_GET_CHARGING_SLOT_DEFAULT, (slot,), 'B', 12, 'H ! !'))

    def calibrate(self, state, password, value):
        r"""
        TODO
        """
        self.check_validity()

        state = int(state)
        password = int(password)
        value = int(value)

        return self.ipcon.send_request(self, BrickletEVSE.FUNCTION_CALIBRATE, (state, password, value), 'B I i', 9, '!')

    def get_user_calibration(self):
        r"""
        TODO
        """
        self.check_validity()

        return GetUserCalibration(*self.ipcon.send_request(self, BrickletEVSE.FUNCTION_GET_USER_CALIBRATION, (), '', 45, '! h h h h 14h'))

    def set_user_calibration(self, password, user_calibration_active, voltage_diff, voltage_mul, voltage_div, resistance_2700, resistance_880):
        r"""
        TODO
        """
        self.check_validity()

        password = int(password)
        user_calibration_active = bool(user_calibration_active)
        voltage_diff = int(voltage_diff)
        voltage_mul = int(voltage_mul)
        voltage_div = int(voltage_div)
        resistance_2700 = int(resistance_2700)
        resistance_880 = list(map(int, resistance_880))

        self.ipcon.send_request(self, BrickletEVSE.FUNCTION_SET_USER_CALIBRATION, (password, user_calibration_active, voltage_diff, voltage_mul, voltage_div, resistance_2700, resistance_880), 'I ! h h h h 14h', 0, '')

    def get_data_storage(self, page):
        r"""
        TODO
        """
        self.check_validity()

        page = int(page)

        return self.ipcon.send_request(self, BrickletEVSE.FUNCTION_GET_DATA_STORAGE, (page,), 'B', 71, '63B')

    def set_data_storage(self, page, data):
        r"""
        TODO
        """
        self.check_validity()

        page = int(page)
        data = list(map(int, data))

        self.ipcon.send_request(self, BrickletEVSE.FUNCTION_SET_DATA_STORAGE, (page, data), 'B 63B', 0, '')

    def get_indicator_led(self):
        r"""
        TODO
        """
        self.check_validity()

        return GetIndicatorLED(*self.ipcon.send_request(self, BrickletEVSE.FUNCTION_GET_INDICATOR_LED, (), '', 12, 'h H'))

    def set_indicator_led(self, indication, duration):
        r"""
        TODO
        """
        self.check_validity()

        indication = int(indication)
        duration = int(duration)

        return self.ipcon.send_request(self, BrickletEVSE.FUNCTION_SET_INDICATOR_LED, (indication, duration), 'h H', 9, 'B')

    def get_button_state(self):
        r"""
        TODO
        """
        self.check_validity()

        return GetButtonState(*self.ipcon.send_request(self, BrickletEVSE.FUNCTION_GET_BUTTON_STATE, (), '', 17, 'I I !'))

    def get_all_data_1(self):
        r"""
        TODO
        """
        self.check_validity()

        return GetAllData1(*self.ipcon.send_request(self, BrickletEVSE.FUNCTION_GET_ALL_DATA_1, (), '', 67, 'B B B B H B B B ! B B H 2H 3h 2I 5! I I I h H I I ! !'))

    def factory_reset(self, password):
        r"""
        TODO
        """
        self.check_validity()

        password = int(password)

        self.ipcon.send_request(self, BrickletEVSE.FUNCTION_FACTORY_RESET, (password,), 'I', 0, '')

    def set_boost_mode(self, boost_mode_enabled):
        r"""
        TODO
        """
        self.check_validity()

        boost_mode_enabled = bool(boost_mode_enabled)

        self.ipcon.send_request(self, BrickletEVSE.FUNCTION_SET_BOOST_MODE, (boost_mode_enabled,), '!', 0, '')

    def get_boost_mode(self):
        r"""
        TODO
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletEVSE.FUNCTION_GET_BOOST_MODE, (), '', 9, '!')

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletEVSE.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletEVSE.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletEVSE.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletEVSE.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletEVSE.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletEVSE.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletEVSE.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletEVSE.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletEVSE.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletEVSE.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletEVSE.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletEVSE.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

EVSE = BrickletEVSE # for backward compatibility
