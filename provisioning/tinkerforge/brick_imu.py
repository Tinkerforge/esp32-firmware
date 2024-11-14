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

GetAcceleration = namedtuple('Acceleration', ['x', 'y', 'z'])
GetMagneticField = namedtuple('MagneticField', ['x', 'y', 'z'])
GetAngularVelocity = namedtuple('AngularVelocity', ['x', 'y', 'z'])
GetAllData = namedtuple('AllData', ['acc_x', 'acc_y', 'acc_z', 'mag_x', 'mag_y', 'mag_z', 'ang_x', 'ang_y', 'ang_z', 'temperature'])
GetOrientation = namedtuple('Orientation', ['roll', 'pitch', 'yaw'])
GetQuaternion = namedtuple('Quaternion', ['x', 'y', 'z', 'w'])
GetSPITFPBaudrateConfig = namedtuple('SPITFPBaudrateConfig', ['enable_dynamic_baudrate', 'minimum_dynamic_baudrate'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetProtocol1BrickletName = namedtuple('Protocol1BrickletName', ['protocol_version', 'firmware_version', 'name'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickIMU(Device):
    r"""
    Full fledged AHRS with 9 degrees of freedom
    """

    DEVICE_IDENTIFIER = 16
    DEVICE_DISPLAY_NAME = 'IMU Brick'
    DEVICE_URL_PART = 'imu' # internal

    CALLBACK_ACCELERATION = 31
    CALLBACK_MAGNETIC_FIELD = 32
    CALLBACK_ANGULAR_VELOCITY = 33
    CALLBACK_ALL_DATA = 34
    CALLBACK_ORIENTATION = 35
    CALLBACK_QUATERNION = 36


    FUNCTION_GET_ACCELERATION = 1
    FUNCTION_GET_MAGNETIC_FIELD = 2
    FUNCTION_GET_ANGULAR_VELOCITY = 3
    FUNCTION_GET_ALL_DATA = 4
    FUNCTION_GET_ORIENTATION = 5
    FUNCTION_GET_QUATERNION = 6
    FUNCTION_GET_IMU_TEMPERATURE = 7
    FUNCTION_LEDS_ON = 8
    FUNCTION_LEDS_OFF = 9
    FUNCTION_ARE_LEDS_ON = 10
    FUNCTION_SET_ACCELERATION_RANGE = 11
    FUNCTION_GET_ACCELERATION_RANGE = 12
    FUNCTION_SET_MAGNETOMETER_RANGE = 13
    FUNCTION_GET_MAGNETOMETER_RANGE = 14
    FUNCTION_SET_CONVERGENCE_SPEED = 15
    FUNCTION_GET_CONVERGENCE_SPEED = 16
    FUNCTION_SET_CALIBRATION = 17
    FUNCTION_GET_CALIBRATION = 18
    FUNCTION_SET_ACCELERATION_PERIOD = 19
    FUNCTION_GET_ACCELERATION_PERIOD = 20
    FUNCTION_SET_MAGNETIC_FIELD_PERIOD = 21
    FUNCTION_GET_MAGNETIC_FIELD_PERIOD = 22
    FUNCTION_SET_ANGULAR_VELOCITY_PERIOD = 23
    FUNCTION_GET_ANGULAR_VELOCITY_PERIOD = 24
    FUNCTION_SET_ALL_DATA_PERIOD = 25
    FUNCTION_GET_ALL_DATA_PERIOD = 26
    FUNCTION_SET_ORIENTATION_PERIOD = 27
    FUNCTION_GET_ORIENTATION_PERIOD = 28
    FUNCTION_SET_QUATERNION_PERIOD = 29
    FUNCTION_GET_QUATERNION_PERIOD = 30
    FUNCTION_ORIENTATION_CALCULATION_ON = 37
    FUNCTION_ORIENTATION_CALCULATION_OFF = 38
    FUNCTION_IS_ORIENTATION_CALCULATION_ON = 39
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

    CALIBRATION_TYPE_ACCELEROMETER_GAIN = 0
    CALIBRATION_TYPE_ACCELEROMETER_BIAS = 1
    CALIBRATION_TYPE_MAGNETOMETER_GAIN = 2
    CALIBRATION_TYPE_MAGNETOMETER_BIAS = 3
    CALIBRATION_TYPE_GYROSCOPE_GAIN = 4
    CALIBRATION_TYPE_GYROSCOPE_BIAS = 5
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
        Device.__init__(self, uid, ipcon, BrickIMU.DEVICE_IDENTIFIER, BrickIMU.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 4)

        self.response_expected[BrickIMU.FUNCTION_GET_ACCELERATION] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_GET_MAGNETIC_FIELD] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_GET_ANGULAR_VELOCITY] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_GET_ALL_DATA] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_GET_ORIENTATION] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_GET_QUATERNION] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_GET_IMU_TEMPERATURE] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_LEDS_ON] = BrickIMU.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickIMU.FUNCTION_LEDS_OFF] = BrickIMU.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickIMU.FUNCTION_ARE_LEDS_ON] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_SET_ACCELERATION_RANGE] = BrickIMU.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickIMU.FUNCTION_GET_ACCELERATION_RANGE] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_SET_MAGNETOMETER_RANGE] = BrickIMU.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickIMU.FUNCTION_GET_MAGNETOMETER_RANGE] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_SET_CONVERGENCE_SPEED] = BrickIMU.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickIMU.FUNCTION_GET_CONVERGENCE_SPEED] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_SET_CALIBRATION] = BrickIMU.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickIMU.FUNCTION_GET_CALIBRATION] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_SET_ACCELERATION_PERIOD] = BrickIMU.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickIMU.FUNCTION_GET_ACCELERATION_PERIOD] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_SET_MAGNETIC_FIELD_PERIOD] = BrickIMU.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickIMU.FUNCTION_GET_MAGNETIC_FIELD_PERIOD] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_SET_ANGULAR_VELOCITY_PERIOD] = BrickIMU.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickIMU.FUNCTION_GET_ANGULAR_VELOCITY_PERIOD] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_SET_ALL_DATA_PERIOD] = BrickIMU.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickIMU.FUNCTION_GET_ALL_DATA_PERIOD] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_SET_ORIENTATION_PERIOD] = BrickIMU.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickIMU.FUNCTION_GET_ORIENTATION_PERIOD] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_SET_QUATERNION_PERIOD] = BrickIMU.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickIMU.FUNCTION_GET_QUATERNION_PERIOD] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_ORIENTATION_CALCULATION_ON] = BrickIMU.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickIMU.FUNCTION_ORIENTATION_CALCULATION_OFF] = BrickIMU.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickIMU.FUNCTION_IS_ORIENTATION_CALCULATION_ON] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_SET_SPITFP_BAUDRATE_CONFIG] = BrickIMU.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickIMU.FUNCTION_GET_SPITFP_BAUDRATE_CONFIG] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_GET_SEND_TIMEOUT_COUNT] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_SET_SPITFP_BAUDRATE] = BrickIMU.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickIMU.FUNCTION_GET_SPITFP_BAUDRATE] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_ENABLE_STATUS_LED] = BrickIMU.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickIMU.FUNCTION_DISABLE_STATUS_LED] = BrickIMU.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickIMU.FUNCTION_IS_STATUS_LED_ENABLED] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_GET_PROTOCOL1_BRICKLET_NAME] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_GET_CHIP_TEMPERATURE] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_RESET] = BrickIMU.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickIMU.FUNCTION_WRITE_BRICKLET_PLUGIN] = BrickIMU.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickIMU.FUNCTION_READ_BRICKLET_PLUGIN] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMU.FUNCTION_GET_IDENTITY] = BrickIMU.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickIMU.CALLBACK_ACCELERATION] = (14, 'h h h')
        self.callback_formats[BrickIMU.CALLBACK_MAGNETIC_FIELD] = (14, 'h h h')
        self.callback_formats[BrickIMU.CALLBACK_ANGULAR_VELOCITY] = (14, 'h h h')
        self.callback_formats[BrickIMU.CALLBACK_ALL_DATA] = (28, 'h h h h h h h h h h')
        self.callback_formats[BrickIMU.CALLBACK_ORIENTATION] = (14, 'h h h')
        self.callback_formats[BrickIMU.CALLBACK_QUATERNION] = (24, 'f f f f')

        ipcon.add_device(self)

    def get_acceleration(self):
        r"""
        Returns the calibrated acceleration from the accelerometer for the
        x, y and z axis.

        If you want to get the acceleration periodically, it is recommended
        to use the :cb:`Acceleration` callback and set the period with
        :func:`Set Acceleration Period`.
        """
        self.check_validity()

        return GetAcceleration(*self.ipcon.send_request(self, BrickIMU.FUNCTION_GET_ACCELERATION, (), '', 14, 'h h h'))

    def get_magnetic_field(self):
        r"""
        Returns the calibrated magnetic field from the magnetometer for the
        x, y and z axis.

        If you want to get the magnetic field periodically, it is recommended
        to use the :cb:`Magnetic Field` callback and set the period with
        :func:`Set Magnetic Field Period`.
        """
        self.check_validity()

        return GetMagneticField(*self.ipcon.send_request(self, BrickIMU.FUNCTION_GET_MAGNETIC_FIELD, (), '', 14, 'h h h'))

    def get_angular_velocity(self):
        r"""
        Returns the calibrated angular velocity from the gyroscope for the
        x, y and z axis in °/14.375s (you have to divide by 14.375 to
        get the value in °/s).

        If you want to get the angular velocity periodically, it is recommended
        to use the :cb:`Angular Velocity` callback and set the period with
        :func:`Set Angular Velocity Period`.
        """
        self.check_validity()

        return GetAngularVelocity(*self.ipcon.send_request(self, BrickIMU.FUNCTION_GET_ANGULAR_VELOCITY, (), '', 14, 'h h h'))

    def get_all_data(self):
        r"""
        Returns the data from :func:`Get Acceleration`, :func:`Get Magnetic Field`
        and :func:`Get Angular Velocity` as well as the temperature of the IMU Brick.

        If you want to get the data periodically, it is recommended
        to use the :cb:`All Data` callback and set the period with
        :func:`Set All Data Period`.
        """
        self.check_validity()

        return GetAllData(*self.ipcon.send_request(self, BrickIMU.FUNCTION_GET_ALL_DATA, (), '', 28, 'h h h h h h h h h h'))

    def get_orientation(self):
        r"""
        Returns the current orientation (roll, pitch, yaw) of the IMU Brick as Euler
        angles. Note that Euler angles always experience a
        `gimbal lock <https://en.wikipedia.org/wiki/Gimbal_lock>`__.

        We recommend that you use quaternions instead.

        The order to sequence in which the orientation values should be applied is
        roll, yaw, pitch.

        If you want to get the orientation periodically, it is recommended
        to use the :cb:`Orientation` callback and set the period with
        :func:`Set Orientation Period`.
        """
        self.check_validity()

        return GetOrientation(*self.ipcon.send_request(self, BrickIMU.FUNCTION_GET_ORIENTATION, (), '', 14, 'h h h'))

    def get_quaternion(self):
        r"""
        Returns the current orientation (x, y, z, w) of the IMU as
        `quaternions <https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation>`__.

        You can go from quaternions to Euler angles with the following formula::

         xAngle = atan2(2*y*w - 2*x*z, 1 - 2*y*y - 2*z*z)
         yAngle = atan2(2*x*w - 2*y*z, 1 - 2*x*x - 2*z*z)
         zAngle =  asin(2*x*y + 2*z*w)

        This process is not reversible, because of the
        `gimbal lock <https://en.wikipedia.org/wiki/Gimbal_lock>`__.

        It is also possible to calculate independent angles. You can calculate
        yaw, pitch and roll in a right-handed vehicle coordinate system according to
        DIN70000 with::

         yaw   =  atan2(2*x*y + 2*w*z, w*w + x*x - y*y - z*z)
         pitch = -asin(2*w*y - 2*x*z)
         roll  = -atan2(2*y*z + 2*w*x, -w*w + x*x + y*y - z*z))

        Converting the quaternions to an OpenGL transformation matrix is
        possible with the following formula::

         matrix = [[1 - 2*(y*y + z*z),     2*(x*y - w*z),     2*(x*z + w*y), 0],
                   [    2*(x*y + w*z), 1 - 2*(x*x + z*z),     2*(y*z - w*x), 0],
                   [    2*(x*z - w*y),     2*(y*z + w*x), 1 - 2*(x*x + y*y), 0],
                   [                0,                 0,                 0, 1]]

        If you want to get the quaternions periodically, it is recommended
        to use the :cb:`Quaternion` callback and set the period with
        :func:`Set Quaternion Period`.
        """
        self.check_validity()

        return GetQuaternion(*self.ipcon.send_request(self, BrickIMU.FUNCTION_GET_QUATERNION, (), '', 24, 'f f f f'))

    def get_imu_temperature(self):
        r"""
        Returns the temperature of the IMU Brick.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMU.FUNCTION_GET_IMU_TEMPERATURE, (), '', 10, 'h')

    def leds_on(self):
        r"""
        Turns the orientation and direction LEDs of the IMU Brick on.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickIMU.FUNCTION_LEDS_ON, (), '', 0, '')

    def leds_off(self):
        r"""
        Turns the orientation and direction LEDs of the IMU Brick off.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickIMU.FUNCTION_LEDS_OFF, (), '', 0, '')

    def are_leds_on(self):
        r"""
        Returns *true* if the orientation and direction LEDs of the IMU Brick
        are on, *false* otherwise.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMU.FUNCTION_ARE_LEDS_ON, (), '', 9, '!')

    def set_acceleration_range(self, range):
        r"""
        Not implemented yet.
        """
        self.check_validity()

        range = int(range)

        self.ipcon.send_request(self, BrickIMU.FUNCTION_SET_ACCELERATION_RANGE, (range,), 'B', 0, '')

    def get_acceleration_range(self):
        r"""
        Not implemented yet.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMU.FUNCTION_GET_ACCELERATION_RANGE, (), '', 9, 'B')

    def set_magnetometer_range(self, range):
        r"""
        Not implemented yet.
        """
        self.check_validity()

        range = int(range)

        self.ipcon.send_request(self, BrickIMU.FUNCTION_SET_MAGNETOMETER_RANGE, (range,), 'B', 0, '')

    def get_magnetometer_range(self):
        r"""
        Not implemented yet.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMU.FUNCTION_GET_MAGNETOMETER_RANGE, (), '', 9, 'B')

    def set_convergence_speed(self, speed):
        r"""
        Sets the convergence speed of the IMU Brick. The convergence speed
        determines how the different sensor measurements are fused.

        If the orientation of the IMU Brick is off by 10° and the convergence speed is
        set to 20°/s, it will take 0.5s until the orientation is corrected. However,
        if the correct orientation is reached and the convergence speed is too high,
        the orientation will fluctuate with the fluctuations of the accelerometer and
        the magnetometer.

        If you set the convergence speed to 0, practically only the gyroscope is used
        to calculate the orientation. This gives very smooth movements, but errors of the
        gyroscope will not be corrected. If you set the convergence speed to something
        above 500, practically only the magnetometer and the accelerometer are used to
        calculate the orientation. In this case the movements are abrupt and the values
        will fluctuate, but there won't be any errors that accumulate over time.

        In an application with high angular velocities, we recommend a high convergence
        speed, so the errors of the gyroscope can be corrected fast. In applications with
        only slow movements we recommend a low convergence speed. You can change the
        convergence speed on the fly. So it is possible (and recommended) to increase
        the convergence speed before an abrupt movement and decrease it afterwards
        again.

        You might want to play around with the convergence speed in the Brick Viewer to
        get a feeling for a good value for your application.
        """
        self.check_validity()

        speed = int(speed)

        self.ipcon.send_request(self, BrickIMU.FUNCTION_SET_CONVERGENCE_SPEED, (speed,), 'H', 0, '')

    def get_convergence_speed(self):
        r"""
        Returns the convergence speed as set by :func:`Set Convergence Speed`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMU.FUNCTION_GET_CONVERGENCE_SPEED, (), '', 10, 'H')

    def set_calibration(self, typ, data):
        r"""
        There are several different types that can be calibrated:

        .. csv-table::
         :header: "Type", "Description", "Values"
         :widths: 10, 30, 110

         "0",    "Accelerometer Gain", "``[mul x, mul y, mul z, div x, div y, div z, 0, 0, 0, 0]``"
         "1",    "Accelerometer Bias", "``[bias x, bias y, bias z, 0, 0, 0, 0, 0, 0, 0]``"
         "2",    "Magnetometer Gain",  "``[mul x, mul y, mul z, div x, div y, div z, 0, 0, 0, 0]``"
         "3",    "Magnetometer Bias",  "``[bias x, bias y, bias z, 0, 0, 0, 0, 0, 0, 0]``"
         "4",    "Gyroscope Gain",     "``[mul x, mul y, mul z, div x, div y, div z, 0, 0, 0, 0]``"
         "5",    "Gyroscope Bias",     "``[bias xl, bias yl, bias zl, temp l, bias xh, bias yh, bias zh, temp h, 0, 0]``"

        The calibration via gain and bias is done with the following formula::

         new_value = (bias + orig_value) * gain_mul / gain_div

        If you really want to write your own calibration software, please keep
        in mind that you first have to undo the old calibration (set bias to 0 and
        gain to 1/1) and that you have to average over several thousand values
        to obtain a usable result in the end.

        The gyroscope bias is highly dependent on the temperature, so you have to
        calibrate the bias two times with different temperatures. The values ``xl``,
        ``yl``, ``zl`` and ``temp l`` are the bias for ``x``, ``y``, ``z`` and the
        corresponding temperature for a low temperature. The values ``xh``, ``yh``,
        ``zh`` and ``temp h`` are the same for a high temperatures. The temperature
        difference should be at least 5°C. If you have a temperature where the
        IMU Brick is mostly used, you should use this temperature for one of the
        sampling points.

        .. note::
         We highly recommend that you use the Brick Viewer to calibrate your
         IMU Brick.
        """
        self.check_validity()

        typ = int(typ)
        data = list(map(int, data))

        self.ipcon.send_request(self, BrickIMU.FUNCTION_SET_CALIBRATION, (typ, data), 'B 10h', 0, '')

    def get_calibration(self, typ):
        r"""
        Returns the calibration for a given type as set by :func:`Set Calibration`.
        """
        self.check_validity()

        typ = int(typ)

        return self.ipcon.send_request(self, BrickIMU.FUNCTION_GET_CALIBRATION, (typ,), 'B', 28, '10h')

    def set_acceleration_period(self, period):
        r"""
        Sets the period with which the :cb:`Acceleration` callback is triggered
        periodically. A value of 0 turns the callback off.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickIMU.FUNCTION_SET_ACCELERATION_PERIOD, (period,), 'I', 0, '')

    def get_acceleration_period(self):
        r"""
        Returns the period as set by :func:`Set Acceleration Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMU.FUNCTION_GET_ACCELERATION_PERIOD, (), '', 12, 'I')

    def set_magnetic_field_period(self, period):
        r"""
        Sets the period with which the :cb:`Magnetic Field` callback is
        triggered periodically. A value of 0 turns the callback off.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickIMU.FUNCTION_SET_MAGNETIC_FIELD_PERIOD, (period,), 'I', 0, '')

    def get_magnetic_field_period(self):
        r"""
        Returns the period as set by :func:`Set Magnetic Field Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMU.FUNCTION_GET_MAGNETIC_FIELD_PERIOD, (), '', 12, 'I')

    def set_angular_velocity_period(self, period):
        r"""
        Sets the period with which the :cb:`Angular Velocity` callback is
        triggered periodically. A value of 0 turns the callback off.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickIMU.FUNCTION_SET_ANGULAR_VELOCITY_PERIOD, (period,), 'I', 0, '')

    def get_angular_velocity_period(self):
        r"""
        Returns the period as set by :func:`Set Angular Velocity Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMU.FUNCTION_GET_ANGULAR_VELOCITY_PERIOD, (), '', 12, 'I')

    def set_all_data_period(self, period):
        r"""
        Sets the period with which the :cb:`All Data` callback is triggered
        periodically. A value of 0 turns the callback off.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickIMU.FUNCTION_SET_ALL_DATA_PERIOD, (period,), 'I', 0, '')

    def get_all_data_period(self):
        r"""
        Returns the period as set by :func:`Set All Data Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMU.FUNCTION_GET_ALL_DATA_PERIOD, (), '', 12, 'I')

    def set_orientation_period(self, period):
        r"""
        Sets the period with which the :cb:`Orientation` callback is triggered
        periodically. A value of 0 turns the callback off.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickIMU.FUNCTION_SET_ORIENTATION_PERIOD, (period,), 'I', 0, '')

    def get_orientation_period(self):
        r"""
        Returns the period as set by :func:`Set Orientation Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMU.FUNCTION_GET_ORIENTATION_PERIOD, (), '', 12, 'I')

    def set_quaternion_period(self, period):
        r"""
        Sets the period with which the :cb:`Quaternion` callback is triggered
        periodically. A value of 0 turns the callback off.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickIMU.FUNCTION_SET_QUATERNION_PERIOD, (period,), 'I', 0, '')

    def get_quaternion_period(self):
        r"""
        Returns the period as set by :func:`Set Quaternion Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMU.FUNCTION_GET_QUATERNION_PERIOD, (), '', 12, 'I')

    def orientation_calculation_on(self):
        r"""
        Turns the orientation calculation of the IMU Brick on.

        As default the calculation is on.

        .. versionadded:: 2.0.2$nbsp;(Firmware)
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickIMU.FUNCTION_ORIENTATION_CALCULATION_ON, (), '', 0, '')

    def orientation_calculation_off(self):
        r"""
        Turns the orientation calculation of the IMU Brick off.

        If the calculation is off, :func:`Get Orientation` will return
        the last calculated value until the calculation is turned on again.

        The trigonometric functions that are needed to calculate the orientation
        are very expensive. We recommend to turn the orientation calculation
        off if the orientation is not needed, to free calculation time for the
        sensor fusion algorithm.

        As default the calculation is on.

        .. versionadded:: 2.0.2$nbsp;(Firmware)
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickIMU.FUNCTION_ORIENTATION_CALCULATION_OFF, (), '', 0, '')

    def is_orientation_calculation_on(self):
        r"""
        Returns *true* if the orientation calculation of the IMU Brick
        is on, *false* otherwise.

        .. versionadded:: 2.0.2$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMU.FUNCTION_IS_ORIENTATION_CALCULATION_ON, (), '', 9, '!')

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

        self.ipcon.send_request(self, BrickIMU.FUNCTION_SET_SPITFP_BAUDRATE_CONFIG, (enable_dynamic_baudrate, minimum_dynamic_baudrate), '! I', 0, '')

    def get_spitfp_baudrate_config(self):
        r"""
        Returns the baudrate config, see :func:`Set SPITFP Baudrate Config`.

        .. versionadded:: 2.3.5$nbsp;(Firmware)
        """
        self.check_validity()

        return GetSPITFPBaudrateConfig(*self.ipcon.send_request(self, BrickIMU.FUNCTION_GET_SPITFP_BAUDRATE_CONFIG, (), '', 13, '! I'))

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

        return self.ipcon.send_request(self, BrickIMU.FUNCTION_GET_SEND_TIMEOUT_COUNT, (communication_method,), 'B', 12, 'I')

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

        self.ipcon.send_request(self, BrickIMU.FUNCTION_SET_SPITFP_BAUDRATE, (bricklet_port, baudrate), 'c I', 0, '')

    def get_spitfp_baudrate(self, bricklet_port):
        r"""
        Returns the baudrate for a given Bricklet port, see :func:`Set SPITFP Baudrate`.

        .. versionadded:: 2.3.3$nbsp;(Firmware)
        """
        self.check_validity()

        bricklet_port = create_char(bricklet_port)

        return self.ipcon.send_request(self, BrickIMU.FUNCTION_GET_SPITFP_BAUDRATE, (bricklet_port,), 'c', 12, 'I')

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickIMU.FUNCTION_GET_SPITFP_ERROR_COUNT, (bricklet_port,), 'c', 24, 'I I I I'))

    def enable_status_led(self):
        r"""
        Enables the status LED.

        The status LED is the blue LED next to the USB connector. If enabled is is
        on and it flickers if data is transfered. If disabled it is always off.

        The default state is enabled.

        .. versionadded:: 2.3.1$nbsp;(Firmware)
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickIMU.FUNCTION_ENABLE_STATUS_LED, (), '', 0, '')

    def disable_status_led(self):
        r"""
        Disables the status LED.

        The status LED is the blue LED next to the USB connector. If enabled is is
        on and it flickers if data is transfered. If disabled it is always off.

        The default state is enabled.

        .. versionadded:: 2.3.1$nbsp;(Firmware)
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickIMU.FUNCTION_DISABLE_STATUS_LED, (), '', 0, '')

    def is_status_led_enabled(self):
        r"""
        Returns *true* if the status LED is enabled, *false* otherwise.

        .. versionadded:: 2.3.1$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMU.FUNCTION_IS_STATUS_LED_ENABLED, (), '', 9, '!')

    def get_protocol1_bricklet_name(self, port):
        r"""
        Returns the firmware and protocol version and the name of the Bricklet for a
        given port.

        This functions sole purpose is to allow automatic flashing of v1.x.y Bricklet
        plugins.
        """
        self.check_validity()

        port = create_char(port)

        return GetProtocol1BrickletName(*self.ipcon.send_request(self, BrickIMU.FUNCTION_GET_PROTOCOL1_BRICKLET_NAME, (port,), 'c', 52, 'B 3B 40s'))

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has an
        accuracy of ±15%. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMU.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Brick. Calling this function
        on a Brick inside of a stack will reset the whole stack.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickIMU.FUNCTION_RESET, (), '', 0, '')

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

        self.ipcon.send_request(self, BrickIMU.FUNCTION_WRITE_BRICKLET_PLUGIN, (port, offset, chunk), 'c B 32B', 0, '')

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

        return self.ipcon.send_request(self, BrickIMU.FUNCTION_READ_BRICKLET_PLUGIN, (port, offset), 'c B', 40, '32B')

    def get_identity(self):
        r"""
        Returns the UID, the UID where the Brick is connected to,
        the position, the hardware and firmware version as well as the
        device identifier.

        The position is the position in the stack from '0' (bottom) to '8' (top).

        The device identifier numbers can be found :ref:`here <device_identifier>`.
        |device_identifier_constant|
        """
        return GetIdentity(*self.ipcon.send_request(self, BrickIMU.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

IMU = BrickIMU # for backward compatibility
