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

GetAcceleration = namedtuple('Acceleration', ['x', 'y', 'z'])
GetConfiguration = namedtuple('Configuration', ['data_rate', 'full_scale'])
GetAccelerationCallbackConfiguration = namedtuple('AccelerationCallbackConfiguration', ['period', 'value_has_to_change'])
GetContinuousAccelerationConfiguration = namedtuple('ContinuousAccelerationConfiguration', ['enable_x', 'enable_y', 'enable_z', 'resolution'])
GetFilterConfiguration = namedtuple('FilterConfiguration', ['iir_bypass', 'low_pass_filter'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletAccelerometerV2(Device):
    r"""
    Measures acceleration in three axis
    """

    DEVICE_IDENTIFIER = 2130
    DEVICE_DISPLAY_NAME = 'Accelerometer Bricklet 2.0'
    DEVICE_URL_PART = 'accelerometer_v2' # internal

    CALLBACK_ACCELERATION = 8
    CALLBACK_CONTINUOUS_ACCELERATION_16_BIT = 11
    CALLBACK_CONTINUOUS_ACCELERATION_8_BIT = 12


    FUNCTION_GET_ACCELERATION = 1
    FUNCTION_SET_CONFIGURATION = 2
    FUNCTION_GET_CONFIGURATION = 3
    FUNCTION_SET_ACCELERATION_CALLBACK_CONFIGURATION = 4
    FUNCTION_GET_ACCELERATION_CALLBACK_CONFIGURATION = 5
    FUNCTION_SET_INFO_LED_CONFIG = 6
    FUNCTION_GET_INFO_LED_CONFIG = 7
    FUNCTION_SET_CONTINUOUS_ACCELERATION_CONFIGURATION = 9
    FUNCTION_GET_CONTINUOUS_ACCELERATION_CONFIGURATION = 10
    FUNCTION_SET_FILTER_CONFIGURATION = 13
    FUNCTION_GET_FILTER_CONFIGURATION = 14
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

    DATA_RATE_0_781HZ = 0
    DATA_RATE_1_563HZ = 1
    DATA_RATE_3_125HZ = 2
    DATA_RATE_6_2512HZ = 3
    DATA_RATE_12_5HZ = 4
    DATA_RATE_25HZ = 5
    DATA_RATE_50HZ = 6
    DATA_RATE_100HZ = 7
    DATA_RATE_200HZ = 8
    DATA_RATE_400HZ = 9
    DATA_RATE_800HZ = 10
    DATA_RATE_1600HZ = 11
    DATA_RATE_3200HZ = 12
    DATA_RATE_6400HZ = 13
    DATA_RATE_12800HZ = 14
    DATA_RATE_25600HZ = 15
    FULL_SCALE_2G = 0
    FULL_SCALE_4G = 1
    FULL_SCALE_8G = 2
    INFO_LED_CONFIG_OFF = 0
    INFO_LED_CONFIG_ON = 1
    INFO_LED_CONFIG_SHOW_HEARTBEAT = 2
    RESOLUTION_8BIT = 0
    RESOLUTION_16BIT = 1
    IIR_BYPASS_APPLIED = 0
    IIR_BYPASS_BYPASSED = 1
    LOW_PASS_FILTER_NINTH = 0
    LOW_PASS_FILTER_HALF = 1
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
        Device.__init__(self, uid, ipcon, BrickletAccelerometerV2.DEVICE_IDENTIFIER, BrickletAccelerometerV2.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 1)

        self.response_expected[BrickletAccelerometerV2.FUNCTION_GET_ACCELERATION] = BrickletAccelerometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAccelerometerV2.FUNCTION_SET_CONFIGURATION] = BrickletAccelerometerV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAccelerometerV2.FUNCTION_GET_CONFIGURATION] = BrickletAccelerometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAccelerometerV2.FUNCTION_SET_ACCELERATION_CALLBACK_CONFIGURATION] = BrickletAccelerometerV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletAccelerometerV2.FUNCTION_GET_ACCELERATION_CALLBACK_CONFIGURATION] = BrickletAccelerometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAccelerometerV2.FUNCTION_SET_INFO_LED_CONFIG] = BrickletAccelerometerV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAccelerometerV2.FUNCTION_GET_INFO_LED_CONFIG] = BrickletAccelerometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAccelerometerV2.FUNCTION_SET_CONTINUOUS_ACCELERATION_CONFIGURATION] = BrickletAccelerometerV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletAccelerometerV2.FUNCTION_GET_CONTINUOUS_ACCELERATION_CONFIGURATION] = BrickletAccelerometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAccelerometerV2.FUNCTION_SET_FILTER_CONFIGURATION] = BrickletAccelerometerV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAccelerometerV2.FUNCTION_GET_FILTER_CONFIGURATION] = BrickletAccelerometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAccelerometerV2.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletAccelerometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAccelerometerV2.FUNCTION_SET_BOOTLOADER_MODE] = BrickletAccelerometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAccelerometerV2.FUNCTION_GET_BOOTLOADER_MODE] = BrickletAccelerometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAccelerometerV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletAccelerometerV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAccelerometerV2.FUNCTION_WRITE_FIRMWARE] = BrickletAccelerometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAccelerometerV2.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletAccelerometerV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAccelerometerV2.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletAccelerometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAccelerometerV2.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletAccelerometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAccelerometerV2.FUNCTION_RESET] = BrickletAccelerometerV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAccelerometerV2.FUNCTION_WRITE_UID] = BrickletAccelerometerV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAccelerometerV2.FUNCTION_READ_UID] = BrickletAccelerometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAccelerometerV2.FUNCTION_GET_IDENTITY] = BrickletAccelerometerV2.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletAccelerometerV2.CALLBACK_ACCELERATION] = (20, 'i i i')
        self.callback_formats[BrickletAccelerometerV2.CALLBACK_CONTINUOUS_ACCELERATION_16_BIT] = (68, '30h')
        self.callback_formats[BrickletAccelerometerV2.CALLBACK_CONTINUOUS_ACCELERATION_8_BIT] = (68, '60b')

        ipcon.add_device(self)

    def get_acceleration(self):
        r"""
        Returns the acceleration in x, y and z direction. The values
        are given in gₙ/10000 (1gₙ = 9.80665m/s²). The range is
        configured with :func:`Set Configuration`.

        If you want to get the acceleration periodically, it is recommended
        to use the :cb:`Acceleration` callback and set the period with
        :func:`Set Acceleration Callback Configuration`.
        """
        self.check_validity()

        return GetAcceleration(*self.ipcon.send_request(self, BrickletAccelerometerV2.FUNCTION_GET_ACCELERATION, (), '', 20, 'i i i'))

    def set_configuration(self, data_rate, full_scale):
        r"""
        Configures the data rate and full scale range.
        Possible values are:

        * Data rate of 0.781Hz to 25600Hz.
        * Full scale range of ±2g up to ±8g.

        Decreasing data rate or full scale range will also decrease the noise on
        the data.
        """
        self.check_validity()

        data_rate = int(data_rate)
        full_scale = int(full_scale)

        self.ipcon.send_request(self, BrickletAccelerometerV2.FUNCTION_SET_CONFIGURATION, (data_rate, full_scale), 'B B', 0, '')

    def get_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Configuration`.
        """
        self.check_validity()

        return GetConfiguration(*self.ipcon.send_request(self, BrickletAccelerometerV2.FUNCTION_GET_CONFIGURATION, (), '', 10, 'B B'))

    def set_acceleration_callback_configuration(self, period, value_has_to_change):
        r"""
        The period is the period with which the :cb:`Acceleration`
        callback is triggered periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change within the
        period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.

        If this callback is enabled, the :cb:`Continuous Acceleration 16 Bit` callback
        and :cb:`Continuous Acceleration 8 Bit` callback will automatically be disabled.
        """
        self.check_validity()

        period = int(period)
        value_has_to_change = bool(value_has_to_change)

        self.ipcon.send_request(self, BrickletAccelerometerV2.FUNCTION_SET_ACCELERATION_CALLBACK_CONFIGURATION, (period, value_has_to_change), 'I !', 0, '')

    def get_acceleration_callback_configuration(self):
        r"""
        Returns the callback configuration as set by
        :func:`Set Acceleration Callback Configuration`.
        """
        self.check_validity()

        return GetAccelerationCallbackConfiguration(*self.ipcon.send_request(self, BrickletAccelerometerV2.FUNCTION_GET_ACCELERATION_CALLBACK_CONFIGURATION, (), '', 13, 'I !'))

    def set_info_led_config(self, config):
        r"""
        Configures the info LED (marked as "Force" on the Bricklet) to be either turned off,
        turned on, or blink in heartbeat mode.
        """
        self.check_validity()

        config = int(config)

        self.ipcon.send_request(self, BrickletAccelerometerV2.FUNCTION_SET_INFO_LED_CONFIG, (config,), 'B', 0, '')

    def get_info_led_config(self):
        r"""
        Returns the LED configuration as set by :func:`Set Info LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAccelerometerV2.FUNCTION_GET_INFO_LED_CONFIG, (), '', 9, 'B')

    def set_continuous_acceleration_configuration(self, enable_x, enable_y, enable_z, resolution):
        r"""
        For high throughput of acceleration data (> 1000Hz) you have to use the
        :cb:`Continuous Acceleration 16 Bit` or :cb:`Continuous Acceleration 8 Bit`
        callbacks.

        You can enable the callback for each axis (x, y, z) individually and choose a
        resolution of 8 bit or 16 bit.

        If at least one of the axis is enabled and the resolution is set to 8 bit,
        the :cb:`Continuous Acceleration 8 Bit` callback is activated. If at least
        one of the axis is enabled and the resolution is set to 16 bit,
        the :cb:`Continuous Acceleration 16 Bit` callback is activated.

        The returned values are raw ADC data. If you want to put this data into
        a FFT to determine the occurrences of specific frequencies we recommend
        that you use the data as is. It has all of the ADC noise in it. This noise
        looks like pure noise at first glance, but it might still have some frequnecy
        information in it that can be utilized by the FFT.

        Otherwise you have to use the following formulas that depend on the configured
        resolution (8/16 bit) and the full scale range (see :func:`Set Configuration`) to calculate
        the data in gₙ/10000 (same unit that is returned by :func:`Get Acceleration`):

        * 16 bit, full scale 2g: acceleration = value * 625 / 1024
        * 16 bit, full scale 4g: acceleration = value * 1250 / 1024
        * 16 bit, full scale 8g: acceleration = value * 2500 / 1024

        If a resolution of 8 bit is used, only the 8 most significant bits will be
        transferred, so you can use the following formulas:

        * 8 bit, full scale 2g: acceleration = value * 256 * 625 / 1024
        * 8 bit, full scale 4g: acceleration = value * 256 * 1250 / 1024
        * 8 bit, full scale 8g: acceleration = value * 256 * 2500 / 1024

        If no axis is enabled, both callbacks are disabled. If one of the continuous
        callbacks is enabled, the :cb:`Acceleration` callback is disabled.

        The maximum throughput depends on the exact configuration:

        .. csv-table::
         :header: "Number of axis enabled", "Throughput 8 bit", "Throughout 16 bit"
         :widths: 20, 20, 20

         "1", "25600Hz", "25600Hz"
         "2", "25600Hz", "15000Hz"
         "3", "20000Hz", "10000Hz"
        """
        self.check_validity()

        enable_x = bool(enable_x)
        enable_y = bool(enable_y)
        enable_z = bool(enable_z)
        resolution = int(resolution)

        self.ipcon.send_request(self, BrickletAccelerometerV2.FUNCTION_SET_CONTINUOUS_ACCELERATION_CONFIGURATION, (enable_x, enable_y, enable_z, resolution), '! ! ! B', 0, '')

    def get_continuous_acceleration_configuration(self):
        r"""
        Returns the continuous acceleration configuration as set by
        :func:`Set Continuous Acceleration Configuration`.
        """
        self.check_validity()

        return GetContinuousAccelerationConfiguration(*self.ipcon.send_request(self, BrickletAccelerometerV2.FUNCTION_GET_CONTINUOUS_ACCELERATION_CONFIGURATION, (), '', 12, '! ! ! B'))

    def set_filter_configuration(self, iir_bypass, low_pass_filter):
        r"""
        Configures IIR Bypass filter mode and low pass filter roll off corner frequency.

        The filter can be applied or bypassed and the corner frequency can be
        half or a ninth of the output data rate.

        .. image:: /Images/Bricklets/bricklet_accelerometer_v2_filter.png
           :scale: 100 %
           :alt: Accelerometer filter
           :align: center
           :target: ../../_images/Bricklets/bricklet_accelerometer_v2_filter.png

        .. versionadded:: 2.0.2$nbsp;(Plugin)
        """
        self.check_validity()

        iir_bypass = int(iir_bypass)
        low_pass_filter = int(low_pass_filter)

        self.ipcon.send_request(self, BrickletAccelerometerV2.FUNCTION_SET_FILTER_CONFIGURATION, (iir_bypass, low_pass_filter), 'B B', 0, '')

    def get_filter_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Filter Configuration`.

        .. versionadded:: 2.0.2$nbsp;(Plugin)
        """
        self.check_validity()

        return GetFilterConfiguration(*self.ipcon.send_request(self, BrickletAccelerometerV2.FUNCTION_GET_FILTER_CONFIGURATION, (), '', 10, 'B B'))

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletAccelerometerV2.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletAccelerometerV2.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAccelerometerV2.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletAccelerometerV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletAccelerometerV2.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletAccelerometerV2.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAccelerometerV2.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAccelerometerV2.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletAccelerometerV2.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletAccelerometerV2.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAccelerometerV2.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletAccelerometerV2.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

AccelerometerV2 = BrickletAccelerometerV2 # for backward compatibility
