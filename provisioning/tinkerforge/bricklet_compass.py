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

GetHeadingCallbackConfiguration = namedtuple('HeadingCallbackConfiguration', ['period', 'value_has_to_change', 'option', 'min', 'max'])
GetMagneticFluxDensity = namedtuple('MagneticFluxDensity', ['x', 'y', 'z'])
GetMagneticFluxDensityCallbackConfiguration = namedtuple('MagneticFluxDensityCallbackConfiguration', ['period', 'value_has_to_change'])
GetConfiguration = namedtuple('Configuration', ['data_rate', 'background_calibration'])
GetCalibration = namedtuple('Calibration', ['offset', 'gain'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletCompass(Device):
    r"""
    3-axis compass with 10 nanotesla and 0.1° resolution
    """

    DEVICE_IDENTIFIER = 2153
    DEVICE_DISPLAY_NAME = 'Compass Bricklet'
    DEVICE_URL_PART = 'compass' # internal

    CALLBACK_HEADING = 4
    CALLBACK_MAGNETIC_FLUX_DENSITY = 8


    FUNCTION_GET_HEADING = 1
    FUNCTION_SET_HEADING_CALLBACK_CONFIGURATION = 2
    FUNCTION_GET_HEADING_CALLBACK_CONFIGURATION = 3
    FUNCTION_GET_MAGNETIC_FLUX_DENSITY = 5
    FUNCTION_SET_MAGNETIC_FLUX_DENSITY_CALLBACK_CONFIGURATION = 6
    FUNCTION_GET_MAGNETIC_FLUX_DENSITY_CALLBACK_CONFIGURATION = 7
    FUNCTION_SET_CONFIGURATION = 9
    FUNCTION_GET_CONFIGURATION = 10
    FUNCTION_SET_CALIBRATION = 11
    FUNCTION_GET_CALIBRATION = 12
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
    DATA_RATE_100HZ = 0
    DATA_RATE_200HZ = 1
    DATA_RATE_400HZ = 2
    DATA_RATE_600HZ = 3
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
        Device.__init__(self, uid, ipcon, BrickletCompass.DEVICE_IDENTIFIER, BrickletCompass.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletCompass.FUNCTION_GET_HEADING] = BrickletCompass.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCompass.FUNCTION_SET_HEADING_CALLBACK_CONFIGURATION] = BrickletCompass.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletCompass.FUNCTION_GET_HEADING_CALLBACK_CONFIGURATION] = BrickletCompass.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCompass.FUNCTION_GET_MAGNETIC_FLUX_DENSITY] = BrickletCompass.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCompass.FUNCTION_SET_MAGNETIC_FLUX_DENSITY_CALLBACK_CONFIGURATION] = BrickletCompass.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletCompass.FUNCTION_GET_MAGNETIC_FLUX_DENSITY_CALLBACK_CONFIGURATION] = BrickletCompass.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCompass.FUNCTION_SET_CONFIGURATION] = BrickletCompass.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletCompass.FUNCTION_GET_CONFIGURATION] = BrickletCompass.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCompass.FUNCTION_SET_CALIBRATION] = BrickletCompass.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletCompass.FUNCTION_GET_CALIBRATION] = BrickletCompass.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCompass.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletCompass.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCompass.FUNCTION_SET_BOOTLOADER_MODE] = BrickletCompass.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCompass.FUNCTION_GET_BOOTLOADER_MODE] = BrickletCompass.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCompass.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletCompass.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletCompass.FUNCTION_WRITE_FIRMWARE] = BrickletCompass.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCompass.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletCompass.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletCompass.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletCompass.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCompass.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletCompass.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCompass.FUNCTION_RESET] = BrickletCompass.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletCompass.FUNCTION_WRITE_UID] = BrickletCompass.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletCompass.FUNCTION_READ_UID] = BrickletCompass.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletCompass.FUNCTION_GET_IDENTITY] = BrickletCompass.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletCompass.CALLBACK_HEADING] = (10, 'h')
        self.callback_formats[BrickletCompass.CALLBACK_MAGNETIC_FLUX_DENSITY] = (20, 'i i i')

        ipcon.add_device(self)

    def get_heading(self):
        r"""
        Returns the heading (north = 0 degree, east = 90 degree).

        Alternatively you can use :func:`Get Magnetic Flux Density` and calculate the
        heading with ``heading = atan2(y, x) * 180 / PI``.


        If you want to get the value periodically, it is recommended to use the
        :cb:`Heading` callback. You can set the callback configuration
        with :func:`Set Heading Callback Configuration`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletCompass.FUNCTION_GET_HEADING, (), '', 10, 'h')

    def set_heading_callback_configuration(self, period, value_has_to_change, option, min, max):
        r"""
        The period is the period with which the :cb:`Heading` callback is triggered
        periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change
        within the period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.

        It is furthermore possible to constrain the callback with thresholds.

        The `option`-parameter together with min/max sets a threshold for the :cb:`Heading` callback.

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

        self.ipcon.send_request(self, BrickletCompass.FUNCTION_SET_HEADING_CALLBACK_CONFIGURATION, (period, value_has_to_change, option, min, max), 'I ! c h h', 0, '')

    def get_heading_callback_configuration(self):
        r"""
        Returns the callback configuration as set by :func:`Set Heading Callback Configuration`.
        """
        self.check_validity()

        return GetHeadingCallbackConfiguration(*self.ipcon.send_request(self, BrickletCompass.FUNCTION_GET_HEADING_CALLBACK_CONFIGURATION, (), '', 18, 'I ! c h h'))

    def get_magnetic_flux_density(self):
        r"""
        Returns the `magnetic flux density (magnetic induction) <https://en.wikipedia.org/wiki/Magnetic_flux>`__
        for all three axis.

        If you want to get the value periodically, it is recommended to use the
        :cb:`Magnetic Flux Density` callback. You can set the callback configuration
        with :func:`Set Magnetic Flux Density Callback Configuration`.
        """
        self.check_validity()

        return GetMagneticFluxDensity(*self.ipcon.send_request(self, BrickletCompass.FUNCTION_GET_MAGNETIC_FLUX_DENSITY, (), '', 20, 'i i i'))

    def set_magnetic_flux_density_callback_configuration(self, period, value_has_to_change):
        r"""
        The period is the period with which the :cb:`Magnetic Flux Density` callback
        is triggered periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change within the
        period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.
        """
        self.check_validity()

        period = int(period)
        value_has_to_change = bool(value_has_to_change)

        self.ipcon.send_request(self, BrickletCompass.FUNCTION_SET_MAGNETIC_FLUX_DENSITY_CALLBACK_CONFIGURATION, (period, value_has_to_change), 'I !', 0, '')

    def get_magnetic_flux_density_callback_configuration(self):
        r"""
        Returns the callback configuration as set by
        :func:`Set Magnetic Flux Density Callback Configuration`.
        """
        self.check_validity()

        return GetMagneticFluxDensityCallbackConfiguration(*self.ipcon.send_request(self, BrickletCompass.FUNCTION_GET_MAGNETIC_FLUX_DENSITY_CALLBACK_CONFIGURATION, (), '', 13, 'I !'))

    def set_configuration(self, data_rate, background_calibration):
        r"""
        Configures the data rate and background calibration.

        * Data Rate: Sets the data rate that is used by the magnetometer.
          The lower the data rate, the lower is the noise on the data.
        * Background Calibration: Set to *true* to enable the background
          calibration and *false* to turn it off. If the background calibration
          is enabled the sensing polarity is flipped once per second to automatically
          calculate and remove offset that is caused by temperature changes.
          This polarity flipping takes about 20ms. This means that once a second
          you will not get new data for a period of 20ms. We highly recommend that
          you keep the background calibration enabled and only disable it if the 20ms
          off-time is a problem in your application.
        """
        self.check_validity()

        data_rate = int(data_rate)
        background_calibration = bool(background_calibration)

        self.ipcon.send_request(self, BrickletCompass.FUNCTION_SET_CONFIGURATION, (data_rate, background_calibration), 'B !', 0, '')

    def get_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Configuration`.
        """
        self.check_validity()

        return GetConfiguration(*self.ipcon.send_request(self, BrickletCompass.FUNCTION_GET_CONFIGURATION, (), '', 10, 'B !'))

    def set_calibration(self, offset, gain):
        r"""
        Sets offset and gain for each of the three axes.

        The Bricklet is factory calibrated. If you want to re-calibrate the
        Bricklet we recommend that you do the calibration through Brick Viewer.

        The calibration is saved in non-volatile memory and only has to be
        done once.
        """
        self.check_validity()

        offset = list(map(int, offset))
        gain = list(map(int, gain))

        self.ipcon.send_request(self, BrickletCompass.FUNCTION_SET_CALIBRATION, (offset, gain), '3h 3h', 0, '')

    def get_calibration(self):
        r"""
        Returns the calibration parameters as set by :func:`Set Calibration`.
        """
        self.check_validity()

        return GetCalibration(*self.ipcon.send_request(self, BrickletCompass.FUNCTION_GET_CALIBRATION, (), '', 20, '3h 3h'))

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletCompass.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletCompass.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletCompass.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletCompass.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletCompass.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletCompass.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletCompass.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletCompass.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletCompass.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletCompass.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletCompass.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletCompass.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

Compass = BrickletCompass # for backward compatibility
