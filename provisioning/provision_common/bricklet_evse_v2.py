# -*- coding: utf-8 -*-
#############################################################
# This file was automatically generated on 2024-03-14.      #
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

GetState = namedtuple('State', ['iec61851_state', 'charger_state', 'contactor_state', 'contactor_error', 'allowed_charging_current', 'error_state', 'lock_state', 'dc_fault_current_state'])
GetHardwareConfiguration = namedtuple('HardwareConfiguration', ['jumper_configuration', 'has_lock_switch', 'evse_version', 'energy_meter_type'])
GetLowLevelState = namedtuple('LowLevelState', ['led_state', 'cp_pwm_duty_cycle', 'adc_values', 'voltages', 'resistances', 'gpio', 'charging_time', 'time_since_state_change', 'time_since_dc_fault_check', 'uptime'])
GetChargingSlot = namedtuple('ChargingSlot', ['max_current', 'active', 'clear_on_disconnect'])
GetAllChargingSlots = namedtuple('AllChargingSlots', ['max_current', 'active_and_clear_on_disconnect'])
GetChargingSlotDefault = namedtuple('ChargingSlotDefault', ['max_current', 'active', 'clear_on_disconnect'])
GetEnergyMeterValues = namedtuple('EnergyMeterValues', ['power', 'current', 'phases_active', 'phases_connected'])
GetAllEnergyMeterValuesLowLevel = namedtuple('AllEnergyMeterValuesLowLevel', ['values_chunk_offset', 'values_chunk_data'])
GetGPIOConfiguration = namedtuple('GPIOConfiguration', ['shutdown_input_configuration', 'input_configuration', 'output_configuration'])
GetIndicatorLED = namedtuple('IndicatorLED', ['indication', 'duration', 'color_h', 'color_s', 'color_v'])
GetButtonState = namedtuple('ButtonState', ['button_press_time', 'button_release_time', 'button_pressed'])
GetAllData1 = namedtuple('AllData1', ['iec61851_state', 'charger_state', 'contactor_state', 'contactor_error', 'allowed_charging_current', 'error_state', 'lock_state', 'dc_fault_current_state', 'jumper_configuration', 'has_lock_switch', 'evse_version', 'energy_meter_type', 'power', 'current', 'phases_active', 'phases_connected', 'error_count'])
GetAllData2 = namedtuple('AllData2', ['shutdown_input_configuration', 'input_configuration', 'output_configuration', 'indication', 'duration', 'color_h', 'color_s', 'color_v', 'button_configuration', 'button_press_time', 'button_release_time', 'button_pressed', 'ev_wakeup_enabled', 'control_pilot_disconnect', 'boost_mode_enabled', 'temperature', 'phases_current', 'phases_requested', 'phases_state', 'phases_info', 'phase_auto_switch_enabled'])
GetPhaseControl = namedtuple('PhaseControl', ['phases_current', 'phases_requested', 'phases_state', 'phases_info'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletEVSEV2(Device):
    r"""
    TBD
    """

    DEVICE_IDENTIFIER = 2167
    DEVICE_DISPLAY_NAME = 'EVSE Bricklet 2.0'
    DEVICE_URL_PART = 'evse_v2' # internal

    CALLBACK_ENERGY_METER_VALUES = 43


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
    FUNCTION_GET_ENERGY_METER_VALUES = 12
    FUNCTION_GET_ALL_ENERGY_METER_VALUES_LOW_LEVEL = 13
    FUNCTION_GET_ENERGY_METER_ERRORS = 14
    FUNCTION_RESET_ENERGY_METER_RELATIVE_ENERGY = 15
    FUNCTION_RESET_DC_FAULT_CURRENT_STATE = 16
    FUNCTION_SET_GPIO_CONFIGURATION = 17
    FUNCTION_GET_GPIO_CONFIGURATION = 18
    FUNCTION_GET_DATA_STORAGE = 19
    FUNCTION_SET_DATA_STORAGE = 20
    FUNCTION_GET_INDICATOR_LED = 21
    FUNCTION_SET_INDICATOR_LED = 22
    FUNCTION_SET_BUTTON_CONFIGURATION = 23
    FUNCTION_GET_BUTTON_CONFIGURATION = 24
    FUNCTION_GET_BUTTON_STATE = 25
    FUNCTION_SET_EV_WAKEUP = 26
    FUNCTION_GET_EV_WAKUEP = 27
    FUNCTION_SET_CONTROL_PILOT_DISCONNECT = 28
    FUNCTION_GET_CONTROL_PILOT_DISCONNECT = 29
    FUNCTION_GET_ALL_DATA_1 = 30
    FUNCTION_GET_ALL_DATA_2 = 31
    FUNCTION_FACTORY_RESET = 32
    FUNCTION_GET_BUTTON_PRESS_BOOT_TIME = 33
    FUNCTION_SET_BOOST_MODE = 34
    FUNCTION_GET_BOOST_MODE = 35
    FUNCTION_TRIGGER_DC_FAULT_TEST = 36
    FUNCTION_SET_GP_OUTPUT = 37
    FUNCTION_GET_TEMPERATURE = 38
    FUNCTION_SET_PHASE_CONTROL = 39
    FUNCTION_GET_PHASE_CONTROL = 40
    FUNCTION_SET_PHASE_AUTO_SWITCH = 41
    FUNCTION_GET_PHASE_AUTO_SWITCH = 42
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
    ERROR_STATE_DC_FAULT = 3
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
    DC_FAULT_CURRENT_STATE_NORMAL_CONDITION = 0
    DC_FAULT_CURRENT_STATE_6_MA_DC_ERROR = 1
    DC_FAULT_CURRENT_STATE_SYSTEM_ERROR = 2
    DC_FAULT_CURRENT_STATE_UNKNOWN_ERROR = 3
    DC_FAULT_CURRENT_STATE_CALIBRATION_ERROR = 4
    DC_FAULT_CURRENT_STATE_20_MA_AC_ERROR = 5
    DC_FAULT_CURRENT_STATE_6_MA_AC_AND_20_MA_AC_ERROR = 6
    SHUTDOWN_INPUT_IGNORED = 0
    SHUTDOWN_INPUT_SHUTDOWN_ON_OPEN = 1
    SHUTDOWN_INPUT_SHUTDOWN_ON_CLOSE = 2
    SHUTDOWN_INPUT_4300_WATT_ON_OPEN = 3
    SHUTDOWN_INPUT_4300_WATT_ON_CLOSE = 4
    OUTPUT_CONNECTED_TO_GROUND = 0
    OUTPUT_HIGH_IMPEDANCE = 1
    BUTTON_CONFIGURATION_DEACTIVATED = 0
    BUTTON_CONFIGURATION_START_CHARGING = 1
    BUTTON_CONFIGURATION_STOP_CHARGING = 2
    BUTTON_CONFIGURATION_START_AND_STOP_CHARGING = 3
    CONTROL_PILOT_DISCONNECTED = 0
    CONTROL_PILOT_CONNECTED = 1
    CONTROL_PILOT_AUTOMATIC = 2
    ENERGY_METER_TYPE_NOT_AVAILABLE = 0
    ENERGY_METER_TYPE_SDM72 = 1
    ENERGY_METER_TYPE_SDM630 = 2
    ENERGY_METER_TYPE_SDM72V2 = 3
    ENERGY_METER_TYPE_SDM72CTM = 4
    ENERGY_METER_TYPE_SDM630MCTV2 = 5
    ENERGY_METER_TYPE_DSZ15DZMOD = 6
    ENERGY_METER_TYPE_DEM4A = 7
    INPUT_UNCONFIGURED = 0
    INPUT_ACTIVE_LOW_MAX_0A = 1
    INPUT_ACTIVE_LOW_MAX_6A = 2
    INPUT_ACTIVE_LOW_MAX_8A = 3
    INPUT_ACTIVE_LOW_MAX_10A = 4
    INPUT_ACTIVE_LOW_MAX_13A = 5
    INPUT_ACTIVE_LOW_MAX_16A = 6
    INPUT_ACTIVE_LOW_MAX_20A = 7
    INPUT_ACTIVE_LOW_MAX_25A = 8
    INPUT_ACTIVE_HIGH_MAX_0A = 9
    INPUT_ACTIVE_HIGH_MAX_6A = 10
    INPUT_ACTIVE_HIGH_MAX_8A = 11
    INPUT_ACTIVE_HIGH_MAX_10A = 12
    INPUT_ACTIVE_HIGH_MAX_13A = 13
    INPUT_ACTIVE_HIGH_MAX_16A = 14
    INPUT_ACTIVE_HIGH_MAX_20A = 15
    INPUT_ACTIVE_HIGH_MAX_25A = 16
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
        Device.__init__(self, uid, ipcon, BrickletEVSEV2.DEVICE_IDENTIFIER, BrickletEVSEV2.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletEVSEV2.FUNCTION_GET_STATE] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_HARDWARE_CONFIGURATION] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_LOW_LEVEL_STATE] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_SET_CHARGING_SLOT] = BrickletEVSEV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSEV2.FUNCTION_SET_CHARGING_SLOT_MAX_CURRENT] = BrickletEVSEV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSEV2.FUNCTION_SET_CHARGING_SLOT_ACTIVE] = BrickletEVSEV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSEV2.FUNCTION_SET_CHARGING_SLOT_CLEAR_ON_DISCONNECT] = BrickletEVSEV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_CHARGING_SLOT] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_ALL_CHARGING_SLOTS] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_SET_CHARGING_SLOT_DEFAULT] = BrickletEVSEV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_CHARGING_SLOT_DEFAULT] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_ENERGY_METER_VALUES] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_ALL_ENERGY_METER_VALUES_LOW_LEVEL] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_ENERGY_METER_ERRORS] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_RESET_ENERGY_METER_RELATIVE_ENERGY] = BrickletEVSEV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSEV2.FUNCTION_RESET_DC_FAULT_CURRENT_STATE] = BrickletEVSEV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_SET_GPIO_CONFIGURATION] = BrickletEVSEV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_GPIO_CONFIGURATION] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_DATA_STORAGE] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_SET_DATA_STORAGE] = BrickletEVSEV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_INDICATOR_LED] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_SET_INDICATOR_LED] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_SET_BUTTON_CONFIGURATION] = BrickletEVSEV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_BUTTON_CONFIGURATION] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_BUTTON_STATE] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_SET_EV_WAKEUP] = BrickletEVSEV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_EV_WAKUEP] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_SET_CONTROL_PILOT_DISCONNECT] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_CONTROL_PILOT_DISCONNECT] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_ALL_DATA_1] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_ALL_DATA_2] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_FACTORY_RESET] = BrickletEVSEV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_BUTTON_PRESS_BOOT_TIME] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_SET_BOOST_MODE] = BrickletEVSEV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_BOOST_MODE] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_TRIGGER_DC_FAULT_TEST] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_SET_GP_OUTPUT] = BrickletEVSEV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_TEMPERATURE] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_SET_PHASE_CONTROL] = BrickletEVSEV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_PHASE_CONTROL] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_SET_PHASE_AUTO_SWITCH] = BrickletEVSEV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_PHASE_AUTO_SWITCH] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_SET_BOOTLOADER_MODE] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_BOOTLOADER_MODE] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletEVSEV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSEV2.FUNCTION_WRITE_FIRMWARE] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletEVSEV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_RESET] = BrickletEVSEV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSEV2.FUNCTION_WRITE_UID] = BrickletEVSEV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEVSEV2.FUNCTION_READ_UID] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEVSEV2.FUNCTION_GET_IDENTITY] = BrickletEVSEV2.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletEVSEV2.CALLBACK_ENERGY_METER_VALUES] = (26, 'f 3f 3! 3!')

        ipcon.add_device(self)

    def get_state(self):
        r"""
        TODO
        """
        self.check_validity()

        return GetState(*self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_STATE, (), '', 17, 'B B B B H B B B'))

    def get_hardware_configuration(self):
        r"""
        TODO
        """
        self.check_validity()

        return GetHardwareConfiguration(*self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_HARDWARE_CONFIGURATION, (), '', 12, 'B ! B B'))

    def get_low_level_state(self):
        r"""
        TODO
        """
        self.check_validity()

        return GetLowLevelState(*self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_LOW_LEVEL_STATE, (), '', 66, 'B H 7H 7h 2I 24! I I I I'))

    def set_charging_slot(self, slot, max_current, active, clear_on_disconnect):
        r"""
        fixed slots:
        0: incoming cable (read-only, configured through slide switch)
        1: outgoing cable (read-only, configured through resistor)
        2: gpio input 0 (shutdown input)
        3: gpio input 1 (input)
        4: button (0A <-> 32A, can be controlled from web interface with start button and physical button if configured)
        """
        self.check_validity()

        slot = int(slot)
        max_current = int(max_current)
        active = bool(active)
        clear_on_disconnect = bool(clear_on_disconnect)

        self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_SET_CHARGING_SLOT, (slot, max_current, active, clear_on_disconnect), 'B H ! !', 0, '')

    def set_charging_slot_max_current(self, slot, max_current):
        r"""

        """
        self.check_validity()

        slot = int(slot)
        max_current = int(max_current)

        self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_SET_CHARGING_SLOT_MAX_CURRENT, (slot, max_current), 'B H', 0, '')

    def set_charging_slot_active(self, slot, active):
        r"""

        """
        self.check_validity()

        slot = int(slot)
        active = bool(active)

        self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_SET_CHARGING_SLOT_ACTIVE, (slot, active), 'B !', 0, '')

    def set_charging_slot_clear_on_disconnect(self, slot, clear_on_disconnect):
        r"""

        """
        self.check_validity()

        slot = int(slot)
        clear_on_disconnect = bool(clear_on_disconnect)

        self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_SET_CHARGING_SLOT_CLEAR_ON_DISCONNECT, (slot, clear_on_disconnect), 'B !', 0, '')

    def get_charging_slot(self, slot):
        r"""

        """
        self.check_validity()

        slot = int(slot)

        return GetChargingSlot(*self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_CHARGING_SLOT, (slot,), 'B', 12, 'H ! !'))

    def get_all_charging_slots(self):
        r"""
        packed getter
        """
        self.check_validity()

        return GetAllChargingSlots(*self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_ALL_CHARGING_SLOTS, (), '', 68, '20H 20B'))

    def set_charging_slot_default(self, slot, max_current, active, clear_on_disconnect):
        r"""
        fixed slots:
        0: incoming cable (read-only, configured through slide switch)
        1: outgoing cable (read-only, configured through resistor)
        2: gpio input 0 (shutdown input)
        3: gpio input 1 (input)
        """
        self.check_validity()

        slot = int(slot)
        max_current = int(max_current)
        active = bool(active)
        clear_on_disconnect = bool(clear_on_disconnect)

        self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_SET_CHARGING_SLOT_DEFAULT, (slot, max_current, active, clear_on_disconnect), 'B H ! !', 0, '')

    def get_charging_slot_default(self, slot):
        r"""

        """
        self.check_validity()

        slot = int(slot)

        return GetChargingSlotDefault(*self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_CHARGING_SLOT_DEFAULT, (slot,), 'B', 12, 'H ! !'))

    def get_energy_meter_values(self):
        r"""
        TODO
        """
        self.check_validity()

        return GetEnergyMeterValues(*self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_ENERGY_METER_VALUES, (), '', 26, 'f 3f 3! 3!'))

    def get_all_energy_meter_values_low_level(self):
        r"""
        TBD
        """
        self.check_validity()

        return GetAllEnergyMeterValuesLowLevel(*self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_ALL_ENERGY_METER_VALUES_LOW_LEVEL, (), '', 70, 'H 15f'))

    def get_energy_meter_errors(self):
        r"""
        TODO
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_ENERGY_METER_ERRORS, (), '', 32, '6I')

    def reset_energy_meter_relative_energy(self):
        r"""
        TODO
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_RESET_ENERGY_METER_RELATIVE_ENERGY, (), '', 0, '')

    def reset_dc_fault_current_state(self, password):
        r"""
        TODO
        """
        self.check_validity()

        password = int(password)

        self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_RESET_DC_FAULT_CURRENT_STATE, (password,), 'I', 0, '')

    def set_gpio_configuration(self, shutdown_input_configuration, input_configuration, output_configuration):
        r"""
        TODO
        """
        self.check_validity()

        shutdown_input_configuration = int(shutdown_input_configuration)
        input_configuration = int(input_configuration)
        output_configuration = int(output_configuration)

        self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_SET_GPIO_CONFIGURATION, (shutdown_input_configuration, input_configuration, output_configuration), 'B B B', 0, '')

    def get_gpio_configuration(self):
        r"""
        TODO
        """
        self.check_validity()

        return GetGPIOConfiguration(*self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_GPIO_CONFIGURATION, (), '', 11, 'B B B'))

    def get_data_storage(self, page):
        r"""
        TODO
        """
        self.check_validity()

        page = int(page)

        return self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_DATA_STORAGE, (page,), 'B', 71, '63B')

    def set_data_storage(self, page, data):
        r"""
        TODO
        """
        self.check_validity()

        page = int(page)
        data = list(map(int, data))

        self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_SET_DATA_STORAGE, (page, data), 'B 63B', 0, '')

    def get_indicator_led(self):
        r"""
        TODO
        """
        self.check_validity()

        return GetIndicatorLED(*self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_INDICATOR_LED, (), '', 16, 'h H H B B'))

    def set_indicator_led(self, indication, duration, color_h, color_s, color_v):
        r"""
        TODO
        """
        self.check_validity()

        indication = int(indication)
        duration = int(duration)
        color_h = int(color_h)
        color_s = int(color_s)
        color_v = int(color_v)

        return self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_SET_INDICATOR_LED, (indication, duration, color_h, color_s, color_v), 'h H H B B', 9, 'B')

    def set_button_configuration(self, button_configuration):
        r"""
        TODO
        """
        self.check_validity()

        button_configuration = int(button_configuration)

        self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_SET_BUTTON_CONFIGURATION, (button_configuration,), 'B', 0, '')

    def get_button_configuration(self):
        r"""
        TODO
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_BUTTON_CONFIGURATION, (), '', 9, 'B')

    def get_button_state(self):
        r"""
        TODO
        """
        self.check_validity()

        return GetButtonState(*self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_BUTTON_STATE, (), '', 17, 'I I !'))

    def set_ev_wakeup(self, ev_wakeup_enabled):
        r"""
        TODO
        """
        self.check_validity()

        ev_wakeup_enabled = bool(ev_wakeup_enabled)

        self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_SET_EV_WAKEUP, (ev_wakeup_enabled,), '!', 0, '')

    def get_ev_wakuep(self):
        r"""
        TODO
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_EV_WAKUEP, (), '', 9, '!')

    def set_control_pilot_disconnect(self, control_pilot_disconnect):
        r"""
        TODO
        """
        self.check_validity()

        control_pilot_disconnect = bool(control_pilot_disconnect)

        return self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_SET_CONTROL_PILOT_DISCONNECT, (control_pilot_disconnect,), '!', 9, '!')

    def get_control_pilot_disconnect(self):
        r"""
        TODO
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_CONTROL_PILOT_DISCONNECT, (), '', 9, '!')

    def get_all_data_1(self):
        r"""
        TODO
        """
        self.check_validity()

        return GetAllData1(*self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_ALL_DATA_1, (), '', 63, 'B B B B H B B B B ! B B f 3f 3! 3! 6I'))

    def get_all_data_2(self):
        r"""
        TODO
        """
        self.check_validity()

        return GetAllData2(*self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_ALL_DATA_2, (), '', 39, 'B B B h H H B B B I I ! ! ! ! h B B B B !'))

    def factory_reset(self, password):
        r"""
        TODO
        """
        self.check_validity()

        password = int(password)

        self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_FACTORY_RESET, (password,), 'I', 0, '')

    def get_button_press_boot_time(self, reset):
        r"""
        TODO
        """
        self.check_validity()

        reset = bool(reset)

        return self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_BUTTON_PRESS_BOOT_TIME, (reset,), '!', 12, 'I')

    def set_boost_mode(self, boost_mode_enabled):
        r"""
        TODO
        """
        self.check_validity()

        boost_mode_enabled = bool(boost_mode_enabled)

        self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_SET_BOOST_MODE, (boost_mode_enabled,), '!', 0, '')

    def get_boost_mode(self):
        r"""
        TODO
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_BOOST_MODE, (), '', 9, '!')

    def trigger_dc_fault_test(self, password):
        r"""
        TODO
        """
        self.check_validity()

        password = int(password)

        return self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_TRIGGER_DC_FAULT_TEST, (password,), 'I', 9, '!')

    def set_gp_output(self, gp_output):
        r"""
        TODO
        """
        self.check_validity()

        gp_output = int(gp_output)

        self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_SET_GP_OUTPUT, (gp_output,), 'B', 0, '')

    def get_temperature(self):
        r"""
        TODO
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_TEMPERATURE, (), '', 10, 'h')

    def set_phase_control(self, phases):
        r"""
        TODO
        """
        self.check_validity()

        phases = int(phases)

        self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_SET_PHASE_CONTROL, (phases,), 'B', 0, '')

    def get_phase_control(self):
        r"""
        TODO
        """
        self.check_validity()

        return GetPhaseControl(*self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_PHASE_CONTROL, (), '', 12, 'B B B B'))

    def set_phase_auto_switch(self, phase_auto_switch_enabled):
        r"""
        TODO
        """
        self.check_validity()

        phase_auto_switch_enabled = bool(phase_auto_switch_enabled)

        self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_SET_PHASE_AUTO_SWITCH, (phase_auto_switch_enabled,), '!', 0, '')

    def get_phase_auto_switch(self):
        r"""
        TODO
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_PHASE_AUTO_SWITCH, (), '', 9, '!')

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletEVSEV2.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def get_all_energy_meter_values(self):
        r"""
        TBD
        """
        values_length = 88

        with self.stream_lock:
            ret = self.get_all_energy_meter_values_low_level()

            if ret.values_chunk_offset == (1 << 16) - 1: # maximum chunk offset -> stream has no data
                values_length = 0
                values_out_of_sync = False
                values_data = ()
            else:
                values_out_of_sync = ret.values_chunk_offset != 0
                values_data = ret.values_chunk_data

            while not values_out_of_sync and len(values_data) < values_length:
                ret = self.get_all_energy_meter_values_low_level()
                values_out_of_sync = ret.values_chunk_offset != len(values_data)
                values_data += ret.values_chunk_data

            if values_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.values_chunk_offset + 15 < values_length:
                    ret = self.get_all_energy_meter_values_low_level()

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Values stream is out-of-sync')

        return values_data[:values_length]

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

EVSEV2 = BrickletEVSEV2 # for backward compatibility
