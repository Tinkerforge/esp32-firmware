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

GetSpeedRamping = namedtuple('SpeedRamping', ['acceleration', 'deacceleration'])
GetStepConfiguration = namedtuple('StepConfiguration', ['step_resolution', 'interpolation'])
GetBasicConfiguration = namedtuple('BasicConfiguration', ['standstill_current', 'motor_run_current', 'standstill_delay_time', 'power_down_time', 'stealth_threshold', 'coolstep_threshold', 'classic_threshold', 'high_velocity_chopper_mode'])
GetSpreadcycleConfiguration = namedtuple('SpreadcycleConfiguration', ['slow_decay_duration', 'enable_random_slow_decay', 'fast_decay_duration', 'hysteresis_start_value', 'hysteresis_end_value', 'sine_wave_offset', 'chopper_mode', 'comparator_blank_time', 'fast_decay_without_comparator'])
GetStealthConfiguration = namedtuple('StealthConfiguration', ['enable_stealth', 'amplitude', 'gradient', 'enable_autoscale', 'force_symmetric', 'freewheel_mode'])
GetCoolstepConfiguration = namedtuple('CoolstepConfiguration', ['minimum_stallguard_value', 'maximum_stallguard_value', 'current_up_step_width', 'current_down_step_width', 'minimum_current', 'stallguard_threshold_value', 'stallguard_mode'])
GetMiscConfiguration = namedtuple('MiscConfiguration', ['disable_short_to_ground_protection', 'synchronize_phase_frequency'])
GetDriverStatus = namedtuple('DriverStatus', ['open_load', 'short_to_ground', 'over_temperature', 'motor_stalled', 'actual_motor_current', 'full_step_active', 'stallguard_result', 'stealth_voltage_amplitude'])
GetAllData = namedtuple('AllData', ['current_velocity', 'current_position', 'remaining_steps', 'stack_voltage', 'external_voltage', 'current_consumption'])
GetSPITFPBaudrateConfig = namedtuple('SPITFPBaudrateConfig', ['enable_dynamic_baudrate', 'minimum_dynamic_baudrate'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetProtocol1BrickletName = namedtuple('Protocol1BrickletName', ['protocol_version', 'firmware_version', 'name'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickSilentStepper(Device):
    r"""
    Silently drives one bipolar stepper motor with up to 46V and 1.6A per phase
    """

    DEVICE_IDENTIFIER = 19
    DEVICE_DISPLAY_NAME = 'Silent Stepper Brick'
    DEVICE_URL_PART = 'silent_stepper' # internal

    CALLBACK_UNDER_VOLTAGE = 40
    CALLBACK_POSITION_REACHED = 41
    CALLBACK_ALL_DATA = 47
    CALLBACK_NEW_STATE = 48


    FUNCTION_SET_MAX_VELOCITY = 1
    FUNCTION_GET_MAX_VELOCITY = 2
    FUNCTION_GET_CURRENT_VELOCITY = 3
    FUNCTION_SET_SPEED_RAMPING = 4
    FUNCTION_GET_SPEED_RAMPING = 5
    FUNCTION_FULL_BRAKE = 6
    FUNCTION_SET_CURRENT_POSITION = 7
    FUNCTION_GET_CURRENT_POSITION = 8
    FUNCTION_SET_TARGET_POSITION = 9
    FUNCTION_GET_TARGET_POSITION = 10
    FUNCTION_SET_STEPS = 11
    FUNCTION_GET_STEPS = 12
    FUNCTION_GET_REMAINING_STEPS = 13
    FUNCTION_SET_STEP_CONFIGURATION = 14
    FUNCTION_GET_STEP_CONFIGURATION = 15
    FUNCTION_DRIVE_FORWARD = 16
    FUNCTION_DRIVE_BACKWARD = 17
    FUNCTION_STOP = 18
    FUNCTION_GET_STACK_INPUT_VOLTAGE = 19
    FUNCTION_GET_EXTERNAL_INPUT_VOLTAGE = 20
    FUNCTION_SET_MOTOR_CURRENT = 22
    FUNCTION_GET_MOTOR_CURRENT = 23
    FUNCTION_ENABLE = 24
    FUNCTION_DISABLE = 25
    FUNCTION_IS_ENABLED = 26
    FUNCTION_SET_BASIC_CONFIGURATION = 27
    FUNCTION_GET_BASIC_CONFIGURATION = 28
    FUNCTION_SET_SPREADCYCLE_CONFIGURATION = 29
    FUNCTION_GET_SPREADCYCLE_CONFIGURATION = 30
    FUNCTION_SET_STEALTH_CONFIGURATION = 31
    FUNCTION_GET_STEALTH_CONFIGURATION = 32
    FUNCTION_SET_COOLSTEP_CONFIGURATION = 33
    FUNCTION_GET_COOLSTEP_CONFIGURATION = 34
    FUNCTION_SET_MISC_CONFIGURATION = 35
    FUNCTION_GET_MISC_CONFIGURATION = 36
    FUNCTION_GET_DRIVER_STATUS = 37
    FUNCTION_SET_MINIMUM_VOLTAGE = 38
    FUNCTION_GET_MINIMUM_VOLTAGE = 39
    FUNCTION_SET_TIME_BASE = 42
    FUNCTION_GET_TIME_BASE = 43
    FUNCTION_GET_ALL_DATA = 44
    FUNCTION_SET_ALL_DATA_PERIOD = 45
    FUNCTION_GET_ALL_DATA_PERIOD = 46
    FUNCTION_SET_SPITFP_BAUDRATE_CONFIG = 231
    FUNCTION_GET_SPITFP_BAUDRATE_CONFIG = 232
    FUNCTION_GET_SEND_TIMEOUT_COUNT = 233
    FUNCTION_SET_SPITFP_BAUDRATE = 234
    FUNCTION_GET_SPITFP_BAUDRATE = 235
    FUNCTION_GET_SPITFP_ERROR_COUNT = 237
    FUNCTION_ENABLE_STATUS_LED = 238
    FUNCTION_DISABLE_STATUS_LED = 239
    FUNCTION_IS_STATUS_LED_ENABLED = 240
    FUNCTION_GET_PROTOCOL1_BRICKLET_NAME = 241
    FUNCTION_GET_CHIP_TEMPERATURE = 242
    FUNCTION_RESET = 243
    FUNCTION_WRITE_BRICKLET_PLUGIN = 246
    FUNCTION_READ_BRICKLET_PLUGIN = 247
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
    COMMUNICATION_METHOD_NONE = 0
    COMMUNICATION_METHOD_USB = 1
    COMMUNICATION_METHOD_SPI_STACK = 2
    COMMUNICATION_METHOD_CHIBI = 3
    COMMUNICATION_METHOD_RS485 = 4
    COMMUNICATION_METHOD_WIFI = 5
    COMMUNICATION_METHOD_ETHERNET = 6
    COMMUNICATION_METHOD_WIFI_V2 = 7

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickSilentStepper.DEVICE_IDENTIFIER, BrickSilentStepper.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 1)

        self.response_expected[BrickSilentStepper.FUNCTION_SET_MAX_VELOCITY] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_MAX_VELOCITY] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_CURRENT_VELOCITY] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_SET_SPEED_RAMPING] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_SPEED_RAMPING] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_FULL_BRAKE] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_SET_CURRENT_POSITION] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_CURRENT_POSITION] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_SET_TARGET_POSITION] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_TARGET_POSITION] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_SET_STEPS] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_STEPS] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_REMAINING_STEPS] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_SET_STEP_CONFIGURATION] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_STEP_CONFIGURATION] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_DRIVE_FORWARD] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_DRIVE_BACKWARD] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_STOP] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_STACK_INPUT_VOLTAGE] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_EXTERNAL_INPUT_VOLTAGE] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_SET_MOTOR_CURRENT] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_MOTOR_CURRENT] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_ENABLE] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_DISABLE] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_IS_ENABLED] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_SET_BASIC_CONFIGURATION] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_BASIC_CONFIGURATION] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_SET_SPREADCYCLE_CONFIGURATION] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_SPREADCYCLE_CONFIGURATION] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_SET_STEALTH_CONFIGURATION] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_STEALTH_CONFIGURATION] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_SET_COOLSTEP_CONFIGURATION] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_COOLSTEP_CONFIGURATION] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_SET_MISC_CONFIGURATION] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_MISC_CONFIGURATION] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_DRIVER_STATUS] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_SET_MINIMUM_VOLTAGE] = BrickSilentStepper.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_MINIMUM_VOLTAGE] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_SET_TIME_BASE] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_TIME_BASE] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_ALL_DATA] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_SET_ALL_DATA_PERIOD] = BrickSilentStepper.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_ALL_DATA_PERIOD] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_SET_SPITFP_BAUDRATE_CONFIG] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_SPITFP_BAUDRATE_CONFIG] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_SEND_TIMEOUT_COUNT] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_SET_SPITFP_BAUDRATE] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_SPITFP_BAUDRATE] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_ENABLE_STATUS_LED] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_DISABLE_STATUS_LED] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_IS_STATUS_LED_ENABLED] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_PROTOCOL1_BRICKLET_NAME] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_CHIP_TEMPERATURE] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_RESET] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_WRITE_BRICKLET_PLUGIN] = BrickSilentStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickSilentStepper.FUNCTION_READ_BRICKLET_PLUGIN] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickSilentStepper.FUNCTION_GET_IDENTITY] = BrickSilentStepper.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickSilentStepper.CALLBACK_UNDER_VOLTAGE] = (10, 'H')
        self.callback_formats[BrickSilentStepper.CALLBACK_POSITION_REACHED] = (12, 'i')
        self.callback_formats[BrickSilentStepper.CALLBACK_ALL_DATA] = (24, 'H i i H H H')
        self.callback_formats[BrickSilentStepper.CALLBACK_NEW_STATE] = (10, 'B B')

        ipcon.add_device(self)

    def set_max_velocity(self, velocity):
        r"""
        Sets the maximum velocity of the stepper motor.
        This function does *not* start the motor, it merely sets the maximum
        velocity the stepper motor is accelerated to. To get the motor running use
        either :func:`Set Target Position`, :func:`Set Steps`, :func:`Drive Forward` or
        :func:`Drive Backward`.
        """
        self.check_validity()

        velocity = int(velocity)

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_SET_MAX_VELOCITY, (velocity,), 'H', 0, '')

    def get_max_velocity(self):
        r"""
        Returns the velocity as set by :func:`Set Max Velocity`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_MAX_VELOCITY, (), '', 10, 'H')

    def get_current_velocity(self):
        r"""
        Returns the *current* velocity of the stepper motor.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_CURRENT_VELOCITY, (), '', 10, 'H')

    def set_speed_ramping(self, acceleration, deacceleration):
        r"""
        Sets the acceleration and deacceleration of the stepper motor.
        An acceleration of 1000 means, that
        every second the velocity is increased by 1000 *steps/s*.

        For example: If the current velocity is 0 and you want to accelerate to a
        velocity of 8000 *steps/s* in 10 seconds, you should set an acceleration
        of 800 *steps/s²*.

        An acceleration/deacceleration of 0 means instantaneous
        acceleration/deacceleration (not recommended)
        """
        self.check_validity()

        acceleration = int(acceleration)
        deacceleration = int(deacceleration)

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_SET_SPEED_RAMPING, (acceleration, deacceleration), 'H H', 0, '')

    def get_speed_ramping(self):
        r"""
        Returns the acceleration and deacceleration as set by
        :func:`Set Speed Ramping`.
        """
        self.check_validity()

        return GetSpeedRamping(*self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_SPEED_RAMPING, (), '', 12, 'H H'))

    def full_brake(self):
        r"""
        Executes an active full brake.

        .. warning::
         This function is for emergency purposes,
         where an immediate brake is necessary. Depending on the current velocity and
         the strength of the motor, a full brake can be quite violent.

        Call :func:`Stop` if you just want to stop the motor.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_FULL_BRAKE, (), '', 0, '')

    def set_current_position(self, position):
        r"""
        Sets the current steps of the internal step counter. This can be used to
        set the current position to 0 when some kind of starting position
        is reached (e.g. when a CNC machine reaches a corner).
        """
        self.check_validity()

        position = int(position)

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_SET_CURRENT_POSITION, (position,), 'i', 0, '')

    def get_current_position(self):
        r"""
        Returns the current position of the stepper motor in steps. On startup
        the position is 0. The steps are counted with all possible driving
        functions (:func:`Set Target Position`, :func:`Set Steps`, :func:`Drive Forward` or
        :func:`Drive Backward`). It also is possible to reset the steps to 0 or
        set them to any other desired value with :func:`Set Current Position`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_CURRENT_POSITION, (), '', 12, 'i')

    def set_target_position(self, position):
        r"""
        Sets the target position of the stepper motor in steps. For example,
        if the current position of the motor is 500 and :func:`Set Target Position` is
        called with 1000, the stepper motor will drive 500 steps forward. It will
        use the velocity, acceleration and deacceleration as set by
        :func:`Set Max Velocity` and :func:`Set Speed Ramping`.

        A call of :func:`Set Target Position` with the parameter *x* is equivalent to
        a call of :func:`Set Steps` with the parameter
        (*x* - :func:`Get Current Position`).
        """
        self.check_validity()

        position = int(position)

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_SET_TARGET_POSITION, (position,), 'i', 0, '')

    def get_target_position(self):
        r"""
        Returns the last target position as set by :func:`Set Target Position`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_TARGET_POSITION, (), '', 12, 'i')

    def set_steps(self, steps):
        r"""
        Sets the number of steps the stepper motor should run. Positive values
        will drive the motor forward and negative values backward.
        The velocity, acceleration and deacceleration as set by
        :func:`Set Max Velocity` and :func:`Set Speed Ramping` will be used.
        """
        self.check_validity()

        steps = int(steps)

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_SET_STEPS, (steps,), 'i', 0, '')

    def get_steps(self):
        r"""
        Returns the last steps as set by :func:`Set Steps`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_STEPS, (), '', 12, 'i')

    def get_remaining_steps(self):
        r"""
        Returns the remaining steps of the last call of :func:`Set Steps`.
        For example, if :func:`Set Steps` is called with 2000 and
        :func:`Get Remaining Steps` is called after the motor has run for 500 steps,
        it will return 1500.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_REMAINING_STEPS, (), '', 12, 'i')

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

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_SET_STEP_CONFIGURATION, (step_resolution, interpolation), 'B !', 0, '')

    def get_step_configuration(self):
        r"""
        Returns the step mode as set by :func:`Set Step Configuration`.
        """
        self.check_validity()

        return GetStepConfiguration(*self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_STEP_CONFIGURATION, (), '', 10, 'B !'))

    def drive_forward(self):
        r"""
        Drives the stepper motor forward until :func:`Drive Backward` or
        :func:`Stop` is called. The velocity, acceleration and deacceleration as
        set by :func:`Set Max Velocity` and :func:`Set Speed Ramping` will be used.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_DRIVE_FORWARD, (), '', 0, '')

    def drive_backward(self):
        r"""
        Drives the stepper motor backward until :func:`Drive Forward` or
        :func:`Stop` is triggered. The velocity, acceleration and deacceleration as
        set by :func:`Set Max Velocity` and :func:`Set Speed Ramping` will be used.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_DRIVE_BACKWARD, (), '', 0, '')

    def stop(self):
        r"""
        Stops the stepper motor with the deacceleration as set by
        :func:`Set Speed Ramping`.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_STOP, (), '', 0, '')

    def get_stack_input_voltage(self):
        r"""
        Returns the stack input voltage. The stack input voltage is the
        voltage that is supplied via the stack, i.e. it is given by a
        Step-Down or Step-Up Power Supply.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_STACK_INPUT_VOLTAGE, (), '', 10, 'H')

    def get_external_input_voltage(self):
        r"""
        Returns the external input voltage. The external input voltage is
        given via the black power input connector on the Silent Stepper Brick.

        If there is an external input voltage and a stack input voltage, the motor
        will be driven by the external input voltage. If there is only a stack
        voltage present, the motor will be driven by this voltage.

        .. warning::
         This means, if you have a high stack voltage and a low external voltage,
         the motor will be driven with the low external voltage. If you then remove
         the external connection, it will immediately be driven by the high
         stack voltage
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_EXTERNAL_INPUT_VOLTAGE, (), '', 10, 'H')

    def set_motor_current(self, current):
        r"""
        Sets the current with which the motor will be driven.

        .. warning::
         Do not set this value above the specifications of your stepper motor.
         Otherwise it may damage your motor.
        """
        self.check_validity()

        current = int(current)

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_SET_MOTOR_CURRENT, (current,), 'H', 0, '')

    def get_motor_current(self):
        r"""
        Returns the current as set by :func:`Set Motor Current`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_MOTOR_CURRENT, (), '', 10, 'H')

    def enable(self):
        r"""
        Enables the driver chip. The driver parameters can be configured (maximum velocity,
        acceleration, etc) before it is enabled.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_ENABLE, (), '', 0, '')

    def disable(self):
        r"""
        Disables the driver chip. The configurations are kept (maximum velocity,
        acceleration, etc) but the motor is not driven until it is enabled again.

        .. warning::
         Disabling the driver chip while the motor is still turning can damage the
         driver chip. The motor should be stopped calling :func:`Stop` function
         before disabling the motor power. The :func:`Stop` function will **not**
         wait until the motor is actually stopped. You have to explicitly wait for the
         appropriate time after calling the :func:`Stop` function before calling
         the :func:`Disable` function.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_DISABLE, (), '', 0, '')

    def is_enabled(self):
        r"""
        Returns *true* if the driver chip is enabled, *false* otherwise.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_IS_ENABLED, (), '', 9, '!')

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

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_SET_BASIC_CONFIGURATION, (standstill_current, motor_run_current, standstill_delay_time, power_down_time, stealth_threshold, coolstep_threshold, classic_threshold, high_velocity_chopper_mode), 'H H H H H H H !', 0, '')

    def get_basic_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Basic Configuration`.
        """
        self.check_validity()

        return GetBasicConfiguration(*self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_BASIC_CONFIGURATION, (), '', 23, 'H H H H H H H !'))

    def set_spreadcycle_configuration(self, slow_decay_duration, enable_random_slow_decay, fast_decay_duration, hysteresis_start_value, hysteresis_end_value, sine_wave_offset, chopper_mode, comparator_blank_time, fast_decay_without_comparator):
        r"""
        Note: If you don't know what any of this means you can very likely keep all of
        the values as default!

        Sets the Spreadcycle configuration parameters. Spreadcycle is a chopper algorithm which actively
        controls the motor current flow. More information can be found in the TMC2130 datasheet on page
        47 (7 spreadCycle and Classic Chopper).

        * Slow Decay Duration: Controls duration of off time setting of slow decay phase.
          0 = driver disabled, all bridges off. Use 1 only with Comparator Blank time >= 2.

        * Enable Random Slow Decay: Set to false to fix chopper off time as set by Slow Decay Duration.
          If you set it to true, Decay Duration is randomly modulated.

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
        enable_random_slow_decay = bool(enable_random_slow_decay)
        fast_decay_duration = int(fast_decay_duration)
        hysteresis_start_value = int(hysteresis_start_value)
        hysteresis_end_value = int(hysteresis_end_value)
        sine_wave_offset = int(sine_wave_offset)
        chopper_mode = int(chopper_mode)
        comparator_blank_time = int(comparator_blank_time)
        fast_decay_without_comparator = bool(fast_decay_without_comparator)

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_SET_SPREADCYCLE_CONFIGURATION, (slow_decay_duration, enable_random_slow_decay, fast_decay_duration, hysteresis_start_value, hysteresis_end_value, sine_wave_offset, chopper_mode, comparator_blank_time, fast_decay_without_comparator), 'B ! B B b b B B !', 0, '')

    def get_spreadcycle_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Basic Configuration`.
        """
        self.check_validity()

        return GetSpreadcycleConfiguration(*self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_SPREADCYCLE_CONFIGURATION, (), '', 17, 'B ! B B b b B B !'))

    def set_stealth_configuration(self, enable_stealth, amplitude, gradient, enable_autoscale, force_symmetric, freewheel_mode):
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

        * Force Symmetric: If true, A symmetric PWM cycle is enforced. Otherwise the PWM value may change within each
          PWM cycle.

        * Freewheel Mode: The freewheel mode defines the behavior in stand still if the Standstill Current
          (see :func:`Set Basic Configuration`) is set to 0.
        """
        self.check_validity()

        enable_stealth = bool(enable_stealth)
        amplitude = int(amplitude)
        gradient = int(gradient)
        enable_autoscale = bool(enable_autoscale)
        force_symmetric = bool(force_symmetric)
        freewheel_mode = int(freewheel_mode)

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_SET_STEALTH_CONFIGURATION, (enable_stealth, amplitude, gradient, enable_autoscale, force_symmetric, freewheel_mode), '! B B ! ! B', 0, '')

    def get_stealth_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Stealth Configuration`.
        """
        self.check_validity()

        return GetStealthConfiguration(*self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_STEALTH_CONFIGURATION, (), '', 14, '! B B ! ! B'))

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

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_SET_COOLSTEP_CONFIGURATION, (minimum_stallguard_value, maximum_stallguard_value, current_up_step_width, current_down_step_width, minimum_current, stallguard_threshold_value, stallguard_mode), 'B B B B B b B', 0, '')

    def get_coolstep_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Coolstep Configuration`.
        """
        self.check_validity()

        return GetCoolstepConfiguration(*self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_COOLSTEP_CONFIGURATION, (), '', 15, 'B B B B B b B'))

    def set_misc_configuration(self, disable_short_to_ground_protection, synchronize_phase_frequency):
        r"""
        Note: If you don't know what any of this means you can very likely keep all of
        the values as default!

        Sets miscellaneous configuration parameters.

        * Disable Short To Ground Protection: Set to false to enable short to ground protection, otherwise
          it is disabled.

        * Synchronize Phase Frequency: With this parameter you can synchronize the chopper for both phases
          of a two phase motor to avoid the occurrence of a beat. The value range is 0-15. If set to 0,
          the synchronization is turned off. Otherwise the synchronization is done through the formula
          f_sync = f_clk/(value*64). In Classic Mode the synchronization is automatically switched off.
          f_clk is 12.8MHz.
        """
        self.check_validity()

        disable_short_to_ground_protection = bool(disable_short_to_ground_protection)
        synchronize_phase_frequency = int(synchronize_phase_frequency)

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_SET_MISC_CONFIGURATION, (disable_short_to_ground_protection, synchronize_phase_frequency), '! B', 0, '')

    def get_misc_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Misc Configuration`.
        """
        self.check_validity()

        return GetMiscConfiguration(*self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_MISC_CONFIGURATION, (), '', 10, '! B'))

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

        return GetDriverStatus(*self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_DRIVER_STATUS, (), '', 16, 'B B B ! B ! B B'))

    def set_minimum_voltage(self, voltage):
        r"""
        Sets the minimum voltage, below which the :cb:`Under Voltage` callback
        is triggered. The minimum possible value that works with the Silent Stepper
        Brick is 8V.
        You can use this function to detect the discharge of a battery that is used
        to drive the stepper motor. If you have a fixed power supply, you likely do
        not need this functionality.
        """
        self.check_validity()

        voltage = int(voltage)

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_SET_MINIMUM_VOLTAGE, (voltage,), 'H', 0, '')

    def get_minimum_voltage(self):
        r"""
        Returns the minimum voltage as set by :func:`Set Minimum Voltage`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_MINIMUM_VOLTAGE, (), '', 10, 'H')

    def set_time_base(self, time_base):
        r"""
        Sets the time base of the velocity and the acceleration of the Silent Stepper
        Brick.

        For example, if you want to make one step every 1.5 seconds, you can set
        the time base to 15 and the velocity to 10. Now the velocity is
        10steps/15s = 1steps/1.5s.
        """
        self.check_validity()

        time_base = int(time_base)

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_SET_TIME_BASE, (time_base,), 'I', 0, '')

    def get_time_base(self):
        r"""
        Returns the time base as set by :func:`Set Time Base`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_TIME_BASE, (), '', 12, 'I')

    def get_all_data(self):
        r"""
        Returns the following parameters: The current velocity,
        the current position, the remaining steps, the stack voltage, the external
        voltage and the current consumption of the stepper motor.

        The current consumption is calculated by multiplying the ``Actual Motor Current``
        value (see :func:`Set Basic Configuration`) with the ``Motor Run Current``
        (see :func:`Get Driver Status`). This is an internal calculation of the
        driver, not an independent external measurement.

        The current consumption calculation was broken up to firmware 2.0.1, it is fixed
        since firmware 2.0.2.

        There is also a callback for this function, see :cb:`All Data` callback.
        """
        self.check_validity()

        return GetAllData(*self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_ALL_DATA, (), '', 24, 'H i i H H H'))

    def set_all_data_period(self, period):
        r"""
        Sets the period with which the :cb:`All Data` callback is triggered
        periodically. A value of 0 turns the callback off.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_SET_ALL_DATA_PERIOD, (period,), 'I', 0, '')

    def get_all_data_period(self):
        r"""
        Returns the period as set by :func:`Set All Data Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_ALL_DATA_PERIOD, (), '', 12, 'I')

    def set_spitfp_baudrate_config(self, enable_dynamic_baudrate, minimum_dynamic_baudrate):
        r"""
        The SPITF protocol can be used with a dynamic baudrate. If the dynamic baudrate is
        enabled, the Brick will try to adapt the baudrate for the communication
        between Bricks and Bricklets according to the amount of data that is transferred.

        The baudrate will be increased exponentially if lots of data is sent/received and
        decreased linearly if little data is sent/received.

        This lowers the baudrate in applications where little data is transferred (e.g.
        a weather station) and increases the robustness. If there is lots of data to transfer
        (e.g. Thermal Imaging Bricklet) it automatically increases the baudrate as needed.

        In cases where some data has to transferred as fast as possible every few seconds
        (e.g. RS485 Bricklet with a high baudrate but small payload) you may want to turn
        the dynamic baudrate off to get the highest possible performance.

        The maximum value of the baudrate can be set per port with the function
        :func:`Set SPITFP Baudrate`. If the dynamic baudrate is disabled, the baudrate
        as set by :func:`Set SPITFP Baudrate` will be used statically.

        .. versionadded:: 2.0.4$nbsp;(Firmware)
        """
        self.check_validity()

        enable_dynamic_baudrate = bool(enable_dynamic_baudrate)
        minimum_dynamic_baudrate = int(minimum_dynamic_baudrate)

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_SET_SPITFP_BAUDRATE_CONFIG, (enable_dynamic_baudrate, minimum_dynamic_baudrate), '! I', 0, '')

    def get_spitfp_baudrate_config(self):
        r"""
        Returns the baudrate config, see :func:`Set SPITFP Baudrate Config`.

        .. versionadded:: 2.0.4$nbsp;(Firmware)
        """
        self.check_validity()

        return GetSPITFPBaudrateConfig(*self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_SPITFP_BAUDRATE_CONFIG, (), '', 13, '! I'))

    def get_send_timeout_count(self, communication_method):
        r"""
        Returns the timeout count for the different communication methods.

        The methods 0-2 are available for all Bricks, 3-7 only for Master Bricks.

        This function is mostly used for debugging during development, in normal operation
        the counters should nearly always stay at 0.
        """
        self.check_validity()

        communication_method = int(communication_method)

        return self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_SEND_TIMEOUT_COUNT, (communication_method,), 'B', 12, 'I')

    def set_spitfp_baudrate(self, bricklet_port, baudrate):
        r"""
        Sets the baudrate for a specific Bricklet port.

        If you want to increase the throughput of Bricklets you can increase
        the baudrate. If you get a high error count because of high
        interference (see :func:`Get SPITFP Error Count`) you can decrease the
        baudrate.

        If the dynamic baudrate feature is enabled, the baudrate set by this
        function corresponds to the maximum baudrate (see :func:`Set SPITFP Baudrate Config`).

        Regulatory testing is done with the default baudrate. If CE compatibility
        or similar is necessary in your applications we recommend to not change
        the baudrate.
        """
        self.check_validity()

        bricklet_port = create_char(bricklet_port)
        baudrate = int(baudrate)

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_SET_SPITFP_BAUDRATE, (bricklet_port, baudrate), 'c I', 0, '')

    def get_spitfp_baudrate(self, bricklet_port):
        r"""
        Returns the baudrate for a given Bricklet port, see :func:`Set SPITFP Baudrate`.
        """
        self.check_validity()

        bricklet_port = create_char(bricklet_port)

        return self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_SPITFP_BAUDRATE, (bricklet_port,), 'c', 12, 'I')

    def get_spitfp_error_count(self, bricklet_port):
        r"""
        Returns the error count for the communication between Brick and Bricklet.

        The errors are divided into

        * ACK checksum errors,
        * message checksum errors,
        * framing errors and
        * overflow errors.

        The errors counts are for errors that occur on the Brick side. All
        Bricklets have a similar function that returns the errors on the Bricklet side.
        """
        self.check_validity()

        bricklet_port = create_char(bricklet_port)

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_SPITFP_ERROR_COUNT, (bricklet_port,), 'c', 24, 'I I I I'))

    def enable_status_led(self):
        r"""
        Enables the status LED.

        The status LED is the blue LED next to the USB connector. If enabled is is
        on and it flickers if data is transfered. If disabled it is always off.

        The default state is enabled.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_ENABLE_STATUS_LED, (), '', 0, '')

    def disable_status_led(self):
        r"""
        Disables the status LED.

        The status LED is the blue LED next to the USB connector. If enabled is is
        on and it flickers if data is transfered. If disabled it is always off.

        The default state is enabled.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_DISABLE_STATUS_LED, (), '', 0, '')

    def is_status_led_enabled(self):
        r"""
        Returns *true* if the status LED is enabled, *false* otherwise.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_IS_STATUS_LED_ENABLED, (), '', 9, '!')

    def get_protocol1_bricklet_name(self, port):
        r"""
        Returns the firmware and protocol version and the name of the Bricklet for a
        given port.

        This functions sole purpose is to allow automatic flashing of v1.x.y Bricklet
        plugins.
        """
        self.check_validity()

        port = create_char(port)

        return GetProtocol1BrickletName(*self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_PROTOCOL1_BRICKLET_NAME, (port,), 'c', 52, 'B 3B 40s'))

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has an
        accuracy of ±15%. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Brick. Calling this function
        on a Brick inside of a stack will reset the whole stack.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_RESET, (), '', 0, '')

    def write_bricklet_plugin(self, port, offset, chunk):
        r"""
        Writes 32 bytes of firmware to the bricklet attached at the given port.
        The bytes are written to the position offset * 32.

        This function is used by Brick Viewer during flashing. It should not be
        necessary to call it in a normal user program.
        """
        self.check_validity()

        port = create_char(port)
        offset = int(offset)
        chunk = list(map(int, chunk))

        self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_WRITE_BRICKLET_PLUGIN, (port, offset, chunk), 'c B 32B', 0, '')

    def read_bricklet_plugin(self, port, offset):
        r"""
        Reads 32 bytes of firmware from the bricklet attached at the given port.
        The bytes are read starting at the position offset * 32.

        This function is used by Brick Viewer during flashing. It should not be
        necessary to call it in a normal user program.
        """
        self.check_validity()

        port = create_char(port)
        offset = int(offset)

        return self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_READ_BRICKLET_PLUGIN, (port, offset), 'c B', 40, '32B')

    def get_identity(self):
        r"""
        Returns the UID, the UID where the Brick is connected to,
        the position, the hardware and firmware version as well as the
        device identifier.

        The position is the position in the stack from '0' (bottom) to '8' (top).

        The device identifier numbers can be found :ref:`here <device_identifier>`.
        |device_identifier_constant|
        """
        return GetIdentity(*self.ipcon.send_request(self, BrickSilentStepper.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

SilentStepper = BrickSilentStepper # for backward compatibility
