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

GetAirPressureCallbackConfiguration = namedtuple('AirPressureCallbackConfiguration', ['period', 'value_has_to_change', 'option', 'min', 'max'])
GetAltitudeCallbackConfiguration = namedtuple('AltitudeCallbackConfiguration', ['period', 'value_has_to_change', 'option', 'min', 'max'])
GetTemperatureCallbackConfiguration = namedtuple('TemperatureCallbackConfiguration', ['period', 'value_has_to_change', 'option', 'min', 'max'])
GetMovingAverageConfiguration = namedtuple('MovingAverageConfiguration', ['moving_average_length_air_pressure', 'moving_average_length_temperature'])
GetCalibration = namedtuple('Calibration', ['measured_air_pressure', 'actual_air_pressure'])
GetSensorConfiguration = namedtuple('SensorConfiguration', ['data_rate', 'air_pressure_low_pass_filter'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletBarometerV2(Device):
    r"""
    Measures air pressure and altitude changes
    """

    DEVICE_IDENTIFIER = 2117
    DEVICE_DISPLAY_NAME = 'Barometer Bricklet 2.0'
    DEVICE_URL_PART = 'barometer_v2' # internal

    CALLBACK_AIR_PRESSURE = 4
    CALLBACK_ALTITUDE = 8
    CALLBACK_TEMPERATURE = 12


    FUNCTION_GET_AIR_PRESSURE = 1
    FUNCTION_SET_AIR_PRESSURE_CALLBACK_CONFIGURATION = 2
    FUNCTION_GET_AIR_PRESSURE_CALLBACK_CONFIGURATION = 3
    FUNCTION_GET_ALTITUDE = 5
    FUNCTION_SET_ALTITUDE_CALLBACK_CONFIGURATION = 6
    FUNCTION_GET_ALTITUDE_CALLBACK_CONFIGURATION = 7
    FUNCTION_GET_TEMPERATURE = 9
    FUNCTION_SET_TEMPERATURE_CALLBACK_CONFIGURATION = 10
    FUNCTION_GET_TEMPERATURE_CALLBACK_CONFIGURATION = 11
    FUNCTION_SET_MOVING_AVERAGE_CONFIGURATION = 13
    FUNCTION_GET_MOVING_AVERAGE_CONFIGURATION = 14
    FUNCTION_SET_REFERENCE_AIR_PRESSURE = 15
    FUNCTION_GET_REFERENCE_AIR_PRESSURE = 16
    FUNCTION_SET_CALIBRATION = 17
    FUNCTION_GET_CALIBRATION = 18
    FUNCTION_SET_SENSOR_CONFIGURATION = 19
    FUNCTION_GET_SENSOR_CONFIGURATION = 20
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

    THRESHOLD_OPTION_OFF = 'x'
    THRESHOLD_OPTION_OUTSIDE = 'o'
    THRESHOLD_OPTION_INSIDE = 'i'
    THRESHOLD_OPTION_SMALLER = '<'
    THRESHOLD_OPTION_GREATER = '>'
    DATA_RATE_OFF = 0
    DATA_RATE_1HZ = 1
    DATA_RATE_10HZ = 2
    DATA_RATE_25HZ = 3
    DATA_RATE_50HZ = 4
    DATA_RATE_75HZ = 5
    LOW_PASS_FILTER_OFF = 0
    LOW_PASS_FILTER_1_9TH = 1
    LOW_PASS_FILTER_1_20TH = 2
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
        Device.__init__(self, uid, ipcon, BrickletBarometerV2.DEVICE_IDENTIFIER, BrickletBarometerV2.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletBarometerV2.FUNCTION_GET_AIR_PRESSURE] = BrickletBarometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometerV2.FUNCTION_SET_AIR_PRESSURE_CALLBACK_CONFIGURATION] = BrickletBarometerV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletBarometerV2.FUNCTION_GET_AIR_PRESSURE_CALLBACK_CONFIGURATION] = BrickletBarometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometerV2.FUNCTION_GET_ALTITUDE] = BrickletBarometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometerV2.FUNCTION_SET_ALTITUDE_CALLBACK_CONFIGURATION] = BrickletBarometerV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletBarometerV2.FUNCTION_GET_ALTITUDE_CALLBACK_CONFIGURATION] = BrickletBarometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometerV2.FUNCTION_GET_TEMPERATURE] = BrickletBarometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometerV2.FUNCTION_SET_TEMPERATURE_CALLBACK_CONFIGURATION] = BrickletBarometerV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletBarometerV2.FUNCTION_GET_TEMPERATURE_CALLBACK_CONFIGURATION] = BrickletBarometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometerV2.FUNCTION_SET_MOVING_AVERAGE_CONFIGURATION] = BrickletBarometerV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletBarometerV2.FUNCTION_GET_MOVING_AVERAGE_CONFIGURATION] = BrickletBarometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometerV2.FUNCTION_SET_REFERENCE_AIR_PRESSURE] = BrickletBarometerV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletBarometerV2.FUNCTION_GET_REFERENCE_AIR_PRESSURE] = BrickletBarometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometerV2.FUNCTION_SET_CALIBRATION] = BrickletBarometerV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletBarometerV2.FUNCTION_GET_CALIBRATION] = BrickletBarometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometerV2.FUNCTION_SET_SENSOR_CONFIGURATION] = BrickletBarometerV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletBarometerV2.FUNCTION_GET_SENSOR_CONFIGURATION] = BrickletBarometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometerV2.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletBarometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometerV2.FUNCTION_SET_BOOTLOADER_MODE] = BrickletBarometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometerV2.FUNCTION_GET_BOOTLOADER_MODE] = BrickletBarometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometerV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletBarometerV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletBarometerV2.FUNCTION_WRITE_FIRMWARE] = BrickletBarometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometerV2.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletBarometerV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletBarometerV2.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletBarometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometerV2.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletBarometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometerV2.FUNCTION_RESET] = BrickletBarometerV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletBarometerV2.FUNCTION_WRITE_UID] = BrickletBarometerV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletBarometerV2.FUNCTION_READ_UID] = BrickletBarometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletBarometerV2.FUNCTION_GET_IDENTITY] = BrickletBarometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletBarometerV2.CALLBACK_AIR_PRESSURE] = (12, 'i')
        self.callback_formats[BrickletBarometerV2.CALLBACK_ALTITUDE] = (12, 'i')
        self.callback_formats[BrickletBarometerV2.CALLBACK_TEMPERATURE] = (12, 'i')

        ipcon.add_device(self)

    def get_air_pressure(self):
        r"""
        Returns the measured air pressure.


        If you want to get the value periodically, it is recommended to use the
        :cb:`Air Pressure` callback. You can set the callback configuration
        with :func:`Set Air Pressure Callback Configuration`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_GET_AIR_PRESSURE, (), '', 12, 'i')

    def set_air_pressure_callback_configuration(self, period, value_has_to_change, option, min, max):
        r"""
        The period is the period with which the :cb:`Air Pressure` callback is triggered
        periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change
        within the period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.

        It is furthermore possible to constrain the callback with thresholds.

        The `option`-parameter together with min/max sets a threshold for the :cb:`Air Pressure` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Threshold is turned off"
         "'o'",    "Threshold is triggered when the value is *outside* the min and max values"
         "'i'",    "Threshold is triggered when the value is *inside* or equal to the min and max values"
         "'<'",    "Threshold is triggered when the value is smaller than the min value (max is ignored)"
         "'>'",    "Threshold is triggered when the value is greater than the min value (max is ignored)"

        If the option is set to 'x' (threshold turned off) the callback is triggered with the fixed period.
        """
        self.check_validity()

        period = int(period)
        value_has_to_change = bool(value_has_to_change)
        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_SET_AIR_PRESSURE_CALLBACK_CONFIGURATION, (period, value_has_to_change, option, min, max), 'I ! c i i', 0, '')

    def get_air_pressure_callback_configuration(self):
        r"""
        Returns the callback configuration as set by :func:`Set Air Pressure Callback Configuration`.
        """
        self.check_validity()

        return GetAirPressureCallbackConfiguration(*self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_GET_AIR_PRESSURE_CALLBACK_CONFIGURATION, (), '', 22, 'I ! c i i'))

    def get_altitude(self):
        r"""
        Returns the relative altitude of the air pressure sensor. The value
        is calculated based on the difference between the
        current air pressure and the reference air pressure that can be set
        with :func:`Set Reference Air Pressure`.


        If you want to get the value periodically, it is recommended to use the
        :cb:`Altitude` callback. You can set the callback configuration
        with :func:`Set Altitude Callback Configuration`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_GET_ALTITUDE, (), '', 12, 'i')

    def set_altitude_callback_configuration(self, period, value_has_to_change, option, min, max):
        r"""
        The period is the period with which the :cb:`Altitude` callback is triggered
        periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change
        within the period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.

        It is furthermore possible to constrain the callback with thresholds.

        The `option`-parameter together with min/max sets a threshold for the :cb:`Altitude` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Threshold is turned off"
         "'o'",    "Threshold is triggered when the value is *outside* the min and max values"
         "'i'",    "Threshold is triggered when the value is *inside* or equal to the min and max values"
         "'<'",    "Threshold is triggered when the value is smaller than the min value (max is ignored)"
         "'>'",    "Threshold is triggered when the value is greater than the min value (max is ignored)"

        If the option is set to 'x' (threshold turned off) the callback is triggered with the fixed period.
        """
        self.check_validity()

        period = int(period)
        value_has_to_change = bool(value_has_to_change)
        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_SET_ALTITUDE_CALLBACK_CONFIGURATION, (period, value_has_to_change, option, min, max), 'I ! c i i', 0, '')

    def get_altitude_callback_configuration(self):
        r"""
        Returns the callback configuration as set by :func:`Set Altitude Callback Configuration`.
        """
        self.check_validity()

        return GetAltitudeCallbackConfiguration(*self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_GET_ALTITUDE_CALLBACK_CONFIGURATION, (), '', 22, 'I ! c i i'))

    def get_temperature(self):
        r"""
        Returns the temperature of the air pressure sensor.

        This temperature is used internally for temperature compensation
        of the air pressure measurement. It is not as accurate as the
        temperature measured by the :ref:`temperature_v2_bricklet` or the
        :ref:`temperature_ir_v2_bricklet`.


        If you want to get the value periodically, it is recommended to use the
        :cb:`Temperature` callback. You can set the callback configuration
        with :func:`Set Temperature Callback Configuration`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_GET_TEMPERATURE, (), '', 12, 'i')

    def set_temperature_callback_configuration(self, period, value_has_to_change, option, min, max):
        r"""
        The period is the period with which the :cb:`Temperature` callback is triggered
        periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change
        within the period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.

        It is furthermore possible to constrain the callback with thresholds.

        The `option`-parameter together with min/max sets a threshold for the :cb:`Temperature` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Threshold is turned off"
         "'o'",    "Threshold is triggered when the value is *outside* the min and max values"
         "'i'",    "Threshold is triggered when the value is *inside* or equal to the min and max values"
         "'<'",    "Threshold is triggered when the value is smaller than the min value (max is ignored)"
         "'>'",    "Threshold is triggered when the value is greater than the min value (max is ignored)"

        If the option is set to 'x' (threshold turned off) the callback is triggered with the fixed period.
        """
        self.check_validity()

        period = int(period)
        value_has_to_change = bool(value_has_to_change)
        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_SET_TEMPERATURE_CALLBACK_CONFIGURATION, (period, value_has_to_change, option, min, max), 'I ! c i i', 0, '')

    def get_temperature_callback_configuration(self):
        r"""
        Returns the callback configuration as set by :func:`Set Temperature Callback Configuration`.
        """
        self.check_validity()

        return GetTemperatureCallbackConfiguration(*self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_GET_TEMPERATURE_CALLBACK_CONFIGURATION, (), '', 22, 'I ! c i i'))

    def set_moving_average_configuration(self, moving_average_length_air_pressure, moving_average_length_temperature):
        r"""
        Sets the length of a `moving averaging <https://en.wikipedia.org/wiki/Moving_average>`__
        for the air pressure and temperature measurements.

        Setting the length to 1 will turn the averaging off. With less
        averaging, there is more noise on the data.

        If you want to do long term measurements the longest moving average will give
        the cleanest results.
        """
        self.check_validity()

        moving_average_length_air_pressure = int(moving_average_length_air_pressure)
        moving_average_length_temperature = int(moving_average_length_temperature)

        self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_SET_MOVING_AVERAGE_CONFIGURATION, (moving_average_length_air_pressure, moving_average_length_temperature), 'H H', 0, '')

    def get_moving_average_configuration(self):
        r"""
        Returns the moving average configuration as set by
        :func:`Set Moving Average Configuration`.
        """
        self.check_validity()

        return GetMovingAverageConfiguration(*self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_GET_MOVING_AVERAGE_CONFIGURATION, (), '', 12, 'H H'))

    def set_reference_air_pressure(self, air_pressure):
        r"""
        Sets the reference air pressure for the altitude calculation.
        Setting the reference to the
        current air pressure results in a calculated altitude of 0mm. Passing 0 is
        a shortcut for passing the current air pressure as reference.

        Well known reference values are the Q codes
        `QNH <https://en.wikipedia.org/wiki/QNH>`__ and
        `QFE <https://en.wikipedia.org/wiki/Mean_sea_level_pressure#Mean_sea_level_pressure>`__
        used in aviation.
        """
        self.check_validity()

        air_pressure = int(air_pressure)

        self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_SET_REFERENCE_AIR_PRESSURE, (air_pressure,), 'i', 0, '')

    def get_reference_air_pressure(self):
        r"""
        Returns the reference air pressure as set by :func:`Set Reference Air Pressure`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_GET_REFERENCE_AIR_PRESSURE, (), '', 12, 'i')

    def set_calibration(self, measured_air_pressure, actual_air_pressure):
        r"""
        Sets the one point calibration (OPC) values for the air pressure measurement.

        Before the Bricklet can be calibrated any previous calibration has to be removed
        by setting ``measured air pressure`` and ``actual air pressure`` to 0.

        Then the current air pressure has to be measured using the Bricklet
        (``measured air pressure``) and with an accurate reference barometer
        (``actual air pressure``) at the same time and passed to this function.

        After proper calibration the air pressure measurement can achieve an accuracy
        up to 0.2 hPa.

        The calibration is saved in the EEPROM of the Bricklet and only needs to be
        configured once.
        """
        self.check_validity()

        measured_air_pressure = int(measured_air_pressure)
        actual_air_pressure = int(actual_air_pressure)

        self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_SET_CALIBRATION, (measured_air_pressure, actual_air_pressure), 'i i', 0, '')

    def get_calibration(self):
        r"""
        Returns the air pressure one point calibration values as set by
        :func:`Set Calibration`.
        """
        self.check_validity()

        return GetCalibration(*self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_GET_CALIBRATION, (), '', 16, 'i i'))

    def set_sensor_configuration(self, data_rate, air_pressure_low_pass_filter):
        r"""
        Configures the data rate and air pressure low pass filter. The low pass filter
        cut-off frequency (if enabled) can be set to 1/9th or 1/20th of the configure
        data rate to decrease the noise on the air pressure data.

        The low pass filter configuration only applies to the air pressure measurement.
        There is no low pass filter for the temperature measurement.

        A higher data rate will result in a less precise temperature because of
        self-heating of the sensor. If the accuracy of the temperature reading is
        important to you, we would recommend the 1Hz data rate.
        """
        self.check_validity()

        data_rate = int(data_rate)
        air_pressure_low_pass_filter = int(air_pressure_low_pass_filter)

        self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_SET_SENSOR_CONFIGURATION, (data_rate, air_pressure_low_pass_filter), 'B B', 0, '')

    def get_sensor_configuration(self):
        r"""
        Returns the sensor configuration as set by :func:`Set Sensor Configuration`.
        """
        self.check_validity()

        return GetSensorConfiguration(*self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_GET_SENSOR_CONFIGURATION, (), '', 10, 'B B'))

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletBarometerV2.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

BarometerV2 = BrickletBarometerV2 # for backward compatibility
