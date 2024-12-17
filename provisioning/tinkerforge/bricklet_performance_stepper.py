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

#### __DEVICE_IS_NOT_RELEASED__ ####

from collections import namedtuple

try:
    from .ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data
except (ValueError, ImportError):
    try:
        from ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data
    except (ValueError, ImportError):
        from tinkerforge.ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data

GetMotionConfiguration = namedtuple('MotionConfiguration', ['ramping_mode', 'velocity_start', 'acceleration_1', 'velocity_1', 'acceleration_max', 'velocity_max', 'deceleration_max', 'deceleration_1', 'velocity_stop', 'ramp_zero_wait'])
GetStepConfiguration = namedtuple('StepConfiguration', ['step_resolution', 'interpolation'])
GetBasicConfiguration = namedtuple('BasicConfiguration', ['standstill_current', 'motor_run_current', 'standstill_delay_time', 'power_down_time', 'stealth_threshold', 'coolstep_threshold', 'classic_threshold', 'high_velocity_chopper_mode'])
GetSpreadcycleConfiguration = namedtuple('SpreadcycleConfiguration', ['slow_decay_duration', 'high_velocity_fullstep', 'enable_random_slow_decay', 'fast_decay_duration', 'hysteresis_start_value', 'hysteresis_end_value', 'sine_wave_offset', 'chopper_mode', 'comparator_blank_time', 'fast_decay_without_comparator'])
GetStealthConfiguration = namedtuple('StealthConfiguration', ['enable_stealth', 'offset', 'gradient', 'enable_autoscale', 'enable_autogradient', 'freewheel_mode', 'regulation_loop_gradient', 'amplitude_limit'])
GetCoolstepConfiguration = namedtuple('CoolstepConfiguration', ['minimum_stallguard_value', 'maximum_stallguard_value', 'current_up_step_width', 'current_down_step_width', 'minimum_current', 'stallguard_threshold_value', 'stallguard_mode'])
GetShortConfiguration = namedtuple('ShortConfiguration', ['disable_short_to_voltage_protection', 'disable_short_to_ground_protection', 'short_to_voltage_level', 'short_to_ground_level', 'spike_filter_bandwidth', 'short_detection_delay', 'filter_time'])
GetDriverStatus = namedtuple('DriverStatus', ['open_load', 'short_to_ground', 'over_temperature', 'motor_stalled', 'actual_motor_current', 'full_step_active', 'stallguard_result', 'stealth_voltage_amplitude'])
GetGPIOConfiguration = namedtuple('GPIOConfiguration', ['debounce', 'stop_deceleration'])
ReadRegister = namedtuple('ReadRegister', ['status', 'value'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletPerformanceStepper(Device):
    r"""
    TBD
    """

    DEVICE_IDENTIFIER = 2158
    DEVICE_DISPLAY_NAME = 'Performance Stepper Bricklet'
    DEVICE_URL_PART = 'performance_stepper' # internal



    FUNCTION_SET_MOTION_CONFIGURATION = 1
    FUNCTION_GET_MOTION_CONFIGURATION = 2
    FUNCTION_SET_CURRENT_POSITION = 3
    FUNCTION_GET_CURRENT_POSITION = 4
    FUNCTION_GET_CURRENT_VELOCITY = 5
    FUNCTION_SET_TARGET_POSITION = 6
    FUNCTION_GET_TARGET_POSITION = 7
    FUNCTION_SET_STEPS = 8
    FUNCTION_GET_STEPS = 9
    FUNCTION_GET_REMAINING_STEPS = 10
    FUNCTION_SET_STEP_CONFIGURATION = 11
    FUNCTION_GET_STEP_CONFIGURATION = 12
    FUNCTION_SET_MOTOR_CURRENT = 22
    FUNCTION_GET_MOTOR_CURRENT = 23
    FUNCTION_SET_ENABLED = 24
    FUNCTION_GET_ENABLED = 25
    FUNCTION_SET_BASIC_CONFIGURATION = 26
    FUNCTION_GET_BASIC_CONFIGURATION = 27
    FUNCTION_SET_SPREADCYCLE_CONFIGURATION = 28
    FUNCTION_GET_SPREADCYCLE_CONFIGURATION = 29
    FUNCTION_SET_STEALTH_CONFIGURATION = 30
    FUNCTION_GET_STEALTH_CONFIGURATION = 31
    FUNCTION_SET_COOLSTEP_CONFIGURATION = 32
    FUNCTION_GET_COOLSTEP_CONFIGURATION = 33
    FUNCTION_SET_SHORT_CONFIGURATION = 34
    FUNCTION_GET_SHORT_CONFIGURATION = 35
    FUNCTION_GET_DRIVER_STATUS = 36
    FUNCTION_GET_INPUT_VOLTAGE = 37
    FUNCTION_GET_TEMPERATURE = 38
    FUNCTION_SET_GPIO_CONFIGURATION = 39
    FUNCTION_GET_GPIO_CONFIGURATION = 40
    FUNCTION_SET_GPIO_ACTION = 41
    FUNCTION_GET_GPIO_ACTION = 42
    FUNCTION_GET_GPIO_STATE = 43
    FUNCTION_SET_ERROR_LED_CONFIG = 44
    FUNCTION_GET_ERROR_LED_CONFIG = 45
    FUNCTION_SET_ENABLE_LED_CONFIG = 46
    FUNCTION_GET_ENABLE_LED_CONFIG = 47
    FUNCTION_SET_STEPS_LED_CONFIG = 48
    FUNCTION_GET_STEPS_LED_CONFIG = 49
    FUNCTION_SET_GPIO_LED_CONFIG = 50
    FUNCTION_GET_GPIO_LED_CONFIG = 51
    FUNCTION_WRITE_REGISTER = 52
    FUNCTION_READ_REGISTER = 53
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

    STEP_RESOLUTION_1 = 8
    STEP_RESOLUTION_2 = 7
    STEP_RESOLUTION_4 = 6
    STEP_RESOLUTION_8 = 5
    STEP_RESOLUTION_16 = 4
    STEP_RESOLUTION_32 = 3
    STEP_RESOLUTION_64 = 2
    STEP_RESOLUTION_128 = 1
    STEP_RESOLUTION_256 = 0
    RAMPING_MODE_POSITIONING = 0
    RAMPING_MODE_VELOCITY_NEGATIVE = 1
    RAMPING_MODE_VELOCITY_POSITIVE = 2
    RAMPING_MODE_HOLD = 3
    CHOPPER_MODE_SPREAD_CYCLE = 0
    CHOPPER_MODE_FAST_DECAY = 1
    FREEWHEEL_MODE_NORMAL = 0
    FREEWHEEL_MODE_FREEWHEELING = 1
    FREEWHEEL_MODE_COIL_SHORT_LS = 2
    FREEWHEEL_MODE_COIL_SHORT_HS = 3
    CURRENT_UP_STEP_INCREMENT_1 = 0
    CURRENT_UP_STEP_INCREMENT_2 = 1
    CURRENT_UP_STEP_INCREMENT_4 = 2
    CURRENT_UP_STEP_INCREMENT_8 = 3
    CURRENT_DOWN_STEP_DECREMENT_1 = 0
    CURRENT_DOWN_STEP_DECREMENT_2 = 1
    CURRENT_DOWN_STEP_DECREMENT_8 = 2
    CURRENT_DOWN_STEP_DECREMENT_32 = 3
    MINIMUM_CURRENT_HALF = 0
    MINIMUM_CURRENT_QUARTER = 1
    STALLGUARD_MODE_STANDARD = 0
    STALLGUARD_MODE_FILTERED = 1
    OPEN_LOAD_NONE = 0
    OPEN_LOAD_PHASE_A = 1
    OPEN_LOAD_PHASE_B = 2
    OPEN_LOAD_PHASE_AB = 3
    SHORT_TO_GROUND_NONE = 0
    SHORT_TO_GROUND_PHASE_A = 1
    SHORT_TO_GROUND_PHASE_B = 2
    SHORT_TO_GROUND_PHASE_AB = 3
    OVER_TEMPERATURE_NONE = 0
    OVER_TEMPERATURE_WARNING = 1
    OVER_TEMPERATURE_LIMIT = 2
    STATE_STOP = 1
    STATE_ACCELERATION = 2
    STATE_RUN = 3
    STATE_DEACCELERATION = 4
    STATE_DIRECTION_CHANGE_TO_FORWARD = 5
    STATE_DIRECTION_CHANGE_TO_BACKWARD = 6
    GPIO_ACTION_NONE = 0
    GPIO_ACTION_NORMAL_STOP_RISING_EDGE = 1
    GPIO_ACTION_NORMAL_STOP_FALLING_EDGE = 2
    GPIO_ACTION_EMERGENCY_STOP_RISING_EDGE = 4
    GPIO_ACTION_EMERGENCY_STOP_FALLING_EDGE = 8
    GPIO_ACTION_CALLBACK_RISING_EDGE = 16
    GPIO_ACTION_CALLBACK_FALLING_EDGE = 32
    ERROR_LED_CONFIG_OFF = 0
    ERROR_LED_CONFIG_ON = 1
    ERROR_LED_CONFIG_SHOW_HEARTBEAT = 2
    ERROR_LED_CONFIG_SHOW_ERROR = 3
    ENABLE_LED_CONFIG_OFF = 0
    ENABLE_LED_CONFIG_ON = 1
    ENABLE_LED_CONFIG_SHOW_HEARTBEAT = 2
    ENABLE_LED_CONFIG_SHOW_ENABLE = 3
    STEPS_LED_CONFIG_OFF = 0
    STEPS_LED_CONFIG_ON = 1
    STEPS_LED_CONFIG_SHOW_HEARTBEAT = 2
    STEPS_LED_CONFIG_SHOW_STEPS = 3
    GPIO_LED_CONFIG_OFF = 0
    GPIO_LED_CONFIG_ON = 1
    GPIO_LED_CONFIG_SHOW_HEARTBEAT = 2
    GPIO_LED_CONFIG_SHOW_GPIO_ACTIVE_HIGH = 3
    GPIO_LED_CONFIG_SHOW_GPIO_ACTIVE_LOW = 4
    FILTER_TIME_100 = 0
    FILTER_TIME_200 = 1
    FILTER_TIME_300 = 2
    FILTER_TIME_400 = 3
    SPIKE_FILTER_BANDWIDTH_100 = 0
    SPIKE_FILTER_BANDWIDTH_1000 = 1
    SPIKE_FILTER_BANDWIDTH_2000 = 2
    SPIKE_FILTER_BANDWIDTH_3000 = 3
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
        Device.__init__(self, uid, ipcon, BrickletPerformanceStepper.DEVICE_IDENTIFIER, BrickletPerformanceStepper.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletPerformanceStepper.FUNCTION_SET_MOTION_CONFIGURATION] = BrickletPerformanceStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_MOTION_CONFIGURATION] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_SET_CURRENT_POSITION] = BrickletPerformanceStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_CURRENT_POSITION] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_CURRENT_VELOCITY] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_SET_TARGET_POSITION] = BrickletPerformanceStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_TARGET_POSITION] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_SET_STEPS] = BrickletPerformanceStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_STEPS] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_REMAINING_STEPS] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_SET_STEP_CONFIGURATION] = BrickletPerformanceStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_STEP_CONFIGURATION] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_SET_MOTOR_CURRENT] = BrickletPerformanceStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_MOTOR_CURRENT] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_SET_ENABLED] = BrickletPerformanceStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_ENABLED] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_SET_BASIC_CONFIGURATION] = BrickletPerformanceStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_BASIC_CONFIGURATION] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_SET_SPREADCYCLE_CONFIGURATION] = BrickletPerformanceStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_SPREADCYCLE_CONFIGURATION] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_SET_STEALTH_CONFIGURATION] = BrickletPerformanceStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_STEALTH_CONFIGURATION] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_SET_COOLSTEP_CONFIGURATION] = BrickletPerformanceStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_COOLSTEP_CONFIGURATION] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_SET_SHORT_CONFIGURATION] = BrickletPerformanceStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_SHORT_CONFIGURATION] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_DRIVER_STATUS] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_INPUT_VOLTAGE] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_TEMPERATURE] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_SET_GPIO_CONFIGURATION] = BrickletPerformanceStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_GPIO_CONFIGURATION] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_SET_GPIO_ACTION] = BrickletPerformanceStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_GPIO_ACTION] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_GPIO_STATE] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_SET_ERROR_LED_CONFIG] = BrickletPerformanceStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_ERROR_LED_CONFIG] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_SET_ENABLE_LED_CONFIG] = BrickletPerformanceStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_ENABLE_LED_CONFIG] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_SET_STEPS_LED_CONFIG] = BrickletPerformanceStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_STEPS_LED_CONFIG] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_SET_GPIO_LED_CONFIG] = BrickletPerformanceStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_GPIO_LED_CONFIG] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_WRITE_REGISTER] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_READ_REGISTER] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_SET_BOOTLOADER_MODE] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_BOOTLOADER_MODE] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletPerformanceStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_WRITE_FIRMWARE] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletPerformanceStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_RESET] = BrickletPerformanceStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_WRITE_UID] = BrickletPerformanceStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_READ_UID] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletPerformanceStepper.FUNCTION_GET_IDENTITY] = BrickletPerformanceStepper.RESPONSE_EXPECTED_ALWAYS_TRUE


        ipcon.add_device(self)

    def set_motion_configuration(self, ramping_mode, velocity_start, acceleration_1, velocity_1, acceleration_max, velocity_max, deceleration_max, deceleration_1, velocity_stop, ramp_zero_wait):
        r"""
        TBD
        """
        self.check_validity()

        ramping_mode = int(ramping_mode)
        velocity_start = int(velocity_start)
        acceleration_1 = int(acceleration_1)
        velocity_1 = int(velocity_1)
        acceleration_max = int(acceleration_max)
        velocity_max = int(velocity_max)
        deceleration_max = int(deceleration_max)
        deceleration_1 = int(deceleration_1)
        velocity_stop = int(velocity_stop)
        ramp_zero_wait = int(ramp_zero_wait)

        self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_SET_MOTION_CONFIGURATION, (ramping_mode, velocity_start, acceleration_1, velocity_1, acceleration_max, velocity_max, deceleration_max, deceleration_1, velocity_stop, ramp_zero_wait), 'B i i i i i i i i i', 0, '')

    def get_motion_configuration(self):
        r"""
        TBD
        """
        self.check_validity()

        return GetMotionConfiguration(*self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_MOTION_CONFIGURATION, (), '', 45, 'B i i i i i i i i i'))

    def set_current_position(self, position):
        r"""
        Sets the current steps of the internal step counter. This can be used to
        set the current position to 0 when some kind of starting position
        is reached (e.g. when a CNC machine reaches a corner).
        """
        self.check_validity()

        position = int(position)

        self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_SET_CURRENT_POSITION, (position,), 'i', 0, '')

    def get_current_position(self):
        r"""
        Returns the current position of the stepper motor in steps. On startup
        the position is 0. The steps are counted with all possible driving
        functions (:func:`Set Target Position`, :func:`Set Steps`).
        It also is possible to reset the steps to 0 or
        set them to any other desired value with :func:`Set Current Position`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_CURRENT_POSITION, (), '', 12, 'i')

    def get_current_velocity(self):
        r"""
        TBD
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_CURRENT_VELOCITY, (), '', 12, 'i')

    def set_target_position(self, position):
        r"""
        Sets the target position of the stepper motor in steps. For example,
        if the current position of the motor is 500 and :func:`Set Target Position` is
        called with 1000, the stepper motor will drive 500 steps forward. It will
        use the velocity, acceleration and deacceleration as set by
        :func:`Set Motion Configuration`.

        A call of :func:`Set Target Position` with the parameter *x* is equivalent to
        a call of :func:`Set Steps` with the parameter
        (*x* - :func:`Get Current Position`).
        """
        self.check_validity()

        position = int(position)

        self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_SET_TARGET_POSITION, (position,), 'i', 0, '')

    def get_target_position(self):
        r"""
        Returns the last target position as set by :func:`Set Target Position`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_TARGET_POSITION, (), '', 12, 'i')

    def set_steps(self, steps):
        r"""
        Sets the number of steps the stepper motor should run. Positive values
        will drive the motor forward and negative values backward.
        The velocity, acceleration and deacceleration as set by
        :func:`Set Motion Configuration` will be used.
        """
        self.check_validity()

        steps = int(steps)

        self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_SET_STEPS, (steps,), 'i', 0, '')

    def get_steps(self):
        r"""
        Returns the last steps as set by :func:`Set Steps`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_STEPS, (), '', 12, 'i')

    def get_remaining_steps(self):
        r"""
        Returns the remaining steps of the last call of :func:`Set Steps`.
        For example, if :func:`Set Steps` is called with 2000 and
        :func:`Get Remaining Steps` is called after the motor has run for 500 steps,
        it will return 1500.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_REMAINING_STEPS, (), '', 12, 'i')

    def set_step_configuration(self, step_resolution, interpolation):
        r"""
        Sets the step resolution from full-step up to 1/256-step.

        If interpolation is turned on, the Silent Stepper Brick will always interpolate
        your step inputs as 1/256-step. If you use full-step mode with interpolation, each
        step will generate 256 1/256 steps.

        For maximum torque use full-step without interpolation. For maximum resolution use
        1/256-step. Turn interpolation on to make the Stepper driving less noisy.

        If you often change the speed with high acceleration you should turn the
        interpolation off.
        """
        self.check_validity()

        step_resolution = int(step_resolution)
        interpolation = bool(interpolation)

        self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_SET_STEP_CONFIGURATION, (step_resolution, interpolation), 'B !', 0, '')

    def get_step_configuration(self):
        r"""
        Returns the step mode as set by :func:`Set Step Configuration`.
        """
        self.check_validity()

        return GetStepConfiguration(*self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_STEP_CONFIGURATION, (), '', 10, 'B !'))

    def set_motor_current(self, current):
        r"""
        Sets the current with which the motor will be driven.

        .. warning::
         Do not set this value above the specifications of your stepper motor.
         Otherwise it may damage your motor.
        """
        self.check_validity()

        current = int(current)

        self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_SET_MOTOR_CURRENT, (current,), 'H', 0, '')

    def get_motor_current(self):
        r"""
        Returns the current as set by :func:`Set Motor Current`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_MOTOR_CURRENT, (), '', 10, 'H')

    def set_enabled(self, enabled):
        r"""
        Enables the driver chip. The driver parameters can be configured (maximum velocity,
        acceleration, etc) before it is enabled.
        """
        self.check_validity()

        enabled = bool(enabled)

        self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_SET_ENABLED, (enabled,), '!', 0, '')

    def get_enabled(self):
        r"""
        TBD
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_ENABLED, (), '', 9, '!')

    def set_basic_configuration(self, standstill_current, motor_run_current, standstill_delay_time, power_down_time, stealth_threshold, coolstep_threshold, classic_threshold, high_velocity_chopper_mode):
        r"""
        Sets the basic configuration parameters for the different modes (Stealth, Coolstep, Classic).

        * Standstill Current: This value can be used to lower the current during stand still. This might
          be reasonable to reduce the heating of the motor and the Brick. When the motor is in standstill
          the configured motor phase current will be driven until the configured
          Power Down Time is elapsed. After that the phase current will be reduced to the standstill
          current. The elapsed time for this reduction can be configured with the Standstill Delay Time.
          The maximum allowed value is the configured maximum motor current
          (see :func:`Set Motor Current`).

        * Motor Run Current: The value sets the motor current when the motor is running.
          Use a value of at least one half of the global maximum motor current for a good
          microstep performance. The maximum allowed value is the current
          motor current. The API maps the entered value to 1/32 ... 32/32 of the maximum
          motor current. This value should be used to change the motor current during motor movement,
          whereas the global maximum motor current should not be changed while the motor is moving
          (see :func:`Set Motor Current`).

        * Standstill Delay Time: Controls the duration for motor power down after a motion
          as soon as standstill is detected and the Power Down Time is expired. A high Standstill Delay
          Time results in a smooth transition that avoids motor jerk during power down.

        * Power Down Time: Sets the delay time after a stand still.

        * Stealth Threshold: Sets the upper threshold for Stealth mode.
          If the velocity of the motor goes above this value, Stealth mode is turned
          off. Otherwise it is turned on. In Stealth mode the torque declines with high speed.

        * Coolstep Threshold: Sets the lower threshold for Coolstep mode.
          The Coolstep Threshold needs to be above the Stealth Threshold.

        * Classic Threshold: Sets the lower threshold for classic mode.
          In classic mode the stepper becomes more noisy, but the torque is maximized.

        * High Velocity Chopper Mode: If High Velocity Chopper Mode is enabled, the stepper control
          is optimized to run the stepper motors at high velocities.

        If you want to use all three thresholds make sure that
        Stealth Threshold < Coolstep Threshold < Classic Threshold.
        """
        self.check_validity()

        standstill_current = int(standstill_current)
        motor_run_current = int(motor_run_current)
        standstill_delay_time = int(standstill_delay_time)
        power_down_time = int(power_down_time)
        stealth_threshold = int(stealth_threshold)
        coolstep_threshold = int(coolstep_threshold)
        classic_threshold = int(classic_threshold)
        high_velocity_chopper_mode = bool(high_velocity_chopper_mode)

        self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_SET_BASIC_CONFIGURATION, (standstill_current, motor_run_current, standstill_delay_time, power_down_time, stealth_threshold, coolstep_threshold, classic_threshold, high_velocity_chopper_mode), 'H H H H H H H !', 0, '')

    def get_basic_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Basic Configuration`.
        """
        self.check_validity()

        return GetBasicConfiguration(*self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_BASIC_CONFIGURATION, (), '', 23, 'H H H H H H H !'))

    def set_spreadcycle_configuration(self, slow_decay_duration, high_velocity_fullstep, fast_decay_duration, hysteresis_start_value, hysteresis_end_value, sine_wave_offset, chopper_mode, comparator_blank_time, fast_decay_without_comparator):
        r"""
        Note: If you don't know what any of this means you can very likely keep all of
        the values as default!

        Sets the Spreadcycle configuration parameters. Spreadcycle is a chopper algorithm which actively
        controls the motor current flow. More information can be found in the TMC2130 datasheet on page
        47 (7 spreadCycle and Classic Chopper).

        * Slow Decay Duration: Controls duration of off time setting of slow decay phase.
          0 = driver disabled, all bridges off. Use 1 only with Comparator Blank time >= 2.

        * High Velocity Fullstep: TODO.

        * Fast Decay Duration: Sets the fast decay duration. This parameters is
          only used if the Chopper Mode is set to Fast Decay.

        * Hysteresis Start Value: Sets the hysteresis start value. This parameter is
          only used if the Chopper Mode is set to Spread Cycle.

        * Hysteresis End Value: Sets the hysteresis end value. This parameter is
          only used if the Chopper Mode is set to Spread Cycle.

        * Sine Wave Offset: Sets the sine wave offset. This parameters is
          only used if the Chopper Mode is set to Fast Decay. 1/512 of the value becomes added to the absolute
          value of the sine wave.

        * Chopper Mode: 0 = Spread Cycle, 1 = Fast Decay.

        * Comparator Blank Time: Sets the blank time of the comparator. Available values are

          * 0 = 16 clocks,
          * 1 = 24 clocks,
          * 2 = 36 clocks and
          * 3 = 54 clocks.

          A value of 1 or 2 is recommended for most applications.

        * Fast Decay Without Comparator: If set to true the current comparator usage for termination of the
          fast decay cycle is disabled.
        """
        self.check_validity()

        slow_decay_duration = int(slow_decay_duration)
        high_velocity_fullstep = bool(high_velocity_fullstep)
        fast_decay_duration = int(fast_decay_duration)
        hysteresis_start_value = int(hysteresis_start_value)
        hysteresis_end_value = int(hysteresis_end_value)
        sine_wave_offset = int(sine_wave_offset)
        chopper_mode = int(chopper_mode)
        comparator_blank_time = int(comparator_blank_time)
        fast_decay_without_comparator = bool(fast_decay_without_comparator)

        self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_SET_SPREADCYCLE_CONFIGURATION, (slow_decay_duration, high_velocity_fullstep, fast_decay_duration, hysteresis_start_value, hysteresis_end_value, sine_wave_offset, chopper_mode, comparator_blank_time, fast_decay_without_comparator), 'B ! B B b b B B !', 0, '')

    def get_spreadcycle_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Basic Configuration`.
        """
        self.check_validity()

        return GetSpreadcycleConfiguration(*self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_SPREADCYCLE_CONFIGURATION, (), '', 18, 'B ! ! B B b b B B !'))

    def set_stealth_configuration(self, enable_stealth, offset, gradient, enable_autoscale, enable_autogradient, freewheel_mode, regulation_loop_gradient, amplitude_limit):
        r"""
        Note: If you don't know what any of this means you can very likely keep all of
        the values as default!

        Sets the configuration relevant for Stealth mode.

        * Enable Stealth: If set to true the stealth mode is enabled, if set to false the
          stealth mode is disabled, even if the speed is below the threshold set in :func:`Set Basic Configuration`.

        * Amplitude: If autoscale is disabled, the PWM amplitude is scaled by this value. If autoscale is enabled,
          this value defines the maximum PWM amplitude change per half wave.

        * Gradient: If autoscale is disabled, the PWM gradient is scaled by this value. If autoscale is enabled,
          this value defines the maximum PWM gradient. With autoscale a value above 64 is recommended,
          otherwise the regulation might not be able to measure the current.

        * Enable Autoscale: If set to true, automatic current control is used. Otherwise the user defined
          amplitude and gradient are used.

        * Enable Autogradient: TODO

        * Freewheel Mode: The freewheel mode defines the behavior in stand still if the Standstill Current
          (see :func:`Set Basic Configuration`) is set to 0.
        """
        self.check_validity()

        enable_stealth = bool(enable_stealth)
        offset = int(offset)
        gradient = int(gradient)
        enable_autoscale = bool(enable_autoscale)
        enable_autogradient = bool(enable_autogradient)
        freewheel_mode = int(freewheel_mode)
        regulation_loop_gradient = int(regulation_loop_gradient)
        amplitude_limit = int(amplitude_limit)

        self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_SET_STEALTH_CONFIGURATION, (enable_stealth, offset, gradient, enable_autoscale, enable_autogradient, freewheel_mode, regulation_loop_gradient, amplitude_limit), '! B B ! ! B B B', 0, '')

    def get_stealth_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Stealth Configuration`.
        """
        self.check_validity()

        return GetStealthConfiguration(*self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_STEALTH_CONFIGURATION, (), '', 16, '! B B ! ! B B B'))

    def set_coolstep_configuration(self, minimum_stallguard_value, maximum_stallguard_value, current_up_step_width, current_down_step_width, minimum_current, stallguard_threshold_value, stallguard_mode):
        r"""
        Note: If you don't know what any of this means you can very likely keep all of
        the values as default!

        Sets the configuration relevant for Coolstep.

        * Minimum Stallguard Value: If the Stallguard result falls below this value*32, the motor current
          is increased to reduce motor load angle. A value of 0 turns Coolstep off.

        * Maximum Stallguard Value: If the Stallguard result goes above
          (Min Stallguard Value + Max Stallguard Value + 1) * 32, the motor current is decreased to save
          energy.

        * Current Up Step Width: Sets the up step increment per Stallguard value. The value range is 0-3,
          corresponding to the increments 1, 2, 4 and 8.

        * Current Down Step Width: Sets the down step decrement per Stallguard value. The value range is 0-3,
          corresponding to the decrements 1, 2, 8 and 16.

        * Minimum Current: Sets the minimum current for Coolstep current control. You can choose between
          half and quarter of the run current.

        * Stallguard Threshold Value: Sets the level for stall output (see :func:`Get Driver Status`).
          A lower value gives a higher sensitivity. You have to find a suitable value for your
          motor by trial and error, 0 works for most motors.

        * Stallguard Mode: Set to 0 for standard resolution or 1 for filtered mode. In filtered mode the Stallguard
          signal will be updated every four full-steps.
        """
        self.check_validity()

        minimum_stallguard_value = int(minimum_stallguard_value)
        maximum_stallguard_value = int(maximum_stallguard_value)
        current_up_step_width = int(current_up_step_width)
        current_down_step_width = int(current_down_step_width)
        minimum_current = int(minimum_current)
        stallguard_threshold_value = int(stallguard_threshold_value)
        stallguard_mode = int(stallguard_mode)

        self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_SET_COOLSTEP_CONFIGURATION, (minimum_stallguard_value, maximum_stallguard_value, current_up_step_width, current_down_step_width, minimum_current, stallguard_threshold_value, stallguard_mode), 'B B B B B b B', 0, '')

    def get_coolstep_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Coolstep Configuration`.
        """
        self.check_validity()

        return GetCoolstepConfiguration(*self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_COOLSTEP_CONFIGURATION, (), '', 15, 'B B B B B b B'))

    def set_short_configuration(self, disable_short_to_voltage_protection, disable_short_to_ground_protection, short_to_voltage_level, short_to_ground_level, spike_filter_bandwidth, short_detection_delay, filter_time):
        r"""
        Note: If you don't know what any of this means you can very likely keep all of
        the values as default!

        Sets miscellaneous configuration parameters.

        * Disable Short To Ground Protection: Set to false to enable short to ground protection, otherwise
          it is disabled.

        * TODO
        """
        self.check_validity()

        disable_short_to_voltage_protection = bool(disable_short_to_voltage_protection)
        disable_short_to_ground_protection = bool(disable_short_to_ground_protection)
        short_to_voltage_level = int(short_to_voltage_level)
        short_to_ground_level = int(short_to_ground_level)
        spike_filter_bandwidth = int(spike_filter_bandwidth)
        short_detection_delay = bool(short_detection_delay)
        filter_time = int(filter_time)

        self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_SET_SHORT_CONFIGURATION, (disable_short_to_voltage_protection, disable_short_to_ground_protection, short_to_voltage_level, short_to_ground_level, spike_filter_bandwidth, short_detection_delay, filter_time), '! ! B B B ! B', 0, '')

    def get_short_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Short Configuration`.
        """
        self.check_validity()

        return GetShortConfiguration(*self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_SHORT_CONFIGURATION, (), '', 15, '! ! B B B ! B'))

    def get_driver_status(self):
        r"""
        Returns the current driver status.

        * Open Load: Indicates if an open load is present on phase A, B or both. This could mean that there is a problem
          with the wiring of the motor. False detection can occur in fast motion as well as during stand still.

        * Short To Ground: Indicates if a short to ground is present on phase A, B or both. If this is detected the driver
          automatically becomes disabled and stays disabled until it is enabled again manually.

        * Over Temperature: The over temperature indicator switches to "Warning" if the driver IC warms up. The warning flag
          is expected during long duration stepper uses. If the temperature limit is reached the indicator switches
          to "Limit". In this case the driver becomes disabled until it cools down again.

        * Motor Stalled: Is true if a motor stall was detected.

        * Actual Motor Current: Indicates the actual current control scaling as used in Coolstep mode.
          It represents a multiplier of 1/32 to 32/32 of the
          ``Motor Run Current`` as set by :func:`Set Basic Configuration`. Example: If a ``Motor Run Current``
          of 1000mA was set and the returned value is 15, the ``Actual Motor Current`` is 16/32*1000mA = 500mA.

        * Stallguard Result: Indicates the load of the motor. A lower value signals a higher load. Per trial and error
          you can find out which value corresponds to a suitable torque for the velocity used in your application.
          After that you can use this threshold value to find out if a motor stall becomes probable and react on it (e.g.
          decrease velocity).
          During stand still this value can not be used for stall detection, it shows the chopper on-time for motor coil A.

        * Stealth Voltage Amplitude: Shows the actual PWM scaling. In Stealth mode it can be used to detect motor load and
          stall if autoscale is enabled (see :func:`Set Stealth Configuration`).
        """
        self.check_validity()

        return GetDriverStatus(*self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_DRIVER_STATUS, (), '', 16, 'B B B ! B ! B B'))

    def get_input_voltage(self):
        r"""
        TBD
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_INPUT_VOLTAGE, (), '', 10, 'H')

    def get_temperature(self):
        r"""
        TBD
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_TEMPERATURE, (), '', 10, 'h')

    def set_gpio_configuration(self, debounce, stop_deceleration):
        r"""
        TBD
        """
        self.check_validity()

        debounce = int(debounce)
        stop_deceleration = int(stop_deceleration)

        self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_SET_GPIO_CONFIGURATION, (debounce, stop_deceleration), 'H i', 0, '')

    def get_gpio_configuration(self):
        r"""
        TBD
        """
        self.check_validity()

        return GetGPIOConfiguration(*self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_GPIO_CONFIGURATION, (), '', 14, 'H i'))

    def set_gpio_action(self, channel, action):
        r"""
        TBD
        """
        self.check_validity()

        channel = int(channel)
        action = int(action)

        self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_SET_GPIO_ACTION, (channel, action), 'B I', 0, '')

    def get_gpio_action(self, channel):
        r"""
        TBD
        """
        self.check_validity()

        channel = int(channel)

        return self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_GPIO_ACTION, (channel,), 'B', 12, 'I')

    def get_gpio_state(self):
        r"""
        TBD
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_GPIO_STATE, (), '', 9, '2!')

    def set_error_led_config(self, config):
        r"""
        Configures the touch LED to be either turned off, turned on, blink in
        heartbeat mode or show TBD.

        TODO:

        * one second interval blink: Input voltage too small
        * 250ms interval blink: Overtemperature warning
        * full red: motor disabled because of short to ground in phase a or b or because of overtemperature
        """
        self.check_validity()

        config = int(config)

        self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_SET_ERROR_LED_CONFIG, (config,), 'B', 0, '')

    def get_error_led_config(self):
        r"""
        Returns the LED configuration as set by :func:`Set Error LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_ERROR_LED_CONFIG, (), '', 9, 'B')

    def set_enable_led_config(self, config):
        r"""
        Configures the touch LED to be either turned off, turned on, blink in
        heartbeat mode or show TBD.
        """
        self.check_validity()

        config = int(config)

        self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_SET_ENABLE_LED_CONFIG, (config,), 'B', 0, '')

    def get_enable_led_config(self):
        r"""
        Returns the LED configuration as set by :func:`Set Enable LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_ENABLE_LED_CONFIG, (), '', 9, 'B')

    def set_steps_led_config(self, config):
        r"""
        Configures the touch LED to be either turned off, turned on, blink in
        heartbeat mode or show TBD.
        """
        self.check_validity()

        config = int(config)

        self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_SET_STEPS_LED_CONFIG, (config,), 'B', 0, '')

    def get_steps_led_config(self):
        r"""
        Returns the LED configuration as set by :func:`Set Steps LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_STEPS_LED_CONFIG, (), '', 9, 'B')

    def set_gpio_led_config(self, channel, config):
        r"""
        Configures the touch LED to be either turned off, turned on, blink in
        heartbeat mode or show TBD.
        """
        self.check_validity()

        channel = int(channel)
        config = int(config)

        self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_SET_GPIO_LED_CONFIG, (channel, config), 'B B', 0, '')

    def get_gpio_led_config(self, channel):
        r"""
        Returns the LED configuration as set by :func:`Set GPIO LED Config`
        """
        self.check_validity()

        channel = int(channel)

        return self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_GPIO_LED_CONFIG, (channel,), 'B', 9, 'B')

    def write_register(self, register, value):
        r"""
        TBD
        """
        self.check_validity()

        register = int(register)
        value = int(value)

        return self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_WRITE_REGISTER, (register, value), 'B I', 9, 'B')

    def read_register(self, register):
        r"""
        TBD
        """
        self.check_validity()

        register = int(register)

        return ReadRegister(*self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_READ_REGISTER, (register,), 'B', 13, 'B I'))

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletPerformanceStepper.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

PerformanceStepper = BrickletPerformanceStepper # for backward compatibility
