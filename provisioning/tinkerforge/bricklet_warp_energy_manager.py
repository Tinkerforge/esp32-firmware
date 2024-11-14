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

GetRGBValue = namedtuple('RGBValue', ['r', 'g', 'b'])
GetEnergyMeterValues = namedtuple('EnergyMeterValues', ['power', 'current'])
GetEnergyMeterDetailedValuesLowLevel = namedtuple('EnergyMeterDetailedValuesLowLevel', ['values_chunk_offset', 'values_chunk_data'])
GetEnergyMeterState = namedtuple('EnergyMeterState', ['energy_meter_type', 'error_count'])
GetAllData1 = namedtuple('AllData1', ['contactor_value', 'r', 'g', 'b', 'power', 'current', 'energy_meter_type', 'error_count', 'input', 'output', 'voltage', 'contactor_check_state', 'uptime'])
GetSDInformation = namedtuple('SDInformation', ['sd_status', 'lfs_status', 'sector_size', 'sector_count', 'card_type', 'product_rev', 'product_name', 'manufacturer_id'])
GetDateTime = namedtuple('DateTime', ['seconds', 'minutes', 'hours', 'days', 'days_of_week', 'month', 'year'])
GetLEDState = namedtuple('LEDState', ['pattern', 'hue'])
GetDataStorage = namedtuple('DataStorage', ['status', 'data'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletWARPEnergyManager(Device):
    r"""
    TBD
    """

    DEVICE_IDENTIFIER = 2169
    DEVICE_DISPLAY_NAME = 'WARP Energy Manager Bricklet'
    DEVICE_URL_PART = 'warp_energy_manager' # internal

    CALLBACK_SD_WALLBOX_DATA_POINTS_LOW_LEVEL = 24
    CALLBACK_SD_WALLBOX_DAILY_DATA_POINTS_LOW_LEVEL = 25
    CALLBACK_SD_ENERGY_MANAGER_DATA_POINTS_LOW_LEVEL = 26
    CALLBACK_SD_ENERGY_MANAGER_DAILY_DATA_POINTS_LOW_LEVEL = 27

    CALLBACK_SD_WALLBOX_DATA_POINTS = -24
    CALLBACK_SD_WALLBOX_DAILY_DATA_POINTS = -25
    CALLBACK_SD_ENERGY_MANAGER_DATA_POINTS = -26
    CALLBACK_SD_ENERGY_MANAGER_DAILY_DATA_POINTS = -27

    FUNCTION_SET_CONTACTOR = 1
    FUNCTION_GET_CONTACTOR = 2
    FUNCTION_SET_RGB_VALUE = 3
    FUNCTION_GET_RGB_VALUE = 4
    FUNCTION_GET_ENERGY_METER_VALUES = 5
    FUNCTION_GET_ENERGY_METER_DETAILED_VALUES_LOW_LEVEL = 6
    FUNCTION_GET_ENERGY_METER_STATE = 7
    FUNCTION_GET_INPUT = 8
    FUNCTION_SET_OUTPUT = 9
    FUNCTION_GET_OUTPUT = 10
    FUNCTION_GET_INPUT_VOLTAGE = 11
    FUNCTION_GET_STATE = 12
    FUNCTION_GET_UPTIME = 13
    FUNCTION_GET_ALL_DATA_1 = 14
    FUNCTION_GET_SD_INFORMATION = 15
    FUNCTION_SET_SD_WALLBOX_DATA_POINT = 16
    FUNCTION_GET_SD_WALLBOX_DATA_POINTS = 17
    FUNCTION_SET_SD_WALLBOX_DAILY_DATA_POINT = 18
    FUNCTION_GET_SD_WALLBOX_DAILY_DATA_POINTS = 19
    FUNCTION_SET_SD_ENERGY_MANAGER_DATA_POINT = 20
    FUNCTION_GET_SD_ENERGY_MANAGER_DATA_POINTS = 21
    FUNCTION_SET_SD_ENERGY_MANAGER_DAILY_DATA_POINT = 22
    FUNCTION_GET_SD_ENERGY_MANAGER_DAILY_DATA_POINTS = 23
    FUNCTION_FORMAT_SD = 28
    FUNCTION_SET_DATE_TIME = 29
    FUNCTION_GET_DATE_TIME = 30
    FUNCTION_SET_LED_STATE = 31
    FUNCTION_GET_LED_STATE = 32
    FUNCTION_GET_DATA_STORAGE = 33
    FUNCTION_SET_DATA_STORAGE = 34
    FUNCTION_RESET_ENERGY_METER_RELATIVE_ENERGY = 35
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

    ENERGY_METER_TYPE_NOT_AVAILABLE = 0
    ENERGY_METER_TYPE_SDM72 = 1
    ENERGY_METER_TYPE_SDM630 = 2
    ENERGY_METER_TYPE_SDM72V2 = 3
    ENERGY_METER_TYPE_SDM72CTM = 4
    ENERGY_METER_TYPE_SDM630MCTV2 = 5
    ENERGY_METER_TYPE_DSZ15DZMOD = 6
    ENERGY_METER_TYPE_DEM4A = 7
    DATA_STATUS_OK = 0
    DATA_STATUS_SD_ERROR = 1
    DATA_STATUS_LFS_ERROR = 2
    DATA_STATUS_QUEUE_FULL = 3
    DATA_STATUS_DATE_OUT_OF_RANGE = 4
    FORMAT_STATUS_OK = 0
    FORMAT_STATUS_PASSWORD_ERROR = 1
    FORMAT_STATUS_FORMAT_ERROR = 2
    LED_PATTERN_OFF = 0
    LED_PATTERN_ON = 1
    LED_PATTERN_BLINKING = 2
    LED_PATTERN_BREATHING = 3
    DATA_STORAGE_STATUS_OK = 0
    DATA_STORAGE_STATUS_NOT_FOUND = 1
    DATA_STORAGE_STATUS_BUSY = 2
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
        Device.__init__(self, uid, ipcon, BrickletWARPEnergyManager.DEVICE_IDENTIFIER, BrickletWARPEnergyManager.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletWARPEnergyManager.FUNCTION_SET_CONTACTOR] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_GET_CONTACTOR] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_SET_RGB_VALUE] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_GET_RGB_VALUE] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_GET_ENERGY_METER_VALUES] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_GET_ENERGY_METER_DETAILED_VALUES_LOW_LEVEL] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_GET_ENERGY_METER_STATE] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_GET_INPUT] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_SET_OUTPUT] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_GET_OUTPUT] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_GET_INPUT_VOLTAGE] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_GET_STATE] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_GET_UPTIME] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_GET_ALL_DATA_1] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_GET_SD_INFORMATION] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_SET_SD_WALLBOX_DATA_POINT] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_GET_SD_WALLBOX_DATA_POINTS] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_SET_SD_WALLBOX_DAILY_DATA_POINT] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_GET_SD_WALLBOX_DAILY_DATA_POINTS] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_SET_SD_ENERGY_MANAGER_DATA_POINT] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_GET_SD_ENERGY_MANAGER_DATA_POINTS] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_SET_SD_ENERGY_MANAGER_DAILY_DATA_POINT] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_GET_SD_ENERGY_MANAGER_DAILY_DATA_POINTS] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_FORMAT_SD] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_SET_DATE_TIME] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_GET_DATE_TIME] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_SET_LED_STATE] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_GET_LED_STATE] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_GET_DATA_STORAGE] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_SET_DATA_STORAGE] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_RESET_ENERGY_METER_RELATIVE_ENERGY] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_SET_BOOTLOADER_MODE] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_GET_BOOTLOADER_MODE] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_WRITE_FIRMWARE] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_RESET] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_WRITE_UID] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_READ_UID] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletWARPEnergyManager.FUNCTION_GET_IDENTITY] = BrickletWARPEnergyManager.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletWARPEnergyManager.CALLBACK_SD_WALLBOX_DATA_POINTS_LOW_LEVEL] = (72, 'H H 60B')
        self.callback_formats[BrickletWARPEnergyManager.CALLBACK_SD_WALLBOX_DAILY_DATA_POINTS_LOW_LEVEL] = (72, 'H H 15I')
        self.callback_formats[BrickletWARPEnergyManager.CALLBACK_SD_ENERGY_MANAGER_DATA_POINTS_LOW_LEVEL] = (45, 'H H 33B')
        self.callback_formats[BrickletWARPEnergyManager.CALLBACK_SD_ENERGY_MANAGER_DAILY_DATA_POINTS_LOW_LEVEL] = (72, 'H H 15I')

        self.high_level_callbacks[BrickletWARPEnergyManager.CALLBACK_SD_WALLBOX_DATA_POINTS] = [('stream_length', 'stream_chunk_offset', 'stream_chunk_data'), {'fixed_length': None, 'single_chunk': False}, None]
        self.high_level_callbacks[BrickletWARPEnergyManager.CALLBACK_SD_WALLBOX_DAILY_DATA_POINTS] = [('stream_length', 'stream_chunk_offset', 'stream_chunk_data'), {'fixed_length': None, 'single_chunk': False}, None]
        self.high_level_callbacks[BrickletWARPEnergyManager.CALLBACK_SD_ENERGY_MANAGER_DATA_POINTS] = [('stream_length', 'stream_chunk_offset', 'stream_chunk_data'), {'fixed_length': None, 'single_chunk': False}, None]
        self.high_level_callbacks[BrickletWARPEnergyManager.CALLBACK_SD_ENERGY_MANAGER_DAILY_DATA_POINTS] = [('stream_length', 'stream_chunk_offset', 'stream_chunk_data'), {'fixed_length': None, 'single_chunk': False}, None]
        ipcon.add_device(self)

    def set_contactor(self, contactor_value):
        r"""
        TBD
        """
        self.check_validity()

        contactor_value = bool(contactor_value)

        self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_SET_CONTACTOR, (contactor_value,), '!', 0, '')

    def get_contactor(self):
        r"""
        TBD
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_GET_CONTACTOR, (), '', 9, '!')

    def set_rgb_value(self, r, g, b):
        r"""
        Sets the *r*, *g* and *b* values for the LED.
        """
        self.check_validity()

        r = int(r)
        g = int(g)
        b = int(b)

        self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_SET_RGB_VALUE, (r, g, b), 'B B B', 0, '')

    def get_rgb_value(self):
        r"""
        Returns the *r*, *g* and *b* values of the LED as set by :func:`Set RGB Value`.
        """
        self.check_validity()

        return GetRGBValue(*self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_GET_RGB_VALUE, (), '', 11, 'B B B'))

    def get_energy_meter_values(self):
        r"""
        TODO
        """
        self.check_validity()

        return GetEnergyMeterValues(*self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_GET_ENERGY_METER_VALUES, (), '', 24, 'f 3f'))

    def get_energy_meter_detailed_values_low_level(self):
        r"""
        TBD
        """
        self.check_validity()

        return GetEnergyMeterDetailedValuesLowLevel(*self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_GET_ENERGY_METER_DETAILED_VALUES_LOW_LEVEL, (), '', 70, 'H 15f'))

    def get_energy_meter_state(self):
        r"""
        TODO
        """
        self.check_validity()

        return GetEnergyMeterState(*self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_GET_ENERGY_METER_STATE, (), '', 33, 'B 6I'))

    def get_input(self):
        r"""
        TODO
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_GET_INPUT, (), '', 9, '2!')

    def set_output(self, output):
        r"""
        TODO
        """
        self.check_validity()

        output = bool(output)

        self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_SET_OUTPUT, (output,), '!', 0, '')

    def get_output(self):
        r"""
        TODO
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_GET_OUTPUT, (), '', 9, '!')

    def get_input_voltage(self):
        r"""
        TODO
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_GET_INPUT_VOLTAGE, (), '', 10, 'H')

    def get_state(self):
        r"""
        TODO
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_GET_STATE, (), '', 9, 'B')

    def get_uptime(self):
        r"""
        TODO
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_GET_UPTIME, (), '', 12, 'I')

    def get_all_data_1(self):
        r"""
        TODO
        """
        self.check_validity()

        return GetAllData1(*self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_GET_ALL_DATA_1, (), '', 62, '! B B B f 3f B 6I 2! ! H B I'))

    def get_sd_information(self):
        r"""
        TODO
        """
        self.check_validity()

        return GetSDInformation(*self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_GET_SD_INFORMATION, (), '', 33, 'I I H I I B 5c B'))

    def set_sd_wallbox_data_point(self, wallbox_id, year, month, day, hour, minute, flags, power):
        r"""
        TODO
        """
        self.check_validity()

        wallbox_id = int(wallbox_id)
        year = int(year)
        month = int(month)
        day = int(day)
        hour = int(hour)
        minute = int(minute)
        flags = int(flags)
        power = int(power)

        return self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_SET_SD_WALLBOX_DATA_POINT, (wallbox_id, year, month, day, hour, minute, flags, power), 'I B B B B B B H', 9, 'B')

    def get_sd_wallbox_data_points(self, wallbox_id, year, month, day, hour, minute, amount):
        r"""
        TODO
        """
        self.check_validity()

        wallbox_id = int(wallbox_id)
        year = int(year)
        month = int(month)
        day = int(day)
        hour = int(hour)
        minute = int(minute)
        amount = int(amount)

        return self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_GET_SD_WALLBOX_DATA_POINTS, (wallbox_id, year, month, day, hour, minute, amount), 'I B B B B B H', 9, 'B')

    def set_sd_wallbox_daily_data_point(self, wallbox_id, year, month, day, energy):
        r"""
        TODO
        """
        self.check_validity()

        wallbox_id = int(wallbox_id)
        year = int(year)
        month = int(month)
        day = int(day)
        energy = int(energy)

        return self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_SET_SD_WALLBOX_DAILY_DATA_POINT, (wallbox_id, year, month, day, energy), 'I B B B I', 9, 'B')

    def get_sd_wallbox_daily_data_points(self, wallbox_id, year, month, day, amount):
        r"""
        TODO
        """
        self.check_validity()

        wallbox_id = int(wallbox_id)
        year = int(year)
        month = int(month)
        day = int(day)
        amount = int(amount)

        return self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_GET_SD_WALLBOX_DAILY_DATA_POINTS, (wallbox_id, year, month, day, amount), 'I B B B B', 9, 'B')

    def set_sd_energy_manager_data_point(self, year, month, day, hour, minute, flags, power_grid, power_general, price):
        r"""
        TODO
        """
        self.check_validity()

        year = int(year)
        month = int(month)
        day = int(day)
        hour = int(hour)
        minute = int(minute)
        flags = int(flags)
        power_grid = int(power_grid)
        power_general = list(map(int, power_general))
        price = int(price)

        return self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_SET_SD_ENERGY_MANAGER_DATA_POINT, (year, month, day, hour, minute, flags, power_grid, power_general, price), 'B B B B B B i 6i I', 9, 'B')

    def get_sd_energy_manager_data_points(self, year, month, day, hour, minute, amount):
        r"""
        TODO
        """
        self.check_validity()

        year = int(year)
        month = int(month)
        day = int(day)
        hour = int(hour)
        minute = int(minute)
        amount = int(amount)

        return self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_GET_SD_ENERGY_MANAGER_DATA_POINTS, (year, month, day, hour, minute, amount), 'B B B B B H', 9, 'B')

    def set_sd_energy_manager_daily_data_point(self, year, month, day, energy_grid_in, energy_grid_out, energy_general_in, energy_general_out, price):
        r"""
        TODO
        """
        self.check_validity()

        year = int(year)
        month = int(month)
        day = int(day)
        energy_grid_in = int(energy_grid_in)
        energy_grid_out = int(energy_grid_out)
        energy_general_in = list(map(int, energy_general_in))
        energy_general_out = list(map(int, energy_general_out))
        price = int(price)

        return self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_SET_SD_ENERGY_MANAGER_DAILY_DATA_POINT, (year, month, day, energy_grid_in, energy_grid_out, energy_general_in, energy_general_out, price), 'B B B I I 6I 6I I', 9, 'B')

    def get_sd_energy_manager_daily_data_points(self, year, month, day, amount):
        r"""
        TODO
        """
        self.check_validity()

        year = int(year)
        month = int(month)
        day = int(day)
        amount = int(amount)

        return self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_GET_SD_ENERGY_MANAGER_DAILY_DATA_POINTS, (year, month, day, amount), 'B B B B', 9, 'B')

    def format_sd(self, password):
        r"""
        TODO
        """
        self.check_validity()

        password = int(password)

        return self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_FORMAT_SD, (password,), 'I', 9, 'B')

    def set_date_time(self, seconds, minutes, hours, days, days_of_week, month, year):
        r"""
        TODO
        """
        self.check_validity()

        seconds = int(seconds)
        minutes = int(minutes)
        hours = int(hours)
        days = int(days)
        days_of_week = int(days_of_week)
        month = int(month)
        year = int(year)

        self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_SET_DATE_TIME, (seconds, minutes, hours, days, days_of_week, month, year), 'B B B B B B H', 0, '')

    def get_date_time(self):
        r"""
        TODO
        """
        self.check_validity()

        return GetDateTime(*self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_GET_DATE_TIME, (), '', 16, 'B B B B B B H'))

    def set_led_state(self, pattern, hue):
        r"""

        """
        self.check_validity()

        pattern = int(pattern)
        hue = int(hue)

        self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_SET_LED_STATE, (pattern, hue), 'B H', 0, '')

    def get_led_state(self):
        r"""

        """
        self.check_validity()

        return GetLEDState(*self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_GET_LED_STATE, (), '', 11, 'B H'))

    def get_data_storage(self, page):
        r"""
        TODO
        """
        self.check_validity()

        page = int(page)

        return GetDataStorage(*self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_GET_DATA_STORAGE, (page,), 'B', 72, 'B 63B'))

    def set_data_storage(self, page, data):
        r"""
        TODO
        """
        self.check_validity()

        page = int(page)
        data = list(map(int, data))

        self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_SET_DATA_STORAGE, (page, data), 'B 63B', 0, '')

    def reset_energy_meter_relative_energy(self):
        r"""
        TODO
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_RESET_ENERGY_METER_RELATIVE_ENERGY, (), '', 0, '')

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletWARPEnergyManager.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def get_energy_meter_detailed_values(self):
        r"""
        TBD
        """
        values_length = 88

        with self.stream_lock:
            ret = self.get_energy_meter_detailed_values_low_level()

            if ret.values_chunk_offset == (1 << 16) - 1: # maximum chunk offset -> stream has no data
                values_length = 0
                values_out_of_sync = False
                values_data = ()
            else:
                values_out_of_sync = ret.values_chunk_offset != 0
                values_data = ret.values_chunk_data

            while not values_out_of_sync and len(values_data) < values_length:
                ret = self.get_energy_meter_detailed_values_low_level()
                values_out_of_sync = ret.values_chunk_offset != len(values_data)
                values_data += ret.values_chunk_data

            if values_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.values_chunk_offset + 15 < values_length:
                    ret = self.get_energy_meter_detailed_values_low_level()

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

WARPEnergyManager = BrickletWARPEnergyManager # for backward compatibility
