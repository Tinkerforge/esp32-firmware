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

GetHighContrastImageLowLevel = namedtuple('HighContrastImageLowLevel', ['image_chunk_offset', 'image_chunk_data'])
GetTemperatureImageLowLevel = namedtuple('TemperatureImageLowLevel', ['image_chunk_offset', 'image_chunk_data'])
GetStatistics = namedtuple('Statistics', ['spotmeter_statistics', 'temperatures', 'resolution', 'ffc_status', 'temperature_warning'])
GetHighContrastConfig = namedtuple('HighContrastConfig', ['region_of_interest', 'dampening_factor', 'clip_limit', 'empty_counts'])
GetFluxLinearParameters = namedtuple('FluxLinearParameters', ['scene_emissivity', 'temperature_background', 'tau_window', 'temperatur_window', 'tau_atmosphere', 'temperature_atmosphere', 'reflection_window', 'temperature_reflection'])
GetFFCShutterMode = namedtuple('FFCShutterMode', ['shutter_mode', 'temp_lockout_state', 'video_freeze_during_ffc', 'ffc_desired', 'elapsed_time_since_last_ffc', 'desired_ffc_period', 'explicit_cmd_to_open', 'desired_ffc_temp_delta', 'imminent_delay'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletThermalImaging(Device):
    r"""
    80x60 pixel thermal imaging camera
    """

    DEVICE_IDENTIFIER = 278
    DEVICE_DISPLAY_NAME = 'Thermal Imaging Bricklet'
    DEVICE_URL_PART = 'thermal_imaging' # internal

    CALLBACK_HIGH_CONTRAST_IMAGE_LOW_LEVEL = 12
    CALLBACK_TEMPERATURE_IMAGE_LOW_LEVEL = 13

    CALLBACK_HIGH_CONTRAST_IMAGE = -12
    CALLBACK_TEMPERATURE_IMAGE = -13

    FUNCTION_GET_HIGH_CONTRAST_IMAGE_LOW_LEVEL = 1
    FUNCTION_GET_TEMPERATURE_IMAGE_LOW_LEVEL = 2
    FUNCTION_GET_STATISTICS = 3
    FUNCTION_SET_RESOLUTION = 4
    FUNCTION_GET_RESOLUTION = 5
    FUNCTION_SET_SPOTMETER_CONFIG = 6
    FUNCTION_GET_SPOTMETER_CONFIG = 7
    FUNCTION_SET_HIGH_CONTRAST_CONFIG = 8
    FUNCTION_GET_HIGH_CONTRAST_CONFIG = 9
    FUNCTION_SET_IMAGE_TRANSFER_CONFIG = 10
    FUNCTION_GET_IMAGE_TRANSFER_CONFIG = 11
    FUNCTION_SET_FLUX_LINEAR_PARAMETERS = 14
    FUNCTION_GET_FLUX_LINEAR_PARAMETERS = 15
    FUNCTION_SET_FFC_SHUTTER_MODE = 16
    FUNCTION_GET_FFC_SHUTTER_MODE = 17
    FUNCTION_RUN_FFC_NORMALIZATION = 18
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

    RESOLUTION_0_TO_6553_KELVIN = 0
    RESOLUTION_0_TO_655_KELVIN = 1
    FFC_STATUS_NEVER_COMMANDED = 0
    FFC_STATUS_IMMINENT = 1
    FFC_STATUS_IN_PROGRESS = 2
    FFC_STATUS_COMPLETE = 3
    IMAGE_TRANSFER_MANUAL_HIGH_CONTRAST_IMAGE = 0
    IMAGE_TRANSFER_MANUAL_TEMPERATURE_IMAGE = 1
    IMAGE_TRANSFER_CALLBACK_HIGH_CONTRAST_IMAGE = 2
    IMAGE_TRANSFER_CALLBACK_TEMPERATURE_IMAGE = 3
    SHUTTER_MODE_MANUAL = 0
    SHUTTER_MODE_AUTO = 1
    SHUTTER_MODE_EXTERNAL = 2
    SHUTTER_LOCKOUT_INACTIVE = 0
    SHUTTER_LOCKOUT_HIGH = 1
    SHUTTER_LOCKOUT_LOW = 2
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
        Device.__init__(self, uid, ipcon, BrickletThermalImaging.DEVICE_IDENTIFIER, BrickletThermalImaging.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 2)

        self.response_expected[BrickletThermalImaging.FUNCTION_GET_HIGH_CONTRAST_IMAGE_LOW_LEVEL] = BrickletThermalImaging.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletThermalImaging.FUNCTION_GET_TEMPERATURE_IMAGE_LOW_LEVEL] = BrickletThermalImaging.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletThermalImaging.FUNCTION_GET_STATISTICS] = BrickletThermalImaging.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletThermalImaging.FUNCTION_SET_RESOLUTION] = BrickletThermalImaging.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletThermalImaging.FUNCTION_GET_RESOLUTION] = BrickletThermalImaging.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletThermalImaging.FUNCTION_SET_SPOTMETER_CONFIG] = BrickletThermalImaging.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletThermalImaging.FUNCTION_GET_SPOTMETER_CONFIG] = BrickletThermalImaging.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletThermalImaging.FUNCTION_SET_HIGH_CONTRAST_CONFIG] = BrickletThermalImaging.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletThermalImaging.FUNCTION_GET_HIGH_CONTRAST_CONFIG] = BrickletThermalImaging.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletThermalImaging.FUNCTION_SET_IMAGE_TRANSFER_CONFIG] = BrickletThermalImaging.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletThermalImaging.FUNCTION_GET_IMAGE_TRANSFER_CONFIG] = BrickletThermalImaging.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletThermalImaging.FUNCTION_SET_FLUX_LINEAR_PARAMETERS] = BrickletThermalImaging.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletThermalImaging.FUNCTION_GET_FLUX_LINEAR_PARAMETERS] = BrickletThermalImaging.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletThermalImaging.FUNCTION_SET_FFC_SHUTTER_MODE] = BrickletThermalImaging.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletThermalImaging.FUNCTION_GET_FFC_SHUTTER_MODE] = BrickletThermalImaging.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletThermalImaging.FUNCTION_RUN_FFC_NORMALIZATION] = BrickletThermalImaging.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletThermalImaging.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletThermalImaging.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletThermalImaging.FUNCTION_SET_BOOTLOADER_MODE] = BrickletThermalImaging.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletThermalImaging.FUNCTION_GET_BOOTLOADER_MODE] = BrickletThermalImaging.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletThermalImaging.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletThermalImaging.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletThermalImaging.FUNCTION_WRITE_FIRMWARE] = BrickletThermalImaging.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletThermalImaging.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletThermalImaging.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletThermalImaging.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletThermalImaging.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletThermalImaging.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletThermalImaging.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletThermalImaging.FUNCTION_RESET] = BrickletThermalImaging.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletThermalImaging.FUNCTION_WRITE_UID] = BrickletThermalImaging.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletThermalImaging.FUNCTION_READ_UID] = BrickletThermalImaging.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletThermalImaging.FUNCTION_GET_IDENTITY] = BrickletThermalImaging.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletThermalImaging.CALLBACK_HIGH_CONTRAST_IMAGE_LOW_LEVEL] = (72, 'H 62B')
        self.callback_formats[BrickletThermalImaging.CALLBACK_TEMPERATURE_IMAGE_LOW_LEVEL] = (72, 'H 31H')

        self.high_level_callbacks[BrickletThermalImaging.CALLBACK_HIGH_CONTRAST_IMAGE] = [('stream_chunk_offset', 'stream_chunk_data'), {'fixed_length': 4800, 'single_chunk': False}, None]
        self.high_level_callbacks[BrickletThermalImaging.CALLBACK_TEMPERATURE_IMAGE] = [('stream_chunk_offset', 'stream_chunk_data'), {'fixed_length': 4800, 'single_chunk': False}, None]
        ipcon.add_device(self)

    def get_high_contrast_image_low_level(self):
        r"""
        Returns the current high contrast image. See `here <https://www.tinkerforge.com/en/doc/Hardware/Bricklets/Thermal_Imaging.html#high-contrast-image-vs-temperature-image>`__
        for the difference between
        High Contrast and Temperature Image. If you don't know what to use
        the High Contrast Image is probably right for you.

        The data is organized as a 8-bit value 80x60 pixel matrix linearized in
        a one-dimensional array. The data is arranged line by line from top left to
        bottom right.

        Each 8-bit value represents one gray-scale image pixel that can directly be
        shown to a user on a display.

        Before you can use this function you have to enable it with
        :func:`Set Image Transfer Config`.
        """
        self.check_validity()

        return GetHighContrastImageLowLevel(*self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_GET_HIGH_CONTRAST_IMAGE_LOW_LEVEL, (), '', 72, 'H 62B'))

    def get_temperature_image_low_level(self):
        r"""
        Returns the current temperature image. See `here <https://www.tinkerforge.com/en/doc/Hardware/Bricklets/Thermal_Imaging.html#high-contrast-image-vs-temperature-image>`__
        for the difference between High Contrast and Temperature Image.
        If you don't know what to use the High Contrast Image is probably right for you.

        The data is organized as a 16-bit value 80x60 pixel matrix linearized in
        a one-dimensional array. The data is arranged line by line from top left to
        bottom right.

        Each 16-bit value represents one temperature measurement in either
        Kelvin/10 or Kelvin/100 (depending on the resolution set with :func:`Set Resolution`).

        Before you can use this function you have to enable it with
        :func:`Set Image Transfer Config`.
        """
        self.check_validity()

        return GetTemperatureImageLowLevel(*self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_GET_TEMPERATURE_IMAGE_LOW_LEVEL, (), '', 72, 'H 31H'))

    def get_statistics(self):
        r"""
        Returns the spotmeter statistics, various temperatures, current resolution and status bits.

        The spotmeter statistics are:

        * Index 0: Mean Temperature.
        * Index 1: Maximum Temperature.
        * Index 2: Minimum Temperature.
        * Index 3: Pixel Count of spotmeter region of interest.

        The temperatures are:

        * Index 0: Focal Plain Array temperature.
        * Index 1: Focal Plain Array temperature at last FFC (Flat Field Correction).
        * Index 2: Housing temperature.
        * Index 3: Housing temperature at last FFC.

        The resolution is either `0 to 6553 Kelvin` or `0 to 655 Kelvin`. If the resolution is the former,
        the temperatures are in Kelvin/10, if it is the latter the temperatures are in Kelvin/100.

        FFC (Flat Field Correction) Status:

        * FFC Never Commanded: Only seen on startup before first FFC.
        * FFC Imminent: This state is entered 2 seconds prior to initiating FFC.
        * FFC In Progress: Flat field correction is started (shutter moves in front of lens and back). Takes about 1 second.
        * FFC Complete: Shutter is in waiting position again, FFC done.

        Temperature warning bits:

        * Index 0: Shutter lockout (if true shutter is locked out because temperature is outside -10°C to +65°C)
        * Index 1: Overtemperature shut down imminent (goes true 10 seconds before shutdown)
        """
        self.check_validity()

        return GetStatistics(*self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_GET_STATISTICS, (), '', 27, '4H 4H B B 2!'))

    def set_resolution(self, resolution):
        r"""
        Sets the resolution. The Thermal Imaging Bricklet can either measure

        * from 0 to 6553 Kelvin (-273.15°C to +6279.85°C) with 0.1°C resolution or
        * from 0 to 655 Kelvin (-273.15°C to +381.85°C) with 0.01°C resolution.

        The accuracy is specified for -10°C to 450°C in the
        first range and -10°C and 140°C in the second range.
        """
        self.check_validity()

        resolution = int(resolution)

        self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_SET_RESOLUTION, (resolution,), 'B', 0, '')

    def get_resolution(self):
        r"""
        Returns the resolution as set by :func:`Set Resolution`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_GET_RESOLUTION, (), '', 9, 'B')

    def set_spotmeter_config(self, region_of_interest):
        r"""
        Sets the spotmeter region of interest. The 4 values are

        * Index 0: Column start (has to be smaller then Column end).
        * Index 1: Row start (has to be smaller then Row end).
        * Index 2: Column end (has to be smaller then 80).
        * Index 3: Row end (has to be smaller then 60).

        The spotmeter statistics can be read out with :func:`Get Statistics`.
        """
        self.check_validity()

        region_of_interest = list(map(int, region_of_interest))

        self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_SET_SPOTMETER_CONFIG, (region_of_interest,), '4B', 0, '')

    def get_spotmeter_config(self):
        r"""
        Returns the spotmeter config as set by :func:`Set Spotmeter Config`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_GET_SPOTMETER_CONFIG, (), '', 12, '4B')

    def set_high_contrast_config(self, region_of_interest, dampening_factor, clip_limit, empty_counts):
        r"""
        Sets the high contrast region of interest, dampening factor, clip limit and empty counts.
        This config is only used in high contrast mode (see :func:`Set Image Transfer Config`).

        The high contrast region of interest consists of four values:

        * Index 0: Column start (has to be smaller or equal then Column end).
        * Index 1: Row start (has to be smaller then Row end).
        * Index 2: Column end (has to be smaller then 80).
        * Index 3: Row end (has to be smaller then 60).

        The algorithm to generate the high contrast image is applied to this region.

        Dampening Factor: This parameter is the amount of temporal dampening applied to the HEQ
        (history equalization) transformation function. An IIR filter of the form::

         (N / 256) * previous + ((256 - N) / 256) * current

        is applied, and the HEQ dampening factor
        represents the value N in the equation, i.e., a value that applies to the amount of
        influence the previous HEQ transformation function has on the current function. The
        lower the value of N the higher the influence of the current video frame whereas
        the higher the value of N the more influence the previous damped transfer function has.

        Clip Limit Index 0 (AGC HEQ Clip Limit High): This parameter defines the maximum number of pixels allowed
        to accumulate in any given histogram bin. Any additional pixels in a given bin are clipped.
        The effect of this parameter is to limit the influence of highly-populated bins on the
        resulting HEQ transformation function.

        Clip Limit Index 1 (AGC HEQ Clip Limit Low): This parameter defines an artificial population that is added to
        every non-empty histogram bin. In other words, if the Clip Limit Low is set to L, a bin
        with an actual population of X will have an effective population of L + X. Any empty bin
        that is nearby a populated bin will be given an artificial population of L. The effect of
        higher values is to provide a more linear transfer function; lower values provide a more
        non-linear (equalized) transfer function.

        Empty Counts: This parameter specifies the maximum number of pixels in a bin that will be
        interpreted as an empty bin. Histogram bins with this number of pixels or less will be
        processed as an empty bin.
        """
        self.check_validity()

        region_of_interest = list(map(int, region_of_interest))
        dampening_factor = int(dampening_factor)
        clip_limit = list(map(int, clip_limit))
        empty_counts = int(empty_counts)

        self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_SET_HIGH_CONTRAST_CONFIG, (region_of_interest, dampening_factor, clip_limit, empty_counts), '4B H 2H H', 0, '')

    def get_high_contrast_config(self):
        r"""
        Returns the high contrast config as set by :func:`Set High Contrast Config`.
        """
        self.check_validity()

        return GetHighContrastConfig(*self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_GET_HIGH_CONTRAST_CONFIG, (), '', 20, '4B H 2H H'))

    def set_image_transfer_config(self, config):
        r"""
        The necessary bandwidth of this Bricklet is too high to use getter/callback or
        high contrast/temperature image at the same time. You have to configure the one
        you want to use, the Bricklet will optimize the internal configuration accordingly.

        Corresponding functions:

        * Manual High Contrast Image: :func:`Get High Contrast Image`.
        * Manual Temperature Image: :func:`Get Temperature Image`.
        * Callback High Contrast Image: :cb:`High Contrast Image` callback.
        * Callback Temperature Image: :cb:`Temperature Image` callback.
        """
        self.check_validity()

        config = int(config)

        self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_SET_IMAGE_TRANSFER_CONFIG, (config,), 'B', 0, '')

    def get_image_transfer_config(self):
        r"""
        Returns the image transfer config, as set by :func:`Set Image Transfer Config`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_GET_IMAGE_TRANSFER_CONFIG, (), '', 9, 'B')

    def set_flux_linear_parameters(self, scene_emissivity, temperature_background, tau_window, temperatur_window, tau_atmosphere, temperature_atmosphere, reflection_window, temperature_reflection):
        r"""
        Sets the flux linear parameters that can be used for radiometry calibration.

        See FLIR document 102-PS245-100-01 for more details.

        .. versionadded:: 2.0.5$nbsp;(Plugin)
        """
        self.check_validity()

        scene_emissivity = int(scene_emissivity)
        temperature_background = int(temperature_background)
        tau_window = int(tau_window)
        temperatur_window = int(temperatur_window)
        tau_atmosphere = int(tau_atmosphere)
        temperature_atmosphere = int(temperature_atmosphere)
        reflection_window = int(reflection_window)
        temperature_reflection = int(temperature_reflection)

        self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_SET_FLUX_LINEAR_PARAMETERS, (scene_emissivity, temperature_background, tau_window, temperatur_window, tau_atmosphere, temperature_atmosphere, reflection_window, temperature_reflection), 'H H H H H H H H', 0, '')

    def get_flux_linear_parameters(self):
        r"""
        Returns the flux linear parameters, as set by :func:`Set Flux Linear Parameters`.

        .. versionadded:: 2.0.5$nbsp;(Plugin)
        """
        self.check_validity()

        return GetFluxLinearParameters(*self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_GET_FLUX_LINEAR_PARAMETERS, (), '', 24, 'H H H H H H H H'))

    def set_ffc_shutter_mode(self, shutter_mode, temp_lockout_state, video_freeze_during_ffc, ffc_desired, elapsed_time_since_last_ffc, desired_ffc_period, explicit_cmd_to_open, desired_ffc_temp_delta, imminent_delay):
        r"""
        Sets the FFC shutter mode parameters.

        See FLIR document 110-0144-03 4.5.15 for more details.

        .. versionadded:: 2.0.6$nbsp;(Plugin)
        """
        self.check_validity()

        shutter_mode = int(shutter_mode)
        temp_lockout_state = int(temp_lockout_state)
        video_freeze_during_ffc = bool(video_freeze_during_ffc)
        ffc_desired = bool(ffc_desired)
        elapsed_time_since_last_ffc = int(elapsed_time_since_last_ffc)
        desired_ffc_period = int(desired_ffc_period)
        explicit_cmd_to_open = bool(explicit_cmd_to_open)
        desired_ffc_temp_delta = int(desired_ffc_temp_delta)
        imminent_delay = int(imminent_delay)

        self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_SET_FFC_SHUTTER_MODE, (shutter_mode, temp_lockout_state, video_freeze_during_ffc, ffc_desired, elapsed_time_since_last_ffc, desired_ffc_period, explicit_cmd_to_open, desired_ffc_temp_delta, imminent_delay), 'B B ! ! I I ! H H', 0, '')

    def get_ffc_shutter_mode(self):
        r"""
        Sets the FFC shutter mode parameters.

        See FLIR document 110-0144-03 4.5.15 for more details.

        .. versionadded:: 2.0.6$nbsp;(Plugin)
        """
        self.check_validity()

        return GetFFCShutterMode(*self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_GET_FFC_SHUTTER_MODE, (), '', 25, 'B B ! ! I I ! H H'))

    def run_ffc_normalization(self):
        r"""
        Starts the Flat-Field Correction (FFC) normalization.

        See FLIR document 110-0144-03 4.5.16 for more details.

        .. versionadded:: 2.0.6$nbsp;(Plugin)
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_RUN_FFC_NORMALIZATION, (), '', 0, '')

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletThermalImaging.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def get_high_contrast_image(self):
        r"""
        Returns the current high contrast image. See `here <https://www.tinkerforge.com/en/doc/Hardware/Bricklets/Thermal_Imaging.html#high-contrast-image-vs-temperature-image>`__
        for the difference between
        High Contrast and Temperature Image. If you don't know what to use
        the High Contrast Image is probably right for you.

        The data is organized as a 8-bit value 80x60 pixel matrix linearized in
        a one-dimensional array. The data is arranged line by line from top left to
        bottom right.

        Each 8-bit value represents one gray-scale image pixel that can directly be
        shown to a user on a display.

        Before you can use this function you have to enable it with
        :func:`Set Image Transfer Config`.
        """
        image_length = 4800

        with self.stream_lock:
            ret = self.get_high_contrast_image_low_level()

            if ret.image_chunk_offset == (1 << 16) - 1: # maximum chunk offset -> stream has no data
                image_length = 0
                image_out_of_sync = False
                image_data = ()
            else:
                image_out_of_sync = ret.image_chunk_offset != 0
                image_data = ret.image_chunk_data

            while not image_out_of_sync and len(image_data) < image_length:
                ret = self.get_high_contrast_image_low_level()
                image_out_of_sync = ret.image_chunk_offset != len(image_data)
                image_data += ret.image_chunk_data

            if image_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.image_chunk_offset + 62 < image_length:
                    ret = self.get_high_contrast_image_low_level()

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Image stream is out-of-sync')

        return image_data[:image_length]

    def get_temperature_image(self):
        r"""
        Returns the current temperature image. See `here <https://www.tinkerforge.com/en/doc/Hardware/Bricklets/Thermal_Imaging.html#high-contrast-image-vs-temperature-image>`__
        for the difference between High Contrast and Temperature Image.
        If you don't know what to use the High Contrast Image is probably right for you.

        The data is organized as a 16-bit value 80x60 pixel matrix linearized in
        a one-dimensional array. The data is arranged line by line from top left to
        bottom right.

        Each 16-bit value represents one temperature measurement in either
        Kelvin/10 or Kelvin/100 (depending on the resolution set with :func:`Set Resolution`).

        Before you can use this function you have to enable it with
        :func:`Set Image Transfer Config`.
        """
        image_length = 4800

        with self.stream_lock:
            ret = self.get_temperature_image_low_level()

            if ret.image_chunk_offset == (1 << 16) - 1: # maximum chunk offset -> stream has no data
                image_length = 0
                image_out_of_sync = False
                image_data = ()
            else:
                image_out_of_sync = ret.image_chunk_offset != 0
                image_data = ret.image_chunk_data

            while not image_out_of_sync and len(image_data) < image_length:
                ret = self.get_temperature_image_low_level()
                image_out_of_sync = ret.image_chunk_offset != len(image_data)
                image_data += ret.image_chunk_data

            if image_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.image_chunk_offset + 31 < image_length:
                    ret = self.get_temperature_image_low_level()

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Image stream is out-of-sync')

        return image_data[:image_length]

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

ThermalImaging = BrickletThermalImaging # for backward compatibility
