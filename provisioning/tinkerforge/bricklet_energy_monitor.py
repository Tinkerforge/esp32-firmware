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

GetEnergyData = namedtuple('EnergyData', ['voltage', 'current', 'energy', 'real_power', 'apparent_power', 'reactive_power', 'power_factor', 'frequency'])
GetWaveformLowLevel = namedtuple('WaveformLowLevel', ['waveform_chunk_offset', 'waveform_chunk_data'])
GetTransformerStatus = namedtuple('TransformerStatus', ['voltage_transformer_connected', 'current_transformer_connected'])
GetTransformerCalibration = namedtuple('TransformerCalibration', ['voltage_ratio', 'current_ratio', 'phase_shift'])
GetEnergyDataCallbackConfiguration = namedtuple('EnergyDataCallbackConfiguration', ['period', 'value_has_to_change'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletEnergyMonitor(Device):
    r"""
    Measures Voltage, Current, Energy, Real/Apparent/Reactive Power, Power Factor and Frequency
    """

    DEVICE_IDENTIFIER = 2152
    DEVICE_DISPLAY_NAME = 'Energy Monitor Bricklet'
    DEVICE_URL_PART = 'energy_monitor' # internal

    CALLBACK_ENERGY_DATA = 10


    FUNCTION_GET_ENERGY_DATA = 1
    FUNCTION_RESET_ENERGY = 2
    FUNCTION_GET_WAVEFORM_LOW_LEVEL = 3
    FUNCTION_GET_TRANSFORMER_STATUS = 4
    FUNCTION_SET_TRANSFORMER_CALIBRATION = 5
    FUNCTION_GET_TRANSFORMER_CALIBRATION = 6
    FUNCTION_CALIBRATE_OFFSET = 7
    FUNCTION_SET_ENERGY_DATA_CALLBACK_CONFIGURATION = 8
    FUNCTION_GET_ENERGY_DATA_CALLBACK_CONFIGURATION = 9
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
        Device.__init__(self, uid, ipcon, BrickletEnergyMonitor.DEVICE_IDENTIFIER, BrickletEnergyMonitor.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletEnergyMonitor.FUNCTION_GET_ENERGY_DATA] = BrickletEnergyMonitor.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEnergyMonitor.FUNCTION_RESET_ENERGY] = BrickletEnergyMonitor.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEnergyMonitor.FUNCTION_GET_WAVEFORM_LOW_LEVEL] = BrickletEnergyMonitor.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEnergyMonitor.FUNCTION_GET_TRANSFORMER_STATUS] = BrickletEnergyMonitor.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEnergyMonitor.FUNCTION_SET_TRANSFORMER_CALIBRATION] = BrickletEnergyMonitor.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEnergyMonitor.FUNCTION_GET_TRANSFORMER_CALIBRATION] = BrickletEnergyMonitor.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEnergyMonitor.FUNCTION_CALIBRATE_OFFSET] = BrickletEnergyMonitor.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEnergyMonitor.FUNCTION_SET_ENERGY_DATA_CALLBACK_CONFIGURATION] = BrickletEnergyMonitor.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletEnergyMonitor.FUNCTION_GET_ENERGY_DATA_CALLBACK_CONFIGURATION] = BrickletEnergyMonitor.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEnergyMonitor.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletEnergyMonitor.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEnergyMonitor.FUNCTION_SET_BOOTLOADER_MODE] = BrickletEnergyMonitor.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEnergyMonitor.FUNCTION_GET_BOOTLOADER_MODE] = BrickletEnergyMonitor.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEnergyMonitor.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletEnergyMonitor.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEnergyMonitor.FUNCTION_WRITE_FIRMWARE] = BrickletEnergyMonitor.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEnergyMonitor.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletEnergyMonitor.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEnergyMonitor.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletEnergyMonitor.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEnergyMonitor.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletEnergyMonitor.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEnergyMonitor.FUNCTION_RESET] = BrickletEnergyMonitor.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEnergyMonitor.FUNCTION_WRITE_UID] = BrickletEnergyMonitor.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletEnergyMonitor.FUNCTION_READ_UID] = BrickletEnergyMonitor.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletEnergyMonitor.FUNCTION_GET_IDENTITY] = BrickletEnergyMonitor.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletEnergyMonitor.CALLBACK_ENERGY_DATA] = (36, 'i i i i i i H H')

        ipcon.add_device(self)

    def get_energy_data(self):
        r"""
        Returns all of the measurements that are done by the Energy Monitor Bricklet.

        * Voltage RMS
        * Current RMS
        * Energy (integrated over time)
        * Real Power
        * Apparent Power
        * Reactive Power
        * Power Factor
        * Frequency (AC Frequency of the mains voltage)

        The frequency is recalculated every 6 seconds.

        All other values are integrated over 10 zero-crossings of the voltage sine wave.
        With a standard AC mains voltage frequency of 50Hz this results in a 5 measurements
        per second (or an integration time of 200ms per measurement).

        If no voltage transformer is connected, the Bricklet will use the current waveform
        to calculate the frequency and it will use an integration time of
        10 zero-crossings of the current waveform.
        """
        self.check_validity()

        return GetEnergyData(*self.ipcon.send_request(self, BrickletEnergyMonitor.FUNCTION_GET_ENERGY_DATA, (), '', 36, 'i i i i i i H H'))

    def reset_energy(self):
        r"""
        Sets the energy value (see :func:`Get Energy Data`) back to 0Wh.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletEnergyMonitor.FUNCTION_RESET_ENERGY, (), '', 0, '')

    def get_waveform_low_level(self):
        r"""
        Returns a snapshot of the voltage and current waveform. The values
        in the returned array alternate between voltage and current. The data from
        one getter call contains 768 data points for voltage and current, which
        correspond to about 3 full sine waves.

        The voltage is given with a resolution of 100mV and the current is given
        with a resolution of 10mA.

        This data is meant to be used for a non-realtime graphical representation of
        the voltage and current waveforms.
        """
        self.check_validity()

        return GetWaveformLowLevel(*self.ipcon.send_request(self, BrickletEnergyMonitor.FUNCTION_GET_WAVEFORM_LOW_LEVEL, (), '', 70, 'H 30h'))

    def get_transformer_status(self):
        r"""
        Returns *true* if a voltage/current transformer is connected to the Bricklet.
        """
        self.check_validity()

        return GetTransformerStatus(*self.ipcon.send_request(self, BrickletEnergyMonitor.FUNCTION_GET_TRANSFORMER_STATUS, (), '', 10, '! !'))

    def set_transformer_calibration(self, voltage_ratio, current_ratio, phase_shift):
        r"""
        Sets the transformer ratio for the voltage and current transformer in 1/100 form.

        Example: If your mains voltage is 230V, you use 9V voltage transformer and a
        1V:30A current clamp your voltage ratio is 230/9 = 25.56 and your current ratio
        is 30/1 = 30.

        In this case you have to set the values 2556 and 3000 for voltage ratio and current
        ratio.

        The calibration is saved in non-volatile memory, you only have to set it once.

        Set the phase shift to 0. It is for future use and currently not supported by the Bricklet.
        """
        self.check_validity()

        voltage_ratio = int(voltage_ratio)
        current_ratio = int(current_ratio)
        phase_shift = int(phase_shift)

        self.ipcon.send_request(self, BrickletEnergyMonitor.FUNCTION_SET_TRANSFORMER_CALIBRATION, (voltage_ratio, current_ratio, phase_shift), 'H H h', 0, '')

    def get_transformer_calibration(self):
        r"""
        Returns the transformer calibration as set by :func:`Set Transformer Calibration`.
        """
        self.check_validity()

        return GetTransformerCalibration(*self.ipcon.send_request(self, BrickletEnergyMonitor.FUNCTION_GET_TRANSFORMER_CALIBRATION, (), '', 14, 'H H h'))

    def calibrate_offset(self):
        r"""
        Calling this function will start an offset calibration. The offset calibration will
        integrate the voltage and current waveform over a longer time period to find the 0
        transition point in the sine wave.

        The Bricklet comes with a factory-calibrated offset value, you should not have to
        call this function.

        If you want to re-calibrate the offset we recommend that you connect a load that
        has a smooth sinusoidal voltage and current waveform. Alternatively you can also
        short both inputs.

        The calibration is saved in non-volatile memory, you only have to set it once.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletEnergyMonitor.FUNCTION_CALIBRATE_OFFSET, (), '', 0, '')

    def set_energy_data_callback_configuration(self, period, value_has_to_change):
        r"""
        The period is the period with which the :cb:`Energy Data`
        callback is triggered periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change within the
        period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.
        """
        self.check_validity()

        period = int(period)
        value_has_to_change = bool(value_has_to_change)

        self.ipcon.send_request(self, BrickletEnergyMonitor.FUNCTION_SET_ENERGY_DATA_CALLBACK_CONFIGURATION, (period, value_has_to_change), 'I !', 0, '')

    def get_energy_data_callback_configuration(self):
        r"""
        Returns the callback configuration as set by
        :func:`Set Energy Data Callback Configuration`.
        """
        self.check_validity()

        return GetEnergyDataCallbackConfiguration(*self.ipcon.send_request(self, BrickletEnergyMonitor.FUNCTION_GET_ENERGY_DATA_CALLBACK_CONFIGURATION, (), '', 13, 'I !'))

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletEnergyMonitor.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletEnergyMonitor.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletEnergyMonitor.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletEnergyMonitor.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletEnergyMonitor.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletEnergyMonitor.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletEnergyMonitor.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletEnergyMonitor.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletEnergyMonitor.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletEnergyMonitor.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletEnergyMonitor.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletEnergyMonitor.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def get_waveform(self):
        r"""
        Returns a snapshot of the voltage and current waveform. The values
        in the returned array alternate between voltage and current. The data from
        one getter call contains 768 data points for voltage and current, which
        correspond to about 3 full sine waves.

        The voltage is given with a resolution of 100mV and the current is given
        with a resolution of 10mA.

        This data is meant to be used for a non-realtime graphical representation of
        the voltage and current waveforms.
        """
        waveform_length = 1536

        with self.stream_lock:
            ret = self.get_waveform_low_level()

            if ret.waveform_chunk_offset == (1 << 16) - 1: # maximum chunk offset -> stream has no data
                waveform_length = 0
                waveform_out_of_sync = False
                waveform_data = ()
            else:
                waveform_out_of_sync = ret.waveform_chunk_offset != 0
                waveform_data = ret.waveform_chunk_data

            while not waveform_out_of_sync and len(waveform_data) < waveform_length:
                ret = self.get_waveform_low_level()
                waveform_out_of_sync = ret.waveform_chunk_offset != len(waveform_data)
                waveform_data += ret.waveform_chunk_data

            if waveform_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.waveform_chunk_offset + 30 < waveform_length:
                    ret = self.get_waveform_low_level()

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Waveform stream is out-of-sync')

        return waveform_data[:waveform_length]

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

EnergyMonitor = BrickletEnergyMonitor # for backward compatibility
