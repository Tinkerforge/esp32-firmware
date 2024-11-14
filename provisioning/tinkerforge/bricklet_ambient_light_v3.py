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

GetIlluminanceCallbackConfiguration = namedtuple('IlluminanceCallbackConfiguration', ['period', 'value_has_to_change', 'option', 'min', 'max'])
GetConfiguration = namedtuple('Configuration', ['illuminance_range', 'integration_time'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletAmbientLightV3(Device):
    r"""
    Measures ambient light up to 64000lux
    """

    DEVICE_IDENTIFIER = 2131
    DEVICE_DISPLAY_NAME = 'Ambient Light Bricklet 3.0'
    DEVICE_URL_PART = 'ambient_light_v3' # internal

    CALLBACK_ILLUMINANCE = 4


    FUNCTION_GET_ILLUMINANCE = 1
    FUNCTION_SET_ILLUMINANCE_CALLBACK_CONFIGURATION = 2
    FUNCTION_GET_ILLUMINANCE_CALLBACK_CONFIGURATION = 3
    FUNCTION_SET_CONFIGURATION = 5
    FUNCTION_GET_CONFIGURATION = 6
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
    ILLUMINANCE_RANGE_UNLIMITED = 6
    ILLUMINANCE_RANGE_64000LUX = 0
    ILLUMINANCE_RANGE_32000LUX = 1
    ILLUMINANCE_RANGE_16000LUX = 2
    ILLUMINANCE_RANGE_8000LUX = 3
    ILLUMINANCE_RANGE_1300LUX = 4
    ILLUMINANCE_RANGE_600LUX = 5
    INTEGRATION_TIME_50MS = 0
    INTEGRATION_TIME_100MS = 1
    INTEGRATION_TIME_150MS = 2
    INTEGRATION_TIME_200MS = 3
    INTEGRATION_TIME_250MS = 4
    INTEGRATION_TIME_300MS = 5
    INTEGRATION_TIME_350MS = 6
    INTEGRATION_TIME_400MS = 7
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
        Device.__init__(self, uid, ipcon, BrickletAmbientLightV3.DEVICE_IDENTIFIER, BrickletAmbientLightV3.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletAmbientLightV3.FUNCTION_GET_ILLUMINANCE] = BrickletAmbientLightV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAmbientLightV3.FUNCTION_SET_ILLUMINANCE_CALLBACK_CONFIGURATION] = BrickletAmbientLightV3.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletAmbientLightV3.FUNCTION_GET_ILLUMINANCE_CALLBACK_CONFIGURATION] = BrickletAmbientLightV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAmbientLightV3.FUNCTION_SET_CONFIGURATION] = BrickletAmbientLightV3.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAmbientLightV3.FUNCTION_GET_CONFIGURATION] = BrickletAmbientLightV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAmbientLightV3.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletAmbientLightV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAmbientLightV3.FUNCTION_SET_BOOTLOADER_MODE] = BrickletAmbientLightV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAmbientLightV3.FUNCTION_GET_BOOTLOADER_MODE] = BrickletAmbientLightV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAmbientLightV3.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletAmbientLightV3.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAmbientLightV3.FUNCTION_WRITE_FIRMWARE] = BrickletAmbientLightV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAmbientLightV3.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletAmbientLightV3.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAmbientLightV3.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletAmbientLightV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAmbientLightV3.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletAmbientLightV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAmbientLightV3.FUNCTION_RESET] = BrickletAmbientLightV3.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAmbientLightV3.FUNCTION_WRITE_UID] = BrickletAmbientLightV3.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletAmbientLightV3.FUNCTION_READ_UID] = BrickletAmbientLightV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletAmbientLightV3.FUNCTION_GET_IDENTITY] = BrickletAmbientLightV3.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletAmbientLightV3.CALLBACK_ILLUMINANCE] = (12, 'I')

        ipcon.add_device(self)

    def get_illuminance(self):
        r"""
        Returns the illuminance of the ambient light sensor. The measurement range goes
        up to about 100000lux, but above 64000lux the precision starts to drop.
        The illuminance is given in lux/100, i.e. a value of 450000 means that an
        illuminance of 4500lux is measured.

        An illuminance of 0lux indicates an error condition where the sensor cannot
        perform a reasonable measurement. This can happen with very dim or very bright
        light conditions. In bright light conditions this might indicate that the sensor
        is saturated and the configuration should be modified (:func:`Set Configuration`)
        to better match the conditions.


        If you want to get the value periodically, it is recommended to use the
        :cb:`Illuminance` callback. You can set the callback configuration
        with :func:`Set Illuminance Callback Configuration`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAmbientLightV3.FUNCTION_GET_ILLUMINANCE, (), '', 12, 'I')

    def set_illuminance_callback_configuration(self, period, value_has_to_change, option, min, max):
        r"""
        The period is the period with which the :cb:`Illuminance` callback is triggered
        periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change
        within the period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.

        It is furthermore possible to constrain the callback with thresholds.

        The `option`-parameter together with min/max sets a threshold for the :cb:`Illuminance` callback.

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

        self.ipcon.send_request(self, BrickletAmbientLightV3.FUNCTION_SET_ILLUMINANCE_CALLBACK_CONFIGURATION, (period, value_has_to_change, option, min, max), 'I ! c I I', 0, '')

    def get_illuminance_callback_configuration(self):
        r"""
        Returns the callback configuration as set by :func:`Set Illuminance Callback Configuration`.
        """
        self.check_validity()

        return GetIlluminanceCallbackConfiguration(*self.ipcon.send_request(self, BrickletAmbientLightV3.FUNCTION_GET_ILLUMINANCE_CALLBACK_CONFIGURATION, (), '', 22, 'I ! c I I'))

    def set_configuration(self, illuminance_range, integration_time):
        r"""
        Sets the configuration. It is possible to configure an illuminance range
        between 0-600lux and 0-64000lux and an integration time between 50ms and 400ms.

        The unlimited illuminance range allows to measure up to about 100000lux, but
        above 64000lux the precision starts to drop.

        A smaller illuminance range increases the resolution of the data. A longer
        integration time will result in less noise on the data.

        If the actual measure illuminance is out-of-range then the current illuminance
        range maximum +0.01lux is reported by :func:`Get Illuminance` and the
        :cb:`Illuminance` callback. For example, 800001 for the 0-8000lux range.

        With a long integration time the sensor might be saturated before the measured
        value reaches the maximum of the selected illuminance range. In this case 0lux
        is reported by :func:`Get Illuminance` and the :cb:`Illuminance` callback.

        If the measurement is out-of-range or the sensor is saturated then you should
        configure the next higher illuminance range. If the highest range is already
        in use, then start to reduce the integration time.
        """
        self.check_validity()

        illuminance_range = int(illuminance_range)
        integration_time = int(integration_time)

        self.ipcon.send_request(self, BrickletAmbientLightV3.FUNCTION_SET_CONFIGURATION, (illuminance_range, integration_time), 'B B', 0, '')

    def get_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Configuration`.
        """
        self.check_validity()

        return GetConfiguration(*self.ipcon.send_request(self, BrickletAmbientLightV3.FUNCTION_GET_CONFIGURATION, (), '', 10, 'B B'))

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletAmbientLightV3.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletAmbientLightV3.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAmbientLightV3.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletAmbientLightV3.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletAmbientLightV3.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletAmbientLightV3.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAmbientLightV3.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAmbientLightV3.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletAmbientLightV3.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletAmbientLightV3.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletAmbientLightV3.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletAmbientLightV3.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

AmbientLightV3 = BrickletAmbientLightV3 # for backward compatibility
