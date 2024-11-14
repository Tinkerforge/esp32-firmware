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
GetAllData = namedtuple('AllData', ['current_velocity', 'current_position', 'remaining_steps', 'stack_voltage', 'external_voltage', 'current_consumption'])
GetSPITFPBaudrateConfig = namedtuple('SPITFPBaudrateConfig', ['enable_dynamic_baudrate', 'minimum_dynamic_baudrate'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetProtocol1BrickletName = namedtuple('Protocol1BrickletName', ['protocol_version', 'firmware_version', 'name'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickStepper(Device):
    r"""
    Drives one bipolar stepper motor with up to 38V and 2.5A per phase
    """

    DEVICE_IDENTIFIER = 15
    DEVICE_DISPLAY_NAME = 'Stepper Brick'
    DEVICE_URL_PART = 'stepper' # internal

    CALLBACK_UNDER_VOLTAGE = 31
    CALLBACK_POSITION_REACHED = 32
    CALLBACK_ALL_DATA = 40
    CALLBACK_NEW_STATE = 41


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
    FUNCTION_SET_STEP_MODE = 14
    FUNCTION_GET_STEP_MODE = 15
    FUNCTION_DRIVE_FORWARD = 16
    FUNCTION_DRIVE_BACKWARD = 17
    FUNCTION_STOP = 18
    FUNCTION_GET_STACK_INPUT_VOLTAGE = 19
    FUNCTION_GET_EXTERNAL_INPUT_VOLTAGE = 20
    FUNCTION_GET_CURRENT_CONSUMPTION = 21
    FUNCTION_SET_MOTOR_CURRENT = 22
    FUNCTION_GET_MOTOR_CURRENT = 23
    FUNCTION_ENABLE = 24
    FUNCTION_DISABLE = 25
    FUNCTION_IS_ENABLED = 26
    FUNCTION_SET_DECAY = 27
    FUNCTION_GET_DECAY = 28
    FUNCTION_SET_MINIMUM_VOLTAGE = 29
    FUNCTION_GET_MINIMUM_VOLTAGE = 30
    FUNCTION_SET_SYNC_RECT = 33
    FUNCTION_IS_SYNC_RECT = 34
    FUNCTION_SET_TIME_BASE = 35
    FUNCTION_GET_TIME_BASE = 36
    FUNCTION_GET_ALL_DATA = 37
    FUNCTION_SET_ALL_DATA_PERIOD = 38
    FUNCTION_GET_ALL_DATA_PERIOD = 39
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

    STEP_MODE_FULL_STEP = 1
    STEP_MODE_HALF_STEP = 2
    STEP_MODE_QUARTER_STEP = 4
    STEP_MODE_EIGHTH_STEP = 8
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
        Device.__init__(self, uid, ipcon, BrickStepper.DEVICE_IDENTIFIER, BrickStepper.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 4)

        self.response_expected[BrickStepper.FUNCTION_SET_MAX_VELOCITY] = BrickStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickStepper.FUNCTION_GET_MAX_VELOCITY] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_GET_CURRENT_VELOCITY] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_SET_SPEED_RAMPING] = BrickStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickStepper.FUNCTION_GET_SPEED_RAMPING] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_FULL_BRAKE] = BrickStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickStepper.FUNCTION_SET_CURRENT_POSITION] = BrickStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickStepper.FUNCTION_GET_CURRENT_POSITION] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_SET_TARGET_POSITION] = BrickStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickStepper.FUNCTION_GET_TARGET_POSITION] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_SET_STEPS] = BrickStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickStepper.FUNCTION_GET_STEPS] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_GET_REMAINING_STEPS] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_SET_STEP_MODE] = BrickStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickStepper.FUNCTION_GET_STEP_MODE] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_DRIVE_FORWARD] = BrickStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickStepper.FUNCTION_DRIVE_BACKWARD] = BrickStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickStepper.FUNCTION_STOP] = BrickStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickStepper.FUNCTION_GET_STACK_INPUT_VOLTAGE] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_GET_EXTERNAL_INPUT_VOLTAGE] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_GET_CURRENT_CONSUMPTION] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_SET_MOTOR_CURRENT] = BrickStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickStepper.FUNCTION_GET_MOTOR_CURRENT] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_ENABLE] = BrickStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickStepper.FUNCTION_DISABLE] = BrickStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickStepper.FUNCTION_IS_ENABLED] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_SET_DECAY] = BrickStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickStepper.FUNCTION_GET_DECAY] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_SET_MINIMUM_VOLTAGE] = BrickStepper.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickStepper.FUNCTION_GET_MINIMUM_VOLTAGE] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_SET_SYNC_RECT] = BrickStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickStepper.FUNCTION_IS_SYNC_RECT] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_SET_TIME_BASE] = BrickStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickStepper.FUNCTION_GET_TIME_BASE] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_GET_ALL_DATA] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_SET_ALL_DATA_PERIOD] = BrickStepper.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickStepper.FUNCTION_GET_ALL_DATA_PERIOD] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_SET_SPITFP_BAUDRATE_CONFIG] = BrickStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickStepper.FUNCTION_GET_SPITFP_BAUDRATE_CONFIG] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_GET_SEND_TIMEOUT_COUNT] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_SET_SPITFP_BAUDRATE] = BrickStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickStepper.FUNCTION_GET_SPITFP_BAUDRATE] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_ENABLE_STATUS_LED] = BrickStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickStepper.FUNCTION_DISABLE_STATUS_LED] = BrickStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickStepper.FUNCTION_IS_STATUS_LED_ENABLED] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_GET_PROTOCOL1_BRICKLET_NAME] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_GET_CHIP_TEMPERATURE] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_RESET] = BrickStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickStepper.FUNCTION_WRITE_BRICKLET_PLUGIN] = BrickStepper.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickStepper.FUNCTION_READ_BRICKLET_PLUGIN] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickStepper.FUNCTION_GET_IDENTITY] = BrickStepper.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickStepper.CALLBACK_UNDER_VOLTAGE] = (10, 'H')
        self.callback_formats[BrickStepper.CALLBACK_POSITION_REACHED] = (12, 'i')
        self.callback_formats[BrickStepper.CALLBACK_ALL_DATA] = (24, 'H i i H H H')
        self.callback_formats[BrickStepper.CALLBACK_NEW_STATE] = (10, 'B B')

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

        self.ipcon.send_request(self, BrickStepper.FUNCTION_SET_MAX_VELOCITY, (velocity,), 'H', 0, '')

    def get_max_velocity(self):
        r"""
        Returns the velocity as set by :func:`Set Max Velocity`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickStepper.FUNCTION_GET_MAX_VELOCITY, (), '', 10, 'H')

    def get_current_velocity(self):
        r"""
        Returns the *current* velocity of the stepper motor.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickStepper.FUNCTION_GET_CURRENT_VELOCITY, (), '', 10, 'H')

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

        self.ipcon.send_request(self, BrickStepper.FUNCTION_SET_SPEED_RAMPING, (acceleration, deacceleration), 'H H', 0, '')

    def get_speed_ramping(self):
        r"""
        Returns the acceleration and deacceleration as set by
        :func:`Set Speed Ramping`.
        """
        self.check_validity()

        return GetSpeedRamping(*self.ipcon.send_request(self, BrickStepper.FUNCTION_GET_SPEED_RAMPING, (), '', 12, 'H H'))

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

        self.ipcon.send_request(self, BrickStepper.FUNCTION_FULL_BRAKE, (), '', 0, '')

    def set_current_position(self, position):
        r"""
        Sets the current steps of the internal step counter. This can be used to
        set the current position to 0 when some kind of starting position
        is reached (e.g. when a CNC machine reaches a corner).
        """
        self.check_validity()

        position = int(position)

        self.ipcon.send_request(self, BrickStepper.FUNCTION_SET_CURRENT_POSITION, (position,), 'i', 0, '')

    def get_current_position(self):
        r"""
        Returns the current position of the stepper motor in steps. On startup
        the position is 0. The steps are counted with all possible driving
        functions (:func:`Set Target Position`, :func:`Set Steps`, :func:`Drive Forward` or
        :func:`Drive Backward`). It also is possible to reset the steps to 0 or
        set them to any other desired value with :func:`Set Current Position`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickStepper.FUNCTION_GET_CURRENT_POSITION, (), '', 12, 'i')

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

        self.ipcon.send_request(self, BrickStepper.FUNCTION_SET_TARGET_POSITION, (position,), 'i', 0, '')

    def get_target_position(self):
        r"""
        Returns the last target position as set by :func:`Set Target Position`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickStepper.FUNCTION_GET_TARGET_POSITION, (), '', 12, 'i')

    def set_steps(self, steps):
        r"""
        Sets the number of steps the stepper motor should run. Positive values
        will drive the motor forward and negative values backward.
        The velocity, acceleration and deacceleration as set by
        :func:`Set Max Velocity` and :func:`Set Speed Ramping` will be used.
        """
        self.check_validity()

        steps = int(steps)

        self.ipcon.send_request(self, BrickStepper.FUNCTION_SET_STEPS, (steps,), 'i', 0, '')

    def get_steps(self):
        r"""
        Returns the last steps as set by :func:`Set Steps`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickStepper.FUNCTION_GET_STEPS, (), '', 12, 'i')

    def get_remaining_steps(self):
        r"""
        Returns the remaining steps of the last call of :func:`Set Steps`.
        For example, if :func:`Set Steps` is called with 2000 and
        :func:`Get Remaining Steps` is called after the motor has run for 500 steps,
        it will return 1500.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickStepper.FUNCTION_GET_REMAINING_STEPS, (), '', 12, 'i')

    def set_step_mode(self, mode):
        r"""
        Sets the step mode of the stepper motor. Possible values are:

        * Full Step = 1
        * Half Step = 2
        * Quarter Step = 4
        * Eighth Step = 8

        A higher value will increase the resolution and
        decrease the torque of the stepper motor.
        """
        self.check_validity()

        mode = int(mode)

        self.ipcon.send_request(self, BrickStepper.FUNCTION_SET_STEP_MODE, (mode,), 'B', 0, '')

    def get_step_mode(self):
        r"""
        Returns the step mode as set by :func:`Set Step Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickStepper.FUNCTION_GET_STEP_MODE, (), '', 9, 'B')

    def drive_forward(self):
        r"""
        Drives the stepper motor forward until :func:`Drive Backward` or
        :func:`Stop` is called. The velocity, acceleration and deacceleration as
        set by :func:`Set Max Velocity` and :func:`Set Speed Ramping` will be used.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickStepper.FUNCTION_DRIVE_FORWARD, (), '', 0, '')

    def drive_backward(self):
        r"""
        Drives the stepper motor backward until :func:`Drive Forward` or
        :func:`Stop` is triggered. The velocity, acceleration and deacceleration as
        set by :func:`Set Max Velocity` and :func:`Set Speed Ramping` will be used.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickStepper.FUNCTION_DRIVE_BACKWARD, (), '', 0, '')

    def stop(self):
        r"""
        Stops the stepper motor with the deacceleration as set by
        :func:`Set Speed Ramping`.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickStepper.FUNCTION_STOP, (), '', 0, '')

    def get_stack_input_voltage(self):
        r"""
        Returns the stack input voltage. The stack input voltage is the
        voltage that is supplied via the stack, i.e. it is given by a
        Step-Down or Step-Up Power Supply.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickStepper.FUNCTION_GET_STACK_INPUT_VOLTAGE, (), '', 10, 'H')

    def get_external_input_voltage(self):
        r"""
        Returns the external input voltage. The external input voltage is
        given via the black power input connector on the Stepper Brick.

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

        return self.ipcon.send_request(self, BrickStepper.FUNCTION_GET_EXTERNAL_INPUT_VOLTAGE, (), '', 10, 'H')

    def get_current_consumption(self):
        r"""
        Returns the current consumption of the motor.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickStepper.FUNCTION_GET_CURRENT_CONSUMPTION, (), '', 10, 'H')

    def set_motor_current(self, current):
        r"""
        Sets the current with which the motor will be driven.

        .. warning::
         Do not set this value above the specifications of your stepper motor.
         Otherwise it may damage your motor.
        """
        self.check_validity()

        current = int(current)

        self.ipcon.send_request(self, BrickStepper.FUNCTION_SET_MOTOR_CURRENT, (current,), 'H', 0, '')

    def get_motor_current(self):
        r"""
        Returns the current as set by :func:`Set Motor Current`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickStepper.FUNCTION_GET_MOTOR_CURRENT, (), '', 10, 'H')

    def enable(self):
        r"""
        Enables the driver chip. The driver parameters can be configured (maximum velocity,
        acceleration, etc) before it is enabled.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickStepper.FUNCTION_ENABLE, (), '', 0, '')

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

        self.ipcon.send_request(self, BrickStepper.FUNCTION_DISABLE, (), '', 0, '')

    def is_enabled(self):
        r"""
        Returns *true* if the driver chip is enabled, *false* otherwise.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickStepper.FUNCTION_IS_ENABLED, (), '', 9, '!')

    def set_decay(self, decay):
        r"""
        Sets the decay mode of the stepper motor.
        A value of 0 sets the fast decay mode, a value of
        65535 sets the slow decay mode and a value in between sets the mixed
        decay mode.

        Changing the decay mode is only possible if synchronous rectification
        is enabled (see :func:`Set Sync Rect`).

        For a good explanation of the different decay modes see
        `this <https://ebldc.com/?p=86/>`__ blog post by Avayan.

        A good decay mode is unfortunately different for every motor. The best
        way to work out a good decay mode for your stepper motor, if you can't
        measure the current with an oscilloscope, is to listen to the sound of
        the motor. If the value is too low, you often hear a high pitched
        sound and if it is too high you can often hear a humming sound.

        Generally, fast decay mode (small value) will be noisier but also
        allow higher motor speeds.

        .. note::
         There is unfortunately no formula to calculate a perfect decay
         mode for a given stepper motor. If you have problems with loud noises
         or the maximum motor speed is too slow, you should try to tinker with
         the decay value
        """
        self.check_validity()

        decay = int(decay)

        self.ipcon.send_request(self, BrickStepper.FUNCTION_SET_DECAY, (decay,), 'H', 0, '')

    def get_decay(self):
        r"""
        Returns the decay mode as set by :func:`Set Decay`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickStepper.FUNCTION_GET_DECAY, (), '', 10, 'H')

    def set_minimum_voltage(self, voltage):
        r"""
        Sets the minimum voltage, below which the :cb:`Under Voltage` callback
        is triggered. The minimum possible value that works with the Stepper Brick is 8V.
        You can use this function to detect the discharge of a battery that is used
        to drive the stepper motor. If you have a fixed power supply, you likely do
        not need this functionality.
        """
        self.check_validity()

        voltage = int(voltage)

        self.ipcon.send_request(self, BrickStepper.FUNCTION_SET_MINIMUM_VOLTAGE, (voltage,), 'H', 0, '')

    def get_minimum_voltage(self):
        r"""
        Returns the minimum voltage as set by :func:`Set Minimum Voltage`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickStepper.FUNCTION_GET_MINIMUM_VOLTAGE, (), '', 10, 'H')

    def set_sync_rect(self, sync_rect):
        r"""
        Turns synchronous rectification on or off (*true* or *false*).

        With synchronous rectification on, the decay can be changed
        (see :func:`Set Decay`). Without synchronous rectification fast
        decay is used.

        For an explanation of synchronous rectification see
        `here <https://en.wikipedia.org/wiki/Active_rectification>`__.

        .. warning::
         If you want to use high speeds (> 10000 steps/s) for a large
         stepper motor with a large inductivity we strongly
         suggest that you disable synchronous rectification. Otherwise the
         Brick may not be able to cope with the load and overheat.
        """
        self.check_validity()

        sync_rect = bool(sync_rect)

        self.ipcon.send_request(self, BrickStepper.FUNCTION_SET_SYNC_RECT, (sync_rect,), '!', 0, '')

    def is_sync_rect(self):
        r"""
        Returns *true* if synchronous rectification is enabled, *false* otherwise.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickStepper.FUNCTION_IS_SYNC_RECT, (), '', 9, '!')

    def set_time_base(self, time_base):
        r"""
        Sets the time base of the velocity and the acceleration of the stepper brick.

        For example, if you want to make one step every 1.5 seconds, you can set
        the time base to 15 and the velocity to 10. Now the velocity is
        10steps/15s = 1steps/1.5s.
        """
        self.check_validity()

        time_base = int(time_base)

        self.ipcon.send_request(self, BrickStepper.FUNCTION_SET_TIME_BASE, (time_base,), 'I', 0, '')

    def get_time_base(self):
        r"""
        Returns the time base as set by :func:`Set Time Base`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickStepper.FUNCTION_GET_TIME_BASE, (), '', 12, 'I')

    def get_all_data(self):
        r"""
        Returns the following parameters: The current velocity,
        the current position, the remaining steps, the stack voltage, the external
        voltage and the current consumption of the stepper motor.

        There is also a callback for this function, see :cb:`All Data` callback.
        """
        self.check_validity()

        return GetAllData(*self.ipcon.send_request(self, BrickStepper.FUNCTION_GET_ALL_DATA, (), '', 24, 'H i i H H H'))

    def set_all_data_period(self, period):
        r"""
        Sets the period with which the :cb:`All Data` callback is triggered
        periodically. A value of 0 turns the callback off.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickStepper.FUNCTION_SET_ALL_DATA_PERIOD, (period,), 'I', 0, '')

    def get_all_data_period(self):
        r"""
        Returns the period as set by :func:`Set All Data Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickStepper.FUNCTION_GET_ALL_DATA_PERIOD, (), '', 12, 'I')

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

        .. versionadded:: 2.3.6$nbsp;(Firmware)
        """
        self.check_validity()

        enable_dynamic_baudrate = bool(enable_dynamic_baudrate)
        minimum_dynamic_baudrate = int(minimum_dynamic_baudrate)

        self.ipcon.send_request(self, BrickStepper.FUNCTION_SET_SPITFP_BAUDRATE_CONFIG, (enable_dynamic_baudrate, minimum_dynamic_baudrate), '! I', 0, '')

    def get_spitfp_baudrate_config(self):
        r"""
        Returns the baudrate config, see :func:`Set SPITFP Baudrate Config`.

        .. versionadded:: 2.3.6$nbsp;(Firmware)
        """
        self.check_validity()

        return GetSPITFPBaudrateConfig(*self.ipcon.send_request(self, BrickStepper.FUNCTION_GET_SPITFP_BAUDRATE_CONFIG, (), '', 13, '! I'))

    def get_send_timeout_count(self, communication_method):
        r"""
        Returns the timeout count for the different communication methods.

        The methods 0-2 are available for all Bricks, 3-7 only for Master Bricks.

        This function is mostly used for debugging during development, in normal operation
        the counters should nearly always stay at 0.

        .. versionadded:: 2.3.4$nbsp;(Firmware)
        """
        self.check_validity()

        communication_method = int(communication_method)

        return self.ipcon.send_request(self, BrickStepper.FUNCTION_GET_SEND_TIMEOUT_COUNT, (communication_method,), 'B', 12, 'I')

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

        .. versionadded:: 2.3.3$nbsp;(Firmware)
        """
        self.check_validity()

        bricklet_port = create_char(bricklet_port)
        baudrate = int(baudrate)

        self.ipcon.send_request(self, BrickStepper.FUNCTION_SET_SPITFP_BAUDRATE, (bricklet_port, baudrate), 'c I', 0, '')

    def get_spitfp_baudrate(self, bricklet_port):
        r"""
        Returns the baudrate for a given Bricklet port, see :func:`Set SPITFP Baudrate`.

        .. versionadded:: 2.3.3$nbsp;(Firmware)
        """
        self.check_validity()

        bricklet_port = create_char(bricklet_port)

        return self.ipcon.send_request(self, BrickStepper.FUNCTION_GET_SPITFP_BAUDRATE, (bricklet_port,), 'c', 12, 'I')

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

        .. versionadded:: 2.3.3$nbsp;(Firmware)
        """
        self.check_validity()

        bricklet_port = create_char(bricklet_port)

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickStepper.FUNCTION_GET_SPITFP_ERROR_COUNT, (bricklet_port,), 'c', 24, 'I I I I'))

    def enable_status_led(self):
        r"""
        Enables the status LED.

        The status LED is the blue LED next to the USB connector. If enabled is is
        on and it flickers if data is transfered. If disabled it is always off.

        The default state is enabled.

        .. versionadded:: 2.3.1$nbsp;(Firmware)
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickStepper.FUNCTION_ENABLE_STATUS_LED, (), '', 0, '')

    def disable_status_led(self):
        r"""
        Disables the status LED.

        The status LED is the blue LED next to the USB connector. If enabled is is
        on and it flickers if data is transfered. If disabled it is always off.

        The default state is enabled.

        .. versionadded:: 2.3.1$nbsp;(Firmware)
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickStepper.FUNCTION_DISABLE_STATUS_LED, (), '', 0, '')

    def is_status_led_enabled(self):
        r"""
        Returns *true* if the status LED is enabled, *false* otherwise.

        .. versionadded:: 2.3.1$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickStepper.FUNCTION_IS_STATUS_LED_ENABLED, (), '', 9, '!')

    def get_protocol1_bricklet_name(self, port):
        r"""
        Returns the firmware and protocol version and the name of the Bricklet for a
        given port.

        This functions sole purpose is to allow automatic flashing of v1.x.y Bricklet
        plugins.
        """
        self.check_validity()

        port = create_char(port)

        return GetProtocol1BrickletName(*self.ipcon.send_request(self, BrickStepper.FUNCTION_GET_PROTOCOL1_BRICKLET_NAME, (port,), 'c', 52, 'B 3B 40s'))

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has an
        accuracy of ±15%. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickStepper.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Brick. Calling this function
        on a Brick inside of a stack will reset the whole stack.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickStepper.FUNCTION_RESET, (), '', 0, '')

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

        self.ipcon.send_request(self, BrickStepper.FUNCTION_WRITE_BRICKLET_PLUGIN, (port, offset, chunk), 'c B 32B', 0, '')

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

        return self.ipcon.send_request(self, BrickStepper.FUNCTION_READ_BRICKLET_PLUGIN, (port, offset), 'c B', 40, '32B')

    def get_identity(self):
        r"""
        Returns the UID, the UID where the Brick is connected to,
        the position, the hardware and firmware version as well as the
        device identifier.

        The position is the position in the stack from '0' (bottom) to '8' (top).

        The device identifier numbers can be found :ref:`here <device_identifier>`.
        |device_identifier_constant|
        """
        return GetIdentity(*self.ipcon.send_request(self, BrickStepper.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

Stepper = BrickStepper # for backward compatibility
