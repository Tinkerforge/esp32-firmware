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

from collections import namedtuple

try:
    from .ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data
except (ValueError, ImportError):
    try:
        from ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data
    except (ValueError, ImportError):
        from tinkerforge.ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data

GetCoordinates = namedtuple('Coordinates', ['latitude', 'ns', 'longitude', 'ew'])
GetStatus = namedtuple('Status', ['has_fix', 'satellites_view'])
GetAltitude = namedtuple('Altitude', ['altitude', 'geoidal_separation'])
GetMotion = namedtuple('Motion', ['course', 'speed'])
GetDateTime = namedtuple('DateTime', ['date', 'time'])
GetSatelliteSystemStatusLowLevel = namedtuple('SatelliteSystemStatusLowLevel', ['satellite_numbers_length', 'satellite_numbers_data', 'fix', 'pdop', 'hdop', 'vdop'])
GetSatelliteStatus = namedtuple('SatelliteStatus', ['elevation', 'azimuth', 'snr'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])
GetSatelliteSystemStatus = namedtuple('SatelliteSystemStatus', ['satellite_numbers', 'fix', 'pdop', 'hdop', 'vdop'])

class BrickletGPSV3(Device):
    r"""
    Determine position, velocity and altitude using GPS
    """

    DEVICE_IDENTIFIER = 2171
    DEVICE_DISPLAY_NAME = 'GPS Bricklet 3.0'
    DEVICE_URL_PART = 'gps_v3' # internal

    CALLBACK_PULSE_PER_SECOND = 21
    CALLBACK_COORDINATES = 22
    CALLBACK_STATUS = 23
    CALLBACK_ALTITUDE = 24
    CALLBACK_MOTION = 25
    CALLBACK_DATE_TIME = 26


    FUNCTION_GET_COORDINATES = 1
    FUNCTION_GET_STATUS = 2
    FUNCTION_GET_ALTITUDE = 3
    FUNCTION_GET_MOTION = 4
    FUNCTION_GET_DATE_TIME = 5
    FUNCTION_RESTART = 6
    FUNCTION_GET_SATELLITE_SYSTEM_STATUS_LOW_LEVEL = 7
    FUNCTION_GET_SATELLITE_STATUS = 8
    FUNCTION_SET_FIX_LED_CONFIG = 9
    FUNCTION_GET_FIX_LED_CONFIG = 10
    FUNCTION_SET_COORDINATES_CALLBACK_PERIOD = 11
    FUNCTION_GET_COORDINATES_CALLBACK_PERIOD = 12
    FUNCTION_SET_STATUS_CALLBACK_PERIOD = 13
    FUNCTION_GET_STATUS_CALLBACK_PERIOD = 14
    FUNCTION_SET_ALTITUDE_CALLBACK_PERIOD = 15
    FUNCTION_GET_ALTITUDE_CALLBACK_PERIOD = 16
    FUNCTION_SET_MOTION_CALLBACK_PERIOD = 17
    FUNCTION_GET_MOTION_CALLBACK_PERIOD = 18
    FUNCTION_SET_DATE_TIME_CALLBACK_PERIOD = 19
    FUNCTION_GET_DATE_TIME_CALLBACK_PERIOD = 20
    FUNCTION_SET_SBAS_CONFIG = 27
    FUNCTION_GET_SBAS_CONFIG = 28
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

    RESTART_TYPE_HOT_START = 0
    RESTART_TYPE_WARM_START = 1
    RESTART_TYPE_COLD_START = 2
    RESTART_TYPE_FACTORY_RESET = 3
    SATELLITE_SYSTEM_GPS = 0
    SATELLITE_SYSTEM_GLONASS = 1
    SATELLITE_SYSTEM_GALILEO = 2
    FIX_NO_FIX = 1
    FIX_2D_FIX = 2
    FIX_3D_FIX = 3
    FIX_LED_CONFIG_OFF = 0
    FIX_LED_CONFIG_ON = 1
    FIX_LED_CONFIG_SHOW_HEARTBEAT = 2
    FIX_LED_CONFIG_SHOW_FIX = 3
    FIX_LED_CONFIG_SHOW_PPS = 4
    SBAS_ENABLED = 0
    SBAS_DISABLED = 1
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
        Device.__init__(self, uid, ipcon, BrickletGPSV3.DEVICE_IDENTIFIER, BrickletGPSV3.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletGPSV3.FUNCTION_GET_COORDINATES] = BrickletGPSV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_GET_STATUS] = BrickletGPSV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_GET_ALTITUDE] = BrickletGPSV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_GET_MOTION] = BrickletGPSV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_GET_DATE_TIME] = BrickletGPSV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_RESTART] = BrickletGPSV3.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletGPSV3.FUNCTION_GET_SATELLITE_SYSTEM_STATUS_LOW_LEVEL] = BrickletGPSV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_GET_SATELLITE_STATUS] = BrickletGPSV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_SET_FIX_LED_CONFIG] = BrickletGPSV3.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletGPSV3.FUNCTION_GET_FIX_LED_CONFIG] = BrickletGPSV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_SET_COORDINATES_CALLBACK_PERIOD] = BrickletGPSV3.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_GET_COORDINATES_CALLBACK_PERIOD] = BrickletGPSV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_SET_STATUS_CALLBACK_PERIOD] = BrickletGPSV3.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_GET_STATUS_CALLBACK_PERIOD] = BrickletGPSV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_SET_ALTITUDE_CALLBACK_PERIOD] = BrickletGPSV3.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_GET_ALTITUDE_CALLBACK_PERIOD] = BrickletGPSV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_SET_MOTION_CALLBACK_PERIOD] = BrickletGPSV3.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_GET_MOTION_CALLBACK_PERIOD] = BrickletGPSV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_SET_DATE_TIME_CALLBACK_PERIOD] = BrickletGPSV3.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_GET_DATE_TIME_CALLBACK_PERIOD] = BrickletGPSV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_SET_SBAS_CONFIG] = BrickletGPSV3.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletGPSV3.FUNCTION_GET_SBAS_CONFIG] = BrickletGPSV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletGPSV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_SET_BOOTLOADER_MODE] = BrickletGPSV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_GET_BOOTLOADER_MODE] = BrickletGPSV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletGPSV3.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletGPSV3.FUNCTION_WRITE_FIRMWARE] = BrickletGPSV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletGPSV3.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletGPSV3.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletGPSV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletGPSV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_RESET] = BrickletGPSV3.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletGPSV3.FUNCTION_WRITE_UID] = BrickletGPSV3.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletGPSV3.FUNCTION_READ_UID] = BrickletGPSV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletGPSV3.FUNCTION_GET_IDENTITY] = BrickletGPSV3.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletGPSV3.CALLBACK_PULSE_PER_SECOND] = (8, '')
        self.callback_formats[BrickletGPSV3.CALLBACK_COORDINATES] = (18, 'I c I c')
        self.callback_formats[BrickletGPSV3.CALLBACK_STATUS] = (10, '! B')
        self.callback_formats[BrickletGPSV3.CALLBACK_ALTITUDE] = (16, 'i i')
        self.callback_formats[BrickletGPSV3.CALLBACK_MOTION] = (16, 'I I')
        self.callback_formats[BrickletGPSV3.CALLBACK_DATE_TIME] = (16, 'I I')

        ipcon.add_device(self)

    def get_coordinates(self):
        r"""
        Returns the GPS coordinates. Latitude and longitude are given in the
        ``DD.dddddd°`` format, the value 57123468 means 57.123468°.
        The parameter ``ns`` and ``ew`` are the cardinal directions for
        latitude and longitude. Possible values for ``ns`` and ``ew`` are 'N', 'S', 'E'
        and 'W' (north, south, east and west).

        This data is only valid if there is currently a fix as indicated by
        :func:`Get Status`.
        """
        self.check_validity()

        return GetCoordinates(*self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_GET_COORDINATES, (), '', 18, 'I c I c'))

    def get_status(self):
        r"""
        Returns if a fix is currently available as well as the number of
        satellites that are in view.

        There is also a :ref:`green LED <gps_v2_bricklet_fix_led>` on the Bricklet that
        indicates the fix status.
        """
        self.check_validity()

        return GetStatus(*self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_GET_STATUS, (), '', 10, '! B'))

    def get_altitude(self):
        r"""
        Returns the current altitude and corresponding geoidal separation.

        This data is only valid if there is currently a fix as indicated by
        :func:`Get Status`.
        """
        self.check_validity()

        return GetAltitude(*self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_GET_ALTITUDE, (), '', 16, 'i i'))

    def get_motion(self):
        r"""
        Returns the current course and speed. A course of 0° means the Bricklet is
        traveling north bound and 90° means it is traveling east bound.

        Please note that this only returns useful values if an actual movement
        is present.

        This data is only valid if there is currently a fix as indicated by
        :func:`Get Status`.
        """
        self.check_validity()

        return GetMotion(*self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_GET_MOTION, (), '', 16, 'I I'))

    def get_date_time(self):
        r"""
        Returns the current date and time. The date is
        given in the format ``ddmmyy`` and the time is given
        in the format ``hhmmss.sss``. For example, 140713 means
        14.07.13 as date and 195923568 means 19:59:23.568 as time.
        """
        self.check_validity()

        return GetDateTime(*self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_GET_DATE_TIME, (), '', 16, 'I I'))

    def restart(self, restart_type):
        r"""
        Restarts the GPS Bricklet, the following restart types are available:

        .. csv-table::
         :header: "Value", "Description"
         :widths: 10, 100

         "0", "Hot start (use all available data in the NV store)"
         "1", "Warm start (don't use ephemeris at restart)"
         "2", "Cold start (don't use time, position, almanacs and ephemeris at restart)"
         "3", "Factory reset (clear all system/user configurations at restart)"
        """
        self.check_validity()

        restart_type = int(restart_type)

        self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_RESTART, (restart_type,), 'B', 0, '')

    def get_satellite_system_status_low_level(self, satellite_system):
        r"""
        Returns the

        * satellite numbers list (up to 12 items)
        * fix value,
        * PDOP value,
        * HDOP value and
        * VDOP value

        for a given satellite system. Currently GPS, GLONASS and Galileo are supported.

        The GPS and GLONASS satellites have unique numbers and the satellite list gives
        the numbers of the satellites that are currently utilized. The number 0 is not
        a valid satellite number and can be ignored in the list.
        """
        self.check_validity()

        satellite_system = int(satellite_system)

        return GetSatelliteSystemStatusLowLevel(*self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_GET_SATELLITE_SYSTEM_STATUS_LOW_LEVEL, (satellite_system,), 'B', 28, 'B 12B B H H H'))

    def get_satellite_status(self, satellite_system, satellite_number):
        r"""
        Returns the current elevation, azimuth and SNR for a given satellite and satellite system.

        The available satellite numbers are:

        * GPS: 1-32
        * GLONASS: 65-96
        * Galileo: 301-332
        """
        self.check_validity()

        satellite_system = int(satellite_system)
        satellite_number = int(satellite_number)

        return GetSatelliteStatus(*self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_GET_SATELLITE_STATUS, (satellite_system, satellite_number), 'B B', 14, 'h h h'))

    def set_fix_led_config(self, config):
        r"""
        Sets the fix LED configuration. By default the LED shows if
        the Bricklet got a GPS fix yet. If a fix is established the LED turns on.
        If there is no fix then the LED is turned off.

        You can also turn the LED permanently on/off, show a heartbeat or let it blink
        in sync with the PPS (pulse per second) output of the GPS module.

        If the Bricklet is in bootloader mode, the LED is off.
        """
        self.check_validity()

        config = int(config)

        self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_SET_FIX_LED_CONFIG, (config,), 'B', 0, '')

    def get_fix_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Fix LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_GET_FIX_LED_CONFIG, (), '', 9, 'B')

    def set_coordinates_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Coordinates` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Coordinates` callback is only triggered if the coordinates changed
        since the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_SET_COORDINATES_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_coordinates_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Coordinates Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_GET_COORDINATES_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_status_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Status` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Status` callback is only triggered if the status changed since the
        last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_SET_STATUS_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_status_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Status Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_GET_STATUS_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_altitude_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Altitude` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Altitude` callback is only triggered if the altitude changed since the
        last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_SET_ALTITUDE_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_altitude_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Altitude Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_GET_ALTITUDE_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_motion_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Motion` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Motion` callback is only triggered if the motion changed since the
        last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_SET_MOTION_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_motion_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Motion Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_GET_MOTION_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_date_time_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Date Time` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Date Time` callback is only triggered if the date or time changed
        since the last triggering.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_SET_DATE_TIME_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_date_time_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Date Time Callback Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_GET_DATE_TIME_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_sbas_config(self, sbas_config):
        r"""
        If `SBAS <https://en.wikipedia.org/wiki/GNSS_augmentation#Satellite-based_augmentation_system>`__ is enabled,
        the position accuracy increases (if SBAS satellites are in view),
        but the update rate is limited to 5Hz. With SBAS disabled the update rate is increased to 10Hz.
        """
        self.check_validity()

        sbas_config = int(sbas_config)

        self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_SET_SBAS_CONFIG, (sbas_config,), 'B', 0, '')

    def get_sbas_config(self):
        r"""
        Returns the SBAS configuration as set by :func:`Set SBAS Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_GET_SBAS_CONFIG, (), '', 9, 'B')

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletGPSV3.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def get_satellite_system_status(self, satellite_system):
        r"""
        Returns the

        * satellite numbers list (up to 12 items)
        * fix value,
        * PDOP value,
        * HDOP value and
        * VDOP value

        for a given satellite system. Currently GPS, GLONASS and Galileo are supported.

        The GPS and GLONASS satellites have unique numbers and the satellite list gives
        the numbers of the satellites that are currently utilized. The number 0 is not
        a valid satellite number and can be ignored in the list.
        """
        satellite_system = int(satellite_system)

        ret = self.get_satellite_system_status_low_level(satellite_system)

        return GetSatelliteSystemStatus(ret.satellite_numbers_data[:ret.satellite_numbers_length], ret.fix, ret.pdop, ret.hdop, ret.vdop)

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

GPSV3 = BrickletGPSV3 # for backward compatibility
