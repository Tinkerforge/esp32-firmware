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

GetStatus = namedtuple('Status', ['enabled', 'current_position', 'current_velocity', 'current', 'input_voltage'])
GetMotionConfiguration = namedtuple('MotionConfiguration', ['velocity', 'acceleration', 'deceleration'])
GetPulseWidth = namedtuple('PulseWidth', ['min', 'max'])
GetDegree = namedtuple('Degree', ['min', 'max'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletServoV2(Device):
    r"""
    Drives up to 10 RC Servos
    """

    DEVICE_IDENTIFIER = 2157
    DEVICE_DISPLAY_NAME = 'Servo Bricklet 2.0'
    DEVICE_URL_PART = 'servo_v2' # internal

    CALLBACK_POSITION_REACHED = 27


    FUNCTION_GET_STATUS = 1
    FUNCTION_SET_ENABLE = 2
    FUNCTION_GET_ENABLED = 3
    FUNCTION_SET_POSITION = 4
    FUNCTION_GET_POSITION = 5
    FUNCTION_GET_CURRENT_POSITION = 6
    FUNCTION_GET_CURRENT_VELOCITY = 7
    FUNCTION_SET_MOTION_CONFIGURATION = 8
    FUNCTION_GET_MOTION_CONFIGURATION = 9
    FUNCTION_SET_PULSE_WIDTH = 10
    FUNCTION_GET_PULSE_WIDTH = 11
    FUNCTION_SET_DEGREE = 12
    FUNCTION_GET_DEGREE = 13
    FUNCTION_SET_PERIOD = 14
    FUNCTION_GET_PERIOD = 15
    FUNCTION_GET_SERVO_CURRENT = 16
    FUNCTION_SET_SERVO_CURRENT_CONFIGURATION = 17
    FUNCTION_GET_SERVO_CURRENT_CONFIGURATION = 18
    FUNCTION_SET_INPUT_VOLTAGE_CONFIGURATION = 19
    FUNCTION_GET_INPUT_VOLTAGE_CONFIGURATION = 20
    FUNCTION_GET_OVERALL_CURRENT = 21
    FUNCTION_GET_INPUT_VOLTAGE = 22
    FUNCTION_SET_CURRENT_CALIBRATION = 23
    FUNCTION_GET_CURRENT_CALIBRATION = 24
    FUNCTION_SET_POSITION_REACHED_CALLBACK_CONFIGURATION = 25
    FUNCTION_GET_POSITION_REACHED_CALLBACK_CONFIGURATION = 26
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
        Device.__init__(self, uid, ipcon, BrickletServoV2.DEVICE_IDENTIFIER, BrickletServoV2.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletServoV2.FUNCTION_GET_STATUS] = BrickletServoV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletServoV2.FUNCTION_SET_ENABLE] = BrickletServoV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletServoV2.FUNCTION_GET_ENABLED] = BrickletServoV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletServoV2.FUNCTION_SET_POSITION] = BrickletServoV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletServoV2.FUNCTION_GET_POSITION] = BrickletServoV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletServoV2.FUNCTION_GET_CURRENT_POSITION] = BrickletServoV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletServoV2.FUNCTION_GET_CURRENT_VELOCITY] = BrickletServoV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletServoV2.FUNCTION_SET_MOTION_CONFIGURATION] = BrickletServoV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletServoV2.FUNCTION_GET_MOTION_CONFIGURATION] = BrickletServoV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletServoV2.FUNCTION_SET_PULSE_WIDTH] = BrickletServoV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletServoV2.FUNCTION_GET_PULSE_WIDTH] = BrickletServoV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletServoV2.FUNCTION_SET_DEGREE] = BrickletServoV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletServoV2.FUNCTION_GET_DEGREE] = BrickletServoV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletServoV2.FUNCTION_SET_PERIOD] = BrickletServoV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletServoV2.FUNCTION_GET_PERIOD] = BrickletServoV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletServoV2.FUNCTION_GET_SERVO_CURRENT] = BrickletServoV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletServoV2.FUNCTION_SET_SERVO_CURRENT_CONFIGURATION] = BrickletServoV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletServoV2.FUNCTION_GET_SERVO_CURRENT_CONFIGURATION] = BrickletServoV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletServoV2.FUNCTION_SET_INPUT_VOLTAGE_CONFIGURATION] = BrickletServoV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletServoV2.FUNCTION_GET_INPUT_VOLTAGE_CONFIGURATION] = BrickletServoV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletServoV2.FUNCTION_GET_OVERALL_CURRENT] = BrickletServoV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletServoV2.FUNCTION_GET_INPUT_VOLTAGE] = BrickletServoV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletServoV2.FUNCTION_SET_CURRENT_CALIBRATION] = BrickletServoV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletServoV2.FUNCTION_GET_CURRENT_CALIBRATION] = BrickletServoV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletServoV2.FUNCTION_SET_POSITION_REACHED_CALLBACK_CONFIGURATION] = BrickletServoV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletServoV2.FUNCTION_GET_POSITION_REACHED_CALLBACK_CONFIGURATION] = BrickletServoV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletServoV2.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletServoV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletServoV2.FUNCTION_SET_BOOTLOADER_MODE] = BrickletServoV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletServoV2.FUNCTION_GET_BOOTLOADER_MODE] = BrickletServoV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletServoV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletServoV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletServoV2.FUNCTION_WRITE_FIRMWARE] = BrickletServoV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletServoV2.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletServoV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletServoV2.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletServoV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletServoV2.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletServoV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletServoV2.FUNCTION_RESET] = BrickletServoV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletServoV2.FUNCTION_WRITE_UID] = BrickletServoV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletServoV2.FUNCTION_READ_UID] = BrickletServoV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletServoV2.FUNCTION_GET_IDENTITY] = BrickletServoV2.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletServoV2.CALLBACK_POSITION_REACHED] = (12, 'H h')

        ipcon.add_device(self)

    def get_status(self):
        r"""
        Returns the status information of the Servo Bricklet 2.0.

        The status includes

        * for each channel if it is enabled or disabled,
        * for each channel the current position,
        * for each channel the current velocity,
        * for each channel the current usage and
        * the input voltage.

        Please note that the position and the velocity is a snapshot of the
        current position and velocity of the servo in motion.
        """
        self.check_validity()

        return GetStatus(*self.ipcon.send_request(self, BrickletServoV2.FUNCTION_GET_STATUS, (), '', 72, '10! 10h 10h 10H H'))

    def set_enable(self, servo_channel, enable):
        r"""
        Enables a servo channel (0 to 9). If a servo is enabled, the configured position,
        velocity, acceleration, etc. are applied immediately.
        """
        self.check_validity()

        servo_channel = int(servo_channel)
        enable = bool(enable)

        self.ipcon.send_request(self, BrickletServoV2.FUNCTION_SET_ENABLE, (servo_channel, enable), 'H !', 0, '')

    def get_enabled(self, servo_channel):
        r"""
        Returns *true* if the specified servo channel is enabled, *false* otherwise.
        """
        self.check_validity()

        servo_channel = int(servo_channel)

        return self.ipcon.send_request(self, BrickletServoV2.FUNCTION_GET_ENABLED, (servo_channel,), 'H', 9, '!')

    def set_position(self, servo_channel, position):
        r"""
        Sets the position in °/100 for the specified servo channel.

        The default range of the position is -9000 to 9000, but it can be specified
        according to your servo with :func:`Set Degree`.

        If you want to control a linear servo or RC brushless motor controller or
        similar with the Servo Brick, you can also define lengths or speeds with
        :func:`Set Degree`.
        """
        self.check_validity()

        servo_channel = int(servo_channel)
        position = int(position)

        self.ipcon.send_request(self, BrickletServoV2.FUNCTION_SET_POSITION, (servo_channel, position), 'H h', 0, '')

    def get_position(self, servo_channel):
        r"""
        Returns the position of the specified servo channel as set by :func:`Set Position`.
        """
        self.check_validity()

        servo_channel = int(servo_channel)

        return self.ipcon.send_request(self, BrickletServoV2.FUNCTION_GET_POSITION, (servo_channel,), 'H', 10, 'h')

    def get_current_position(self, servo_channel):
        r"""
        Returns the *current* position of the specified servo channel. This may not be the
        value of :func:`Set Position` if the servo is currently approaching a
        position goal.
        """
        self.check_validity()

        servo_channel = int(servo_channel)

        return self.ipcon.send_request(self, BrickletServoV2.FUNCTION_GET_CURRENT_POSITION, (servo_channel,), 'H', 10, 'h')

    def get_current_velocity(self, servo_channel):
        r"""
        Returns the *current* velocity of the specified servo channel. This may not be the
        velocity specified by :func:`Set Motion Configuration`. if the servo is
        currently approaching a velocity goal.
        """
        self.check_validity()

        servo_channel = int(servo_channel)

        return self.ipcon.send_request(self, BrickletServoV2.FUNCTION_GET_CURRENT_VELOCITY, (servo_channel,), 'H', 10, 'H')

    def set_motion_configuration(self, servo_channel, velocity, acceleration, deceleration):
        r"""
        Sets the maximum velocity of the specified servo channel in °/100s as well as
        the acceleration and deceleration in °/100s²

        With a velocity of 0 °/100s the position will be set immediately (no velocity).

        With an acc-/deceleration of 0 °/100s² the velocity will be set immediately
        (no acc-/deceleration).
        """
        self.check_validity()

        servo_channel = int(servo_channel)
        velocity = int(velocity)
        acceleration = int(acceleration)
        deceleration = int(deceleration)

        self.ipcon.send_request(self, BrickletServoV2.FUNCTION_SET_MOTION_CONFIGURATION, (servo_channel, velocity, acceleration, deceleration), 'H I I I', 0, '')

    def get_motion_configuration(self, servo_channel):
        r"""
        Returns the motion configuration as set by :func:`Set Motion Configuration`.
        """
        self.check_validity()

        servo_channel = int(servo_channel)

        return GetMotionConfiguration(*self.ipcon.send_request(self, BrickletServoV2.FUNCTION_GET_MOTION_CONFIGURATION, (servo_channel,), 'H', 20, 'I I I'))

    def set_pulse_width(self, servo_channel, min, max):
        r"""
        Sets the minimum and maximum pulse width of the specified servo channel in µs.

        Usually, servos are controlled with a
        `PWM <https://en.wikipedia.org/wiki/Pulse-width_modulation>`__, whereby the
        length of the pulse controls the position of the servo. Every servo has
        different minimum and maximum pulse widths, these can be specified with
        this function.

        If you have a datasheet for your servo that specifies the minimum and
        maximum pulse width, you should set the values accordingly. If your servo
        comes without any datasheet you have to find the values via trial and error.

        Both values have a range from 1 to 65535 (unsigned 16-bit integer). The
        minimum must be smaller than the maximum.

        The default values are 1000µs (1ms) and 2000µs (2ms) for minimum and
        maximum pulse width.
        """
        self.check_validity()

        servo_channel = int(servo_channel)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickletServoV2.FUNCTION_SET_PULSE_WIDTH, (servo_channel, min, max), 'H I I', 0, '')

    def get_pulse_width(self, servo_channel):
        r"""
        Returns the minimum and maximum pulse width for the specified servo channel as set by
        :func:`Set Pulse Width`.
        """
        self.check_validity()

        servo_channel = int(servo_channel)

        return GetPulseWidth(*self.ipcon.send_request(self, BrickletServoV2.FUNCTION_GET_PULSE_WIDTH, (servo_channel,), 'H', 16, 'I I'))

    def set_degree(self, servo_channel, min, max):
        r"""
        Sets the minimum and maximum degree for the specified servo channel (by default
        given as °/100).

        This only specifies the abstract values between which the minimum and maximum
        pulse width is scaled. For example: If you specify a pulse width of 1000µs
        to 2000µs and a degree range of -90° to 90°, a call of :func:`Set Position`
        with 0 will result in a pulse width of 1500µs
        (-90° = 1000µs, 90° = 2000µs, etc.).

        Possible usage:

        * The datasheet of your servo specifies a range of 200° with the middle position
          at 110°. In this case you can set the minimum to -9000 and the maximum to 11000.
        * You measure a range of 220° on your servo and you don't have or need a middle
          position. In this case you can set the minimum to 0 and the maximum to 22000.
        * You have a linear servo with a drive length of 20cm, In this case you could
          set the minimum to 0 and the maximum to 20000. Now you can set the Position
          with :func:`Set Position` with a resolution of cm/100. Also the velocity will
          have a resolution of cm/100s and the acceleration will have a resolution of
          cm/100s².
        * You don't care about units and just want the highest possible resolution. In
          this case you should set the minimum to -32767 and the maximum to 32767.
        * You have a brushless motor with a maximum speed of 10000 rpm and want to
          control it with a RC brushless motor controller. In this case you can set the
          minimum to 0 and the maximum to 10000. :func:`Set Position` now controls the rpm.

        Both values have a possible range from -32767 to 32767
        (signed 16-bit integer). The minimum must be smaller than the maximum.

        The default values are -9000 and 9000 for the minimum and maximum degree.
        """
        self.check_validity()

        servo_channel = int(servo_channel)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickletServoV2.FUNCTION_SET_DEGREE, (servo_channel, min, max), 'H h h', 0, '')

    def get_degree(self, servo_channel):
        r"""
        Returns the minimum and maximum degree for the specified servo channel as set by
        :func:`Set Degree`.
        """
        self.check_validity()

        servo_channel = int(servo_channel)

        return GetDegree(*self.ipcon.send_request(self, BrickletServoV2.FUNCTION_GET_DEGREE, (servo_channel,), 'H', 12, 'h h'))

    def set_period(self, servo_channel, period):
        r"""
        Sets the period of the specified servo channel in µs.

        Usually, servos are controlled with a
        `PWM <https://en.wikipedia.org/wiki/Pulse-width_modulation>`__. Different
        servos expect PWMs with different periods. Most servos run well with a
        period of about 20ms.

        If your servo comes with a datasheet that specifies a period, you should
        set it accordingly. If you don't have a datasheet and you have no idea
        what the correct period is, the default value (19.5ms) will most likely
        work fine.

        The minimum possible period is 1µs and the maximum is 1000000µs.

        The default value is 19.5ms (19500µs).
        """
        self.check_validity()

        servo_channel = int(servo_channel)
        period = int(period)

        self.ipcon.send_request(self, BrickletServoV2.FUNCTION_SET_PERIOD, (servo_channel, period), 'H I', 0, '')

    def get_period(self, servo_channel):
        r"""
        Returns the period for the specified servo channel as set by :func:`Set Period`.
        """
        self.check_validity()

        servo_channel = int(servo_channel)

        return self.ipcon.send_request(self, BrickletServoV2.FUNCTION_GET_PERIOD, (servo_channel,), 'H', 12, 'I')

    def get_servo_current(self, servo_channel):
        r"""
        Returns the current consumption of the specified servo channel in mA.
        """
        self.check_validity()

        servo_channel = int(servo_channel)

        return self.ipcon.send_request(self, BrickletServoV2.FUNCTION_GET_SERVO_CURRENT, (servo_channel,), 'H', 10, 'H')

    def set_servo_current_configuration(self, servo_channel, averaging_duration):
        r"""
        Sets the averaging duration of the current measurement for the specified servo channel in ms.
        """
        self.check_validity()

        servo_channel = int(servo_channel)
        averaging_duration = int(averaging_duration)

        self.ipcon.send_request(self, BrickletServoV2.FUNCTION_SET_SERVO_CURRENT_CONFIGURATION, (servo_channel, averaging_duration), 'H B', 0, '')

    def get_servo_current_configuration(self, servo_channel):
        r"""
        Returns the servo current configuration for the specified servo channel as set
        by :func:`Set Servo Current Configuration`.
        """
        self.check_validity()

        servo_channel = int(servo_channel)

        return self.ipcon.send_request(self, BrickletServoV2.FUNCTION_GET_SERVO_CURRENT_CONFIGURATION, (servo_channel,), 'H', 9, 'B')

    def set_input_voltage_configuration(self, averaging_duration):
        r"""
        Sets the averaging duration of the input voltage measurement for the specified servo channel in ms.
        """
        self.check_validity()

        averaging_duration = int(averaging_duration)

        self.ipcon.send_request(self, BrickletServoV2.FUNCTION_SET_INPUT_VOLTAGE_CONFIGURATION, (averaging_duration,), 'B', 0, '')

    def get_input_voltage_configuration(self):
        r"""
        Returns the input voltage configuration as set by :func:`Set Input Voltage Configuration`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletServoV2.FUNCTION_GET_INPUT_VOLTAGE_CONFIGURATION, (), '', 9, 'B')

    def get_overall_current(self):
        r"""
        Returns the current consumption of all servos together in mA.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletServoV2.FUNCTION_GET_OVERALL_CURRENT, (), '', 10, 'H')

    def get_input_voltage(self):
        r"""
        Returns the input voltage in mV. The input voltage is
        given via the black power input connector on the Servo Brick.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletServoV2.FUNCTION_GET_INPUT_VOLTAGE, (), '', 10, 'H')

    def set_current_calibration(self, offset):
        r"""
        Sets an offset value (in mA) for each channel.

        Note: On delivery the Servo Bricklet 2.0 is already calibrated.
        """
        self.check_validity()

        offset = list(map(int, offset))

        self.ipcon.send_request(self, BrickletServoV2.FUNCTION_SET_CURRENT_CALIBRATION, (offset,), '10h', 0, '')

    def get_current_calibration(self):
        r"""
        Returns the current calibration as set by :func:`Set Current Calibration`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletServoV2.FUNCTION_GET_CURRENT_CALIBRATION, (), '', 28, '10h')

    def set_position_reached_callback_configuration(self, servo_channel, enabled):
        r"""
        Enable/Disable :cb:`Position Reached` callback.
        """
        self.check_validity()

        servo_channel = int(servo_channel)
        enabled = bool(enabled)

        self.ipcon.send_request(self, BrickletServoV2.FUNCTION_SET_POSITION_REACHED_CALLBACK_CONFIGURATION, (servo_channel, enabled), 'H !', 0, '')

    def get_position_reached_callback_configuration(self, servo_channel):
        r"""
        Returns the callback configuration as set by
        :func:`Set Position Reached Callback Configuration`.
        """
        self.check_validity()

        servo_channel = int(servo_channel)

        return self.ipcon.send_request(self, BrickletServoV2.FUNCTION_GET_POSITION_REACHED_CALLBACK_CONFIGURATION, (servo_channel,), 'H', 9, '!')

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletServoV2.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletServoV2.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletServoV2.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletServoV2.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletServoV2.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletServoV2.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletServoV2.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletServoV2.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletServoV2.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletServoV2.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletServoV2.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletServoV2.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

ServoV2 = BrickletServoV2 # for backward compatibility
