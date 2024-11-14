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

GetSPITFPBaudrateConfig = namedtuple('SPITFPBaudrateConfig', ['enable_dynamic_baudrate', 'minimum_dynamic_baudrate'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetProtocol1BrickletName = namedtuple('Protocol1BrickletName', ['protocol_version', 'firmware_version', 'name'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickDC(Device):
    r"""
    Drives one brushed DC motor with up to 28V and 5A (peak)
    """

    DEVICE_IDENTIFIER = 11
    DEVICE_DISPLAY_NAME = 'DC Brick'
    DEVICE_URL_PART = 'dc' # internal

    CALLBACK_UNDER_VOLTAGE = 21
    CALLBACK_EMERGENCY_SHUTDOWN = 22
    CALLBACK_VELOCITY_REACHED = 23
    CALLBACK_CURRENT_VELOCITY = 24


    FUNCTION_SET_VELOCITY = 1
    FUNCTION_GET_VELOCITY = 2
    FUNCTION_GET_CURRENT_VELOCITY = 3
    FUNCTION_SET_ACCELERATION = 4
    FUNCTION_GET_ACCELERATION = 5
    FUNCTION_SET_PWM_FREQUENCY = 6
    FUNCTION_GET_PWM_FREQUENCY = 7
    FUNCTION_FULL_BRAKE = 8
    FUNCTION_GET_STACK_INPUT_VOLTAGE = 9
    FUNCTION_GET_EXTERNAL_INPUT_VOLTAGE = 10
    FUNCTION_GET_CURRENT_CONSUMPTION = 11
    FUNCTION_ENABLE = 12
    FUNCTION_DISABLE = 13
    FUNCTION_IS_ENABLED = 14
    FUNCTION_SET_MINIMUM_VOLTAGE = 15
    FUNCTION_GET_MINIMUM_VOLTAGE = 16
    FUNCTION_SET_DRIVE_MODE = 17
    FUNCTION_GET_DRIVE_MODE = 18
    FUNCTION_SET_CURRENT_VELOCITY_PERIOD = 19
    FUNCTION_GET_CURRENT_VELOCITY_PERIOD = 20
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

    DRIVE_MODE_DRIVE_BRAKE = 0
    DRIVE_MODE_DRIVE_COAST = 1
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
        Device.__init__(self, uid, ipcon, BrickDC.DEVICE_IDENTIFIER, BrickDC.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 3)

        self.response_expected[BrickDC.FUNCTION_SET_VELOCITY] = BrickDC.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickDC.FUNCTION_GET_VELOCITY] = BrickDC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickDC.FUNCTION_GET_CURRENT_VELOCITY] = BrickDC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickDC.FUNCTION_SET_ACCELERATION] = BrickDC.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickDC.FUNCTION_GET_ACCELERATION] = BrickDC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickDC.FUNCTION_SET_PWM_FREQUENCY] = BrickDC.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickDC.FUNCTION_GET_PWM_FREQUENCY] = BrickDC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickDC.FUNCTION_FULL_BRAKE] = BrickDC.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickDC.FUNCTION_GET_STACK_INPUT_VOLTAGE] = BrickDC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickDC.FUNCTION_GET_EXTERNAL_INPUT_VOLTAGE] = BrickDC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickDC.FUNCTION_GET_CURRENT_CONSUMPTION] = BrickDC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickDC.FUNCTION_ENABLE] = BrickDC.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickDC.FUNCTION_DISABLE] = BrickDC.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickDC.FUNCTION_IS_ENABLED] = BrickDC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickDC.FUNCTION_SET_MINIMUM_VOLTAGE] = BrickDC.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickDC.FUNCTION_GET_MINIMUM_VOLTAGE] = BrickDC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickDC.FUNCTION_SET_DRIVE_MODE] = BrickDC.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickDC.FUNCTION_GET_DRIVE_MODE] = BrickDC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickDC.FUNCTION_SET_CURRENT_VELOCITY_PERIOD] = BrickDC.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickDC.FUNCTION_GET_CURRENT_VELOCITY_PERIOD] = BrickDC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickDC.FUNCTION_SET_SPITFP_BAUDRATE_CONFIG] = BrickDC.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickDC.FUNCTION_GET_SPITFP_BAUDRATE_CONFIG] = BrickDC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickDC.FUNCTION_GET_SEND_TIMEOUT_COUNT] = BrickDC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickDC.FUNCTION_SET_SPITFP_BAUDRATE] = BrickDC.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickDC.FUNCTION_GET_SPITFP_BAUDRATE] = BrickDC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickDC.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickDC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickDC.FUNCTION_ENABLE_STATUS_LED] = BrickDC.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickDC.FUNCTION_DISABLE_STATUS_LED] = BrickDC.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickDC.FUNCTION_IS_STATUS_LED_ENABLED] = BrickDC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickDC.FUNCTION_GET_PROTOCOL1_BRICKLET_NAME] = BrickDC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickDC.FUNCTION_GET_CHIP_TEMPERATURE] = BrickDC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickDC.FUNCTION_RESET] = BrickDC.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickDC.FUNCTION_WRITE_BRICKLET_PLUGIN] = BrickDC.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickDC.FUNCTION_READ_BRICKLET_PLUGIN] = BrickDC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickDC.FUNCTION_GET_IDENTITY] = BrickDC.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickDC.CALLBACK_UNDER_VOLTAGE] = (10, 'H')
        self.callback_formats[BrickDC.CALLBACK_EMERGENCY_SHUTDOWN] = (8, '')
        self.callback_formats[BrickDC.CALLBACK_VELOCITY_REACHED] = (10, 'h')
        self.callback_formats[BrickDC.CALLBACK_CURRENT_VELOCITY] = (10, 'h')

        ipcon.add_device(self)

    def set_velocity(self, velocity):
        r"""
        Sets the velocity of the motor. Whereas -32767 is full speed backward,
        0 is stop and 32767 is full speed forward. Depending on the
        acceleration (see :func:`Set Acceleration`), the motor is not immediately
        brought to the velocity but smoothly accelerated.

        The velocity describes the duty cycle of the PWM with which the motor is
        controlled, e.g. a velocity of 3277 sets a PWM with a 10% duty cycle.
        You can not only control the duty cycle of the PWM but also the frequency,
        see :func:`Set PWM Frequency`.
        """
        self.check_validity()

        velocity = int(velocity)

        self.ipcon.send_request(self, BrickDC.FUNCTION_SET_VELOCITY, (velocity,), 'h', 0, '')

    def get_velocity(self):
        r"""
        Returns the velocity as set by :func:`Set Velocity`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickDC.FUNCTION_GET_VELOCITY, (), '', 10, 'h')

    def get_current_velocity(self):
        r"""
        Returns the *current* velocity of the motor. This value is different
        from :func:`Get Velocity` whenever the motor is currently accelerating
        to a goal set by :func:`Set Velocity`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickDC.FUNCTION_GET_CURRENT_VELOCITY, (), '', 10, 'h')

    def set_acceleration(self, acceleration):
        r"""
        Sets the acceleration of the motor. It is given in *velocity/s*. An
        acceleration of 10000 means, that every second the velocity is increased
        by 10000 (or about 30% duty cycle).

        For example: If the current velocity is 0 and you want to accelerate to a
        velocity of 16000 (about 50% duty cycle) in 10 seconds, you should set
        an acceleration of 1600.

        If acceleration is set to 0, there is no speed ramping, i.e. a new velocity
        is immediately given to the motor.
        """
        self.check_validity()

        acceleration = int(acceleration)

        self.ipcon.send_request(self, BrickDC.FUNCTION_SET_ACCELERATION, (acceleration,), 'H', 0, '')

    def get_acceleration(self):
        r"""
        Returns the acceleration as set by :func:`Set Acceleration`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickDC.FUNCTION_GET_ACCELERATION, (), '', 10, 'H')

    def set_pwm_frequency(self, frequency):
        r"""
        Sets the frequency of the PWM with which the motor is driven.
        Often a high frequency
        is less noisy and the motor runs smoother. However, with a low frequency
        there are less switches and therefore fewer switching losses. Also with
        most motors lower frequencies enable higher torque.

        If you have no idea what all this means, just ignore this function and use
        the default frequency, it will very likely work fine.
        """
        self.check_validity()

        frequency = int(frequency)

        self.ipcon.send_request(self, BrickDC.FUNCTION_SET_PWM_FREQUENCY, (frequency,), 'H', 0, '')

    def get_pwm_frequency(self):
        r"""
        Returns the PWM frequency as set by :func:`Set PWM Frequency`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickDC.FUNCTION_GET_PWM_FREQUENCY, (), '', 10, 'H')

    def full_brake(self):
        r"""
        Executes an active full brake.

        .. warning::
         This function is for emergency purposes,
         where an immediate brake is necessary. Depending on the current velocity and
         the strength of the motor, a full brake can be quite violent.

        Call :func:`Set Velocity` with 0 if you just want to stop the motor.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickDC.FUNCTION_FULL_BRAKE, (), '', 0, '')

    def get_stack_input_voltage(self):
        r"""
        Returns the stack input voltage. The stack input voltage is the
        voltage that is supplied via the stack, i.e. it is given by a
        Step-Down or Step-Up Power Supply.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickDC.FUNCTION_GET_STACK_INPUT_VOLTAGE, (), '', 10, 'H')

    def get_external_input_voltage(self):
        r"""
        Returns the external input voltage. The external input voltage is
        given via the black power input connector on the DC Brick.

        If there is an external input voltage and a stack input voltage, the motor
        will be driven by the external input voltage. If there is only a stack
        voltage present, the motor will be driven by this voltage.

        .. warning::
         This means, if you have a high stack voltage and a low external voltage,
         the motor will be driven with the low external voltage. If you then remove
         the external connection, it will immediately be driven by the high
         stack voltage.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickDC.FUNCTION_GET_EXTERNAL_INPUT_VOLTAGE, (), '', 10, 'H')

    def get_current_consumption(self):
        r"""
        Returns the current consumption of the motor.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickDC.FUNCTION_GET_CURRENT_CONSUMPTION, (), '', 10, 'H')

    def enable(self):
        r"""
        Enables the driver chip. The driver parameters can be configured (velocity,
        acceleration, etc) before it is enabled.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickDC.FUNCTION_ENABLE, (), '', 0, '')

    def disable(self):
        r"""
        Disables the driver chip. The configurations are kept (velocity,
        acceleration, etc) but the motor is not driven until it is enabled again.

        .. warning::
         Disabling the driver chip while the motor is still turning can damage the
         driver chip. The motor should be stopped calling :func:`Set Velocity` with 0
         before disabling the motor power. The :func:`Set Velocity` function will **not**
         wait until the motor is actually stopped. You have to explicitly wait for the
         appropriate time after calling the :func:`Set Velocity` function before calling
         the :func:`Disable` function.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickDC.FUNCTION_DISABLE, (), '', 0, '')

    def is_enabled(self):
        r"""
        Returns *true* if the driver chip is enabled, *false* otherwise.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickDC.FUNCTION_IS_ENABLED, (), '', 9, '!')

    def set_minimum_voltage(self, voltage):
        r"""
        Sets the minimum voltage, below which the :cb:`Under Voltage` callback
        is triggered. The minimum possible value that works with the DC Brick is 6V.
        You can use this function to detect the discharge of a battery that is used
        to drive the motor. If you have a fixed power supply, you likely do not need
        this functionality.
        """
        self.check_validity()

        voltage = int(voltage)

        self.ipcon.send_request(self, BrickDC.FUNCTION_SET_MINIMUM_VOLTAGE, (voltage,), 'H', 0, '')

    def get_minimum_voltage(self):
        r"""
        Returns the minimum voltage as set by :func:`Set Minimum Voltage`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickDC.FUNCTION_GET_MINIMUM_VOLTAGE, (), '', 10, 'H')

    def set_drive_mode(self, mode):
        r"""
        Sets the drive mode. Possible modes are:

        * 0 = Drive/Brake
        * 1 = Drive/Coast

        These modes are different kinds of motor controls.

        In Drive/Brake mode, the motor is always either driving or braking. There
        is no freewheeling. Advantages are: A more linear correlation between
        PWM and velocity, more exact accelerations and the possibility to drive
        with slower velocities.

        In Drive/Coast mode, the motor is always either driving or freewheeling.
        Advantages are: Less current consumption and less demands on the motor and
        driver chip.
        """
        self.check_validity()

        mode = int(mode)

        self.ipcon.send_request(self, BrickDC.FUNCTION_SET_DRIVE_MODE, (mode,), 'B', 0, '')

    def get_drive_mode(self):
        r"""
        Returns the drive mode, as set by :func:`Set Drive Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickDC.FUNCTION_GET_DRIVE_MODE, (), '', 9, 'B')

    def set_current_velocity_period(self, period):
        r"""
        Sets a period with which the :cb:`Current Velocity` callback is triggered.
        A period of 0 turns the callback off.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickDC.FUNCTION_SET_CURRENT_VELOCITY_PERIOD, (period,), 'H', 0, '')

    def get_current_velocity_period(self):
        r"""
        Returns the period as set by :func:`Set Current Velocity Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickDC.FUNCTION_GET_CURRENT_VELOCITY_PERIOD, (), '', 10, 'H')

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

        .. versionadded:: 2.3.5$nbsp;(Firmware)
        """
        self.check_validity()

        enable_dynamic_baudrate = bool(enable_dynamic_baudrate)
        minimum_dynamic_baudrate = int(minimum_dynamic_baudrate)

        self.ipcon.send_request(self, BrickDC.FUNCTION_SET_SPITFP_BAUDRATE_CONFIG, (enable_dynamic_baudrate, minimum_dynamic_baudrate), '! I', 0, '')

    def get_spitfp_baudrate_config(self):
        r"""
        Returns the baudrate config, see :func:`Set SPITFP Baudrate Config`.

        .. versionadded:: 2.3.5$nbsp;(Firmware)
        """
        self.check_validity()

        return GetSPITFPBaudrateConfig(*self.ipcon.send_request(self, BrickDC.FUNCTION_GET_SPITFP_BAUDRATE_CONFIG, (), '', 13, '! I'))

    def get_send_timeout_count(self, communication_method):
        r"""
        Returns the timeout count for the different communication methods.

        The methods 0-2 are available for all Bricks, 3-7 only for Master Bricks.

        This function is mostly used for debugging during development, in normal operation
        the counters should nearly always stay at 0.

        .. versionadded:: 2.3.3$nbsp;(Firmware)
        """
        self.check_validity()

        communication_method = int(communication_method)

        return self.ipcon.send_request(self, BrickDC.FUNCTION_GET_SEND_TIMEOUT_COUNT, (communication_method,), 'B', 12, 'I')

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

        self.ipcon.send_request(self, BrickDC.FUNCTION_SET_SPITFP_BAUDRATE, (bricklet_port, baudrate), 'c I', 0, '')

    def get_spitfp_baudrate(self, bricklet_port):
        r"""
        Returns the baudrate for a given Bricklet port, see :func:`Set SPITFP Baudrate`.

        .. versionadded:: 2.3.3$nbsp;(Firmware)
        """
        self.check_validity()

        bricklet_port = create_char(bricklet_port)

        return self.ipcon.send_request(self, BrickDC.FUNCTION_GET_SPITFP_BAUDRATE, (bricklet_port,), 'c', 12, 'I')

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickDC.FUNCTION_GET_SPITFP_ERROR_COUNT, (bricklet_port,), 'c', 24, 'I I I I'))

    def enable_status_led(self):
        r"""
        Enables the status LED.

        The status LED is the blue LED next to the USB connector. If enabled is is
        on and it flickers if data is transfered. If disabled it is always off.

        The default state is enabled.

        .. versionadded:: 2.3.1$nbsp;(Firmware)
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickDC.FUNCTION_ENABLE_STATUS_LED, (), '', 0, '')

    def disable_status_led(self):
        r"""
        Disables the status LED.

        The status LED is the blue LED next to the USB connector. If enabled is is
        on and it flickers if data is transfered. If disabled it is always off.

        The default state is enabled.

        .. versionadded:: 2.3.1$nbsp;(Firmware)
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickDC.FUNCTION_DISABLE_STATUS_LED, (), '', 0, '')

    def is_status_led_enabled(self):
        r"""
        Returns *true* if the status LED is enabled, *false* otherwise.

        .. versionadded:: 2.3.1$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickDC.FUNCTION_IS_STATUS_LED_ENABLED, (), '', 9, '!')

    def get_protocol1_bricklet_name(self, port):
        r"""
        Returns the firmware and protocol version and the name of the Bricklet for a
        given port.

        This functions sole purpose is to allow automatic flashing of v1.x.y Bricklet
        plugins.
        """
        self.check_validity()

        port = create_char(port)

        return GetProtocol1BrickletName(*self.ipcon.send_request(self, BrickDC.FUNCTION_GET_PROTOCOL1_BRICKLET_NAME, (port,), 'c', 52, 'B 3B 40s'))

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has an
        accuracy of ±15%. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickDC.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Brick. Calling this function
        on a Brick inside of a stack will reset the whole stack.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickDC.FUNCTION_RESET, (), '', 0, '')

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

        self.ipcon.send_request(self, BrickDC.FUNCTION_WRITE_BRICKLET_PLUGIN, (port, offset, chunk), 'c B 32B', 0, '')

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

        return self.ipcon.send_request(self, BrickDC.FUNCTION_READ_BRICKLET_PLUGIN, (port, offset), 'c B', 40, '32B')

    def get_identity(self):
        r"""
        Returns the UID, the UID where the Brick is connected to,
        the position, the hardware and firmware version as well as the
        device identifier.

        The position is the position in the stack from '0' (bottom) to '8' (top).

        The device identifier numbers can be found :ref:`here <device_identifier>`.
        |device_identifier_constant|
        """
        return GetIdentity(*self.ipcon.send_request(self, BrickDC.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

DC = BrickDC # for backward compatibility
