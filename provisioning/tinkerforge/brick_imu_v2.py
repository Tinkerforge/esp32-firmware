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
GetOrientation = namedtuple('Orientation', ['heading', 'roll', 'pitch'])
GetLinearAcceleration = namedtuple('LinearAcceleration', ['x', 'y', 'z'])
GetGravityVector = namedtuple('GravityVector', ['x', 'y', 'z'])
GetQuaternion = namedtuple('Quaternion', ['w', 'x', 'y', 'z'])
GetAllData = namedtuple('AllData', ['acceleration', 'magnetic_field', 'angular_velocity', 'euler_angle', 'quaternion', 'linear_acceleration', 'gravity_vector', 'temperature', 'calibration_status'])
GetSensorConfiguration = namedtuple('SensorConfiguration', ['magnetometer_rate', 'gyroscope_range', 'gyroscope_bandwidth', 'accelerometer_range', 'accelerometer_bandwidth'])
GetSPITFPBaudrateConfig = namedtuple('SPITFPBaudrateConfig', ['enable_dynamic_baudrate', 'minimum_dynamic_baudrate'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetProtocol1BrickletName = namedtuple('Protocol1BrickletName', ['protocol_version', 'firmware_version', 'name'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickIMUV2(Device):
    r"""
    Full fledged AHRS with 9 degrees of freedom
    """

    DEVICE_IDENTIFIER = 18
    DEVICE_DISPLAY_NAME = 'IMU Brick 2.0'
    DEVICE_URL_PART = 'imu_v2' # internal

    CALLBACK_ACCELERATION = 32
    CALLBACK_MAGNETIC_FIELD = 33
    CALLBACK_ANGULAR_VELOCITY = 34
    CALLBACK_TEMPERATURE = 35
    CALLBACK_LINEAR_ACCELERATION = 36
    CALLBACK_GRAVITY_VECTOR = 37
    CALLBACK_ORIENTATION = 38
    CALLBACK_QUATERNION = 39
    CALLBACK_ALL_DATA = 40


    FUNCTION_GET_ACCELERATION = 1
    FUNCTION_GET_MAGNETIC_FIELD = 2
    FUNCTION_GET_ANGULAR_VELOCITY = 3
    FUNCTION_GET_TEMPERATURE = 4
    FUNCTION_GET_ORIENTATION = 5
    FUNCTION_GET_LINEAR_ACCELERATION = 6
    FUNCTION_GET_GRAVITY_VECTOR = 7
    FUNCTION_GET_QUATERNION = 8
    FUNCTION_GET_ALL_DATA = 9
    FUNCTION_LEDS_ON = 10
    FUNCTION_LEDS_OFF = 11
    FUNCTION_ARE_LEDS_ON = 12
    FUNCTION_SAVE_CALIBRATION = 13
    FUNCTION_SET_ACCELERATION_PERIOD = 14
    FUNCTION_GET_ACCELERATION_PERIOD = 15
    FUNCTION_SET_MAGNETIC_FIELD_PERIOD = 16
    FUNCTION_GET_MAGNETIC_FIELD_PERIOD = 17
    FUNCTION_SET_ANGULAR_VELOCITY_PERIOD = 18
    FUNCTION_GET_ANGULAR_VELOCITY_PERIOD = 19
    FUNCTION_SET_TEMPERATURE_PERIOD = 20
    FUNCTION_GET_TEMPERATURE_PERIOD = 21
    FUNCTION_SET_ORIENTATION_PERIOD = 22
    FUNCTION_GET_ORIENTATION_PERIOD = 23
    FUNCTION_SET_LINEAR_ACCELERATION_PERIOD = 24
    FUNCTION_GET_LINEAR_ACCELERATION_PERIOD = 25
    FUNCTION_SET_GRAVITY_VECTOR_PERIOD = 26
    FUNCTION_GET_GRAVITY_VECTOR_PERIOD = 27
    FUNCTION_SET_QUATERNION_PERIOD = 28
    FUNCTION_GET_QUATERNION_PERIOD = 29
    FUNCTION_SET_ALL_DATA_PERIOD = 30
    FUNCTION_GET_ALL_DATA_PERIOD = 31
    FUNCTION_SET_SENSOR_CONFIGURATION = 41
    FUNCTION_GET_SENSOR_CONFIGURATION = 42
    FUNCTION_SET_SENSOR_FUSION_MODE = 43
    FUNCTION_GET_SENSOR_FUSION_MODE = 44
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

    MAGNETOMETER_RATE_2HZ = 0
    MAGNETOMETER_RATE_6HZ = 1
    MAGNETOMETER_RATE_8HZ = 2
    MAGNETOMETER_RATE_10HZ = 3
    MAGNETOMETER_RATE_15HZ = 4
    MAGNETOMETER_RATE_20HZ = 5
    MAGNETOMETER_RATE_25HZ = 6
    MAGNETOMETER_RATE_30HZ = 7
    GYROSCOPE_RANGE_2000DPS = 0
    GYROSCOPE_RANGE_1000DPS = 1
    GYROSCOPE_RANGE_500DPS = 2
    GYROSCOPE_RANGE_250DPS = 3
    GYROSCOPE_RANGE_125DPS = 4
    GYROSCOPE_BANDWIDTH_523HZ = 0
    GYROSCOPE_BANDWIDTH_230HZ = 1
    GYROSCOPE_BANDWIDTH_116HZ = 2
    GYROSCOPE_BANDWIDTH_47HZ = 3
    GYROSCOPE_BANDWIDTH_23HZ = 4
    GYROSCOPE_BANDWIDTH_12HZ = 5
    GYROSCOPE_BANDWIDTH_64HZ = 6
    GYROSCOPE_BANDWIDTH_32HZ = 7
    ACCELEROMETER_RANGE_2G = 0
    ACCELEROMETER_RANGE_4G = 1
    ACCELEROMETER_RANGE_8G = 2
    ACCELEROMETER_RANGE_16G = 3
    ACCELEROMETER_BANDWIDTH_7_81HZ = 0
    ACCELEROMETER_BANDWIDTH_15_63HZ = 1
    ACCELEROMETER_BANDWIDTH_31_25HZ = 2
    ACCELEROMETER_BANDWIDTH_62_5HZ = 3
    ACCELEROMETER_BANDWIDTH_125HZ = 4
    ACCELEROMETER_BANDWIDTH_250HZ = 5
    ACCELEROMETER_BANDWIDTH_500HZ = 6
    ACCELEROMETER_BANDWIDTH_1000HZ = 7
    SENSOR_FUSION_OFF = 0
    SENSOR_FUSION_ON = 1
    SENSOR_FUSION_ON_WITHOUT_MAGNETOMETER = 2
    SENSOR_FUSION_ON_WITHOUT_FAST_MAGNETOMETER_CALIBRATION = 3
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
        Device.__init__(self, uid, ipcon, BrickIMUV2.DEVICE_IDENTIFIER, BrickIMUV2.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 3)

        self.response_expected[BrickIMUV2.FUNCTION_GET_ACCELERATION] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_GET_MAGNETIC_FIELD] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_GET_ANGULAR_VELOCITY] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_GET_TEMPERATURE] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_GET_ORIENTATION] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_GET_LINEAR_ACCELERATION] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_GET_GRAVITY_VECTOR] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_GET_QUATERNION] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_GET_ALL_DATA] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_LEDS_ON] = BrickIMUV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickIMUV2.FUNCTION_LEDS_OFF] = BrickIMUV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickIMUV2.FUNCTION_ARE_LEDS_ON] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_SAVE_CALIBRATION] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_SET_ACCELERATION_PERIOD] = BrickIMUV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_GET_ACCELERATION_PERIOD] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_SET_MAGNETIC_FIELD_PERIOD] = BrickIMUV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_GET_MAGNETIC_FIELD_PERIOD] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_SET_ANGULAR_VELOCITY_PERIOD] = BrickIMUV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_GET_ANGULAR_VELOCITY_PERIOD] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_SET_TEMPERATURE_PERIOD] = BrickIMUV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_GET_TEMPERATURE_PERIOD] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_SET_ORIENTATION_PERIOD] = BrickIMUV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_GET_ORIENTATION_PERIOD] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_SET_LINEAR_ACCELERATION_PERIOD] = BrickIMUV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_GET_LINEAR_ACCELERATION_PERIOD] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_SET_GRAVITY_VECTOR_PERIOD] = BrickIMUV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_GET_GRAVITY_VECTOR_PERIOD] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_SET_QUATERNION_PERIOD] = BrickIMUV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_GET_QUATERNION_PERIOD] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_SET_ALL_DATA_PERIOD] = BrickIMUV2.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_GET_ALL_DATA_PERIOD] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_SET_SENSOR_CONFIGURATION] = BrickIMUV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickIMUV2.FUNCTION_GET_SENSOR_CONFIGURATION] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_SET_SENSOR_FUSION_MODE] = BrickIMUV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickIMUV2.FUNCTION_GET_SENSOR_FUSION_MODE] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_SET_SPITFP_BAUDRATE_CONFIG] = BrickIMUV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickIMUV2.FUNCTION_GET_SPITFP_BAUDRATE_CONFIG] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_GET_SEND_TIMEOUT_COUNT] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_SET_SPITFP_BAUDRATE] = BrickIMUV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickIMUV2.FUNCTION_GET_SPITFP_BAUDRATE] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_ENABLE_STATUS_LED] = BrickIMUV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickIMUV2.FUNCTION_DISABLE_STATUS_LED] = BrickIMUV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickIMUV2.FUNCTION_IS_STATUS_LED_ENABLED] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_GET_PROTOCOL1_BRICKLET_NAME] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_GET_CHIP_TEMPERATURE] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_RESET] = BrickIMUV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickIMUV2.FUNCTION_WRITE_BRICKLET_PLUGIN] = BrickIMUV2.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickIMUV2.FUNCTION_READ_BRICKLET_PLUGIN] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickIMUV2.FUNCTION_GET_IDENTITY] = BrickIMUV2.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickIMUV2.CALLBACK_ACCELERATION] = (14, 'h h h')
        self.callback_formats[BrickIMUV2.CALLBACK_MAGNETIC_FIELD] = (14, 'h h h')
        self.callback_formats[BrickIMUV2.CALLBACK_ANGULAR_VELOCITY] = (14, 'h h h')
        self.callback_formats[BrickIMUV2.CALLBACK_TEMPERATURE] = (9, 'b')
        self.callback_formats[BrickIMUV2.CALLBACK_LINEAR_ACCELERATION] = (14, 'h h h')
        self.callback_formats[BrickIMUV2.CALLBACK_GRAVITY_VECTOR] = (14, 'h h h')
        self.callback_formats[BrickIMUV2.CALLBACK_ORIENTATION] = (14, 'h h h')
        self.callback_formats[BrickIMUV2.CALLBACK_QUATERNION] = (16, 'h h h h')
        self.callback_formats[BrickIMUV2.CALLBACK_ALL_DATA] = (54, '3h 3h 3h 3h 4h 3h 3h b B')

        ipcon.add_device(self)

    def get_acceleration(self):
        r"""
        Returns the calibrated acceleration from the accelerometer for the
        x, y and z axis. The acceleration is in the range configured with
        :func:`Set Sensor Configuration`.

        If you want to get the acceleration periodically, it is recommended
        to use the :cb:`Acceleration` callback and set the period with
        :func:`Set Acceleration Period`.
        """
        self.check_validity()

        return GetAcceleration(*self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_ACCELERATION, (), '', 14, 'h h h'))

    def get_magnetic_field(self):
        r"""
        Returns the calibrated magnetic field from the magnetometer for the
        x, y and z axis.

        If you want to get the magnetic field periodically, it is recommended
        to use the :cb:`Magnetic Field` callback and set the period with
        :func:`Set Magnetic Field Period`.
        """
        self.check_validity()

        return GetMagneticField(*self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_MAGNETIC_FIELD, (), '', 14, 'h h h'))

    def get_angular_velocity(self):
        r"""
        Returns the calibrated angular velocity from the gyroscope for the
        x, y and z axis. The angular velocity is in the range configured with
        :func:`Set Sensor Configuration`.

        If you want to get the angular velocity periodically, it is recommended
        to use the :cb:`Angular Velocity` acallback nd set the period with
        :func:`Set Angular Velocity Period`.
        """
        self.check_validity()

        return GetAngularVelocity(*self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_ANGULAR_VELOCITY, (), '', 14, 'h h h'))

    def get_temperature(self):
        r"""
        Returns the temperature of the IMU Brick.
        The temperature is measured in the core of the BNO055 IC, it is not the
        ambient temperature
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_TEMPERATURE, (), '', 9, 'b')

    def get_orientation(self):
        r"""
        Returns the current orientation (heading, roll, pitch) of the IMU Brick as
        independent Euler angles. Note that Euler angles always
        experience a `gimbal lock <https://en.wikipedia.org/wiki/Gimbal_lock>`__.
        We recommend that you use quaternions instead, if you need the absolute
        orientation.

        If you want to get the orientation periodically, it is recommended
        to use the :cb:`Orientation` callback and set the period with
        :func:`Set Orientation Period`.
        """
        self.check_validity()

        return GetOrientation(*self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_ORIENTATION, (), '', 14, 'h h h'))

    def get_linear_acceleration(self):
        r"""
        Returns the linear acceleration of the IMU Brick for the
        x, y and z axis. The acceleration is in the range configured with
        :func:`Set Sensor Configuration`.

        The linear acceleration is the acceleration in each of the three
        axis of the IMU Brick with the influences of gravity removed.

        It is also possible to get the gravity vector with the influence of linear
        acceleration removed, see :func:`Get Gravity Vector`.

        If you want to get the linear acceleration periodically, it is recommended
        to use the :cb:`Linear Acceleration` callback and set the period with
        :func:`Set Linear Acceleration Period`.
        """
        self.check_validity()

        return GetLinearAcceleration(*self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_LINEAR_ACCELERATION, (), '', 14, 'h h h'))

    def get_gravity_vector(self):
        r"""
        Returns the current gravity vector of the IMU Brick for the
        x, y and z axis.

        The gravity vector is the acceleration that occurs due to gravity.
        Influences of additional linear acceleration are removed.

        It is also possible to get the linear acceleration with the influence
        of gravity removed, see :func:`Get Linear Acceleration`.

        If you want to get the gravity vector periodically, it is recommended
        to use the :cb:`Gravity Vector` callback and set the period with
        :func:`Set Gravity Vector Period`.
        """
        self.check_validity()

        return GetGravityVector(*self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_GRAVITY_VECTOR, (), '', 14, 'h h h'))

    def get_quaternion(self):
        r"""
        Returns the current orientation (w, x, y, z) of the IMU Brick as
        `quaternions <https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation>`__.

        You have to divide the return values by 16383 (14 bit) to get
        the usual range of -1.0 to +1.0 for quaternions.

        If you want to get the quaternions periodically, it is recommended
        to use the :cb:`Quaternion` callback and set the period with
        :func:`Set Quaternion Period`.
        """
        self.check_validity()

        return GetQuaternion(*self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_QUATERNION, (), '', 16, 'h h h h'))

    def get_all_data(self):
        r"""
        Return all of the available data of the IMU Brick.

        * acceleration (see :func:`Get Acceleration`)
        * magnetic field (see :func:`Get Magnetic Field`)
        * angular velocity (see :func:`Get Angular Velocity`)
        * Euler angles (see :func:`Get Orientation`)
        * quaternion (see :func:`Get Quaternion`)
        * linear acceleration (see :func:`Get Linear Acceleration`)
        * gravity vector (see :func:`Get Gravity Vector`)
        * temperature (see :func:`Get Temperature`)
        * calibration status (see below)

        The calibration status consists of four pairs of two bits. Each pair
        of bits represents the status of the current calibration.

        * bit 0-1: Magnetometer
        * bit 2-3: Accelerometer
        * bit 4-5: Gyroscope
        * bit 6-7: System

        A value of 0 means for "not calibrated" and a value of 3 means
        "fully calibrated". In your program you should always be able to
        ignore the calibration status, it is used by the calibration
        window of the Brick Viewer and it can be ignored after the first
        calibration. See the documentation in the calibration window for
        more information regarding the calibration of the IMU Brick.

        If you want to get the data periodically, it is recommended
        to use the :cb:`All Data` callback and set the period with
        :func:`Set All Data Period`.
        """
        self.check_validity()

        return GetAllData(*self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_ALL_DATA, (), '', 54, '3h 3h 3h 3h 4h 3h 3h b B'))

    def leds_on(self):
        r"""
        Turns the orientation and direction LEDs of the IMU Brick on.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickIMUV2.FUNCTION_LEDS_ON, (), '', 0, '')

    def leds_off(self):
        r"""
        Turns the orientation and direction LEDs of the IMU Brick off.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickIMUV2.FUNCTION_LEDS_OFF, (), '', 0, '')

    def are_leds_on(self):
        r"""
        Returns *true* if the orientation and direction LEDs of the IMU Brick
        are on, *false* otherwise.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMUV2.FUNCTION_ARE_LEDS_ON, (), '', 9, '!')

    def save_calibration(self):
        r"""
        A call of this function saves the current calibration to be used
        as a starting point for the next restart of continuous calibration
        of the IMU Brick.

        A return value of *true* means that the calibration could be used and
        *false* means that it could not be used (this happens if the calibration
        status is not "fully calibrated").

        This function is used by the calibration window of the Brick Viewer, you
        should not need to call it in your program.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMUV2.FUNCTION_SAVE_CALIBRATION, (), '', 9, '!')

    def set_acceleration_period(self, period):
        r"""
        Sets the period with which the :cb:`Acceleration` callback is triggered
        periodically. A value of 0 turns the callback off.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickIMUV2.FUNCTION_SET_ACCELERATION_PERIOD, (period,), 'I', 0, '')

    def get_acceleration_period(self):
        r"""
        Returns the period as set by :func:`Set Acceleration Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_ACCELERATION_PERIOD, (), '', 12, 'I')

    def set_magnetic_field_period(self, period):
        r"""
        Sets the period with which the :cb:`Magnetic Field` callback is triggered
        periodically. A value of 0 turns the callback off.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickIMUV2.FUNCTION_SET_MAGNETIC_FIELD_PERIOD, (period,), 'I', 0, '')

    def get_magnetic_field_period(self):
        r"""
        Returns the period as set by :func:`Set Magnetic Field Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_MAGNETIC_FIELD_PERIOD, (), '', 12, 'I')

    def set_angular_velocity_period(self, period):
        r"""
        Sets the period with which the :cb:`Angular Velocity` callback is
        triggered periodically. A value of 0 turns the callback off.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickIMUV2.FUNCTION_SET_ANGULAR_VELOCITY_PERIOD, (period,), 'I', 0, '')

    def get_angular_velocity_period(self):
        r"""
        Returns the period as set by :func:`Set Angular Velocity Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_ANGULAR_VELOCITY_PERIOD, (), '', 12, 'I')

    def set_temperature_period(self, period):
        r"""
        Sets the period with which the :cb:`Temperature` callback is triggered
        periodically. A value of 0 turns the callback off.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickIMUV2.FUNCTION_SET_TEMPERATURE_PERIOD, (period,), 'I', 0, '')

    def get_temperature_period(self):
        r"""
        Returns the period as set by :func:`Set Temperature Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_TEMPERATURE_PERIOD, (), '', 12, 'I')

    def set_orientation_period(self, period):
        r"""
        Sets the period with which the :cb:`Orientation` callback is triggered
        periodically. A value of 0 turns the callback off.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickIMUV2.FUNCTION_SET_ORIENTATION_PERIOD, (period,), 'I', 0, '')

    def get_orientation_period(self):
        r"""
        Returns the period as set by :func:`Set Orientation Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_ORIENTATION_PERIOD, (), '', 12, 'I')

    def set_linear_acceleration_period(self, period):
        r"""
        Sets the period with which the :cb:`Linear Acceleration` callback is
        triggered periodically. A value of 0 turns the callback off.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickIMUV2.FUNCTION_SET_LINEAR_ACCELERATION_PERIOD, (period,), 'I', 0, '')

    def get_linear_acceleration_period(self):
        r"""
        Returns the period as set by :func:`Set Linear Acceleration Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_LINEAR_ACCELERATION_PERIOD, (), '', 12, 'I')

    def set_gravity_vector_period(self, period):
        r"""
        Sets the period with which the :cb:`Gravity Vector` callback is triggered
        periodically. A value of 0 turns the callback off.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickIMUV2.FUNCTION_SET_GRAVITY_VECTOR_PERIOD, (period,), 'I', 0, '')

    def get_gravity_vector_period(self):
        r"""
        Returns the period as set by :func:`Set Gravity Vector Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_GRAVITY_VECTOR_PERIOD, (), '', 12, 'I')

    def set_quaternion_period(self, period):
        r"""
        Sets the period with which the :cb:`Quaternion` callback is triggered
        periodically. A value of 0 turns the callback off.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickIMUV2.FUNCTION_SET_QUATERNION_PERIOD, (period,), 'I', 0, '')

    def get_quaternion_period(self):
        r"""
        Returns the period as set by :func:`Set Quaternion Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_QUATERNION_PERIOD, (), '', 12, 'I')

    def set_all_data_period(self, period):
        r"""
        Sets the period with which the :cb:`All Data` callback is triggered
        periodically. A value of 0 turns the callback off.
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickIMUV2.FUNCTION_SET_ALL_DATA_PERIOD, (period,), 'I', 0, '')

    def get_all_data_period(self):
        r"""
        Returns the period as set by :func:`Set All Data Period`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_ALL_DATA_PERIOD, (), '', 12, 'I')

    def set_sensor_configuration(self, magnetometer_rate, gyroscope_range, gyroscope_bandwidth, accelerometer_range, accelerometer_bandwidth):
        r"""
        Sets the available sensor configuration for the Magnetometer, Gyroscope and
        Accelerometer. The Accelerometer Range is user selectable in all fusion modes,
        all other configurations are auto-controlled in fusion mode.

        .. versionadded:: 2.0.5$nbsp;(Firmware)
        """
        self.check_validity()

        magnetometer_rate = int(magnetometer_rate)
        gyroscope_range = int(gyroscope_range)
        gyroscope_bandwidth = int(gyroscope_bandwidth)
        accelerometer_range = int(accelerometer_range)
        accelerometer_bandwidth = int(accelerometer_bandwidth)

        self.ipcon.send_request(self, BrickIMUV2.FUNCTION_SET_SENSOR_CONFIGURATION, (magnetometer_rate, gyroscope_range, gyroscope_bandwidth, accelerometer_range, accelerometer_bandwidth), 'B B B B B', 0, '')

    def get_sensor_configuration(self):
        r"""
        Returns the sensor configuration as set by :func:`Set Sensor Configuration`.

        .. versionadded:: 2.0.5$nbsp;(Firmware)
        """
        self.check_validity()

        return GetSensorConfiguration(*self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_SENSOR_CONFIGURATION, (), '', 13, 'B B B B B'))

    def set_sensor_fusion_mode(self, mode):
        r"""
        If the fusion mode is turned off, the functions :func:`Get Acceleration`,
        :func:`Get Magnetic Field` and :func:`Get Angular Velocity` return uncalibrated
        and uncompensated sensor data. All other sensor data getters return no data.

        Since firmware version 2.0.6 you can also use a fusion mode without magnetometer.
        In this mode the calculated orientation is relative (with magnetometer it is
        absolute with respect to the earth). However, the calculation can't be influenced
        by spurious magnetic fields.

        Since firmware version 2.0.13 you can also use a fusion mode without fast
        magnetometer calibration. This mode is the same as the normal fusion mode,
        but the fast magnetometer calibration is turned off. So to find the orientation
        the first time will likely take longer, but small magnetic influences might
        not affect the automatic calibration as much.

        .. versionadded:: 2.0.5$nbsp;(Firmware)
        """
        self.check_validity()

        mode = int(mode)

        self.ipcon.send_request(self, BrickIMUV2.FUNCTION_SET_SENSOR_FUSION_MODE, (mode,), 'B', 0, '')

    def get_sensor_fusion_mode(self):
        r"""
        Returns the sensor fusion mode as set by :func:`Set Sensor Fusion Mode`.

        .. versionadded:: 2.0.5$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_SENSOR_FUSION_MODE, (), '', 9, 'B')

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

        .. versionadded:: 2.0.10$nbsp;(Firmware)
        """
        self.check_validity()

        enable_dynamic_baudrate = bool(enable_dynamic_baudrate)
        minimum_dynamic_baudrate = int(minimum_dynamic_baudrate)

        self.ipcon.send_request(self, BrickIMUV2.FUNCTION_SET_SPITFP_BAUDRATE_CONFIG, (enable_dynamic_baudrate, minimum_dynamic_baudrate), '! I', 0, '')

    def get_spitfp_baudrate_config(self):
        r"""
        Returns the baudrate config, see :func:`Set SPITFP Baudrate Config`.

        .. versionadded:: 2.0.10$nbsp;(Firmware)
        """
        self.check_validity()

        return GetSPITFPBaudrateConfig(*self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_SPITFP_BAUDRATE_CONFIG, (), '', 13, '! I'))

    def get_send_timeout_count(self, communication_method):
        r"""
        Returns the timeout count for the different communication methods.

        The methods 0-2 are available for all Bricks, 3-7 only for Master Bricks.

        This function is mostly used for debugging during development, in normal operation
        the counters should nearly always stay at 0.

        .. versionadded:: 2.0.7$nbsp;(Firmware)
        """
        self.check_validity()

        communication_method = int(communication_method)

        return self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_SEND_TIMEOUT_COUNT, (communication_method,), 'B', 12, 'I')

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

        .. versionadded:: 2.0.5$nbsp;(Firmware)
        """
        self.check_validity()

        bricklet_port = create_char(bricklet_port)
        baudrate = int(baudrate)

        self.ipcon.send_request(self, BrickIMUV2.FUNCTION_SET_SPITFP_BAUDRATE, (bricklet_port, baudrate), 'c I', 0, '')

    def get_spitfp_baudrate(self, bricklet_port):
        r"""
        Returns the baudrate for a given Bricklet port, see :func:`Set SPITFP Baudrate`.

        .. versionadded:: 2.0.5$nbsp;(Firmware)
        """
        self.check_validity()

        bricklet_port = create_char(bricklet_port)

        return self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_SPITFP_BAUDRATE, (bricklet_port,), 'c', 12, 'I')

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

        .. versionadded:: 2.0.5$nbsp;(Firmware)
        """
        self.check_validity()

        bricklet_port = create_char(bricklet_port)

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_SPITFP_ERROR_COUNT, (bricklet_port,), 'c', 24, 'I I I I'))

    def enable_status_led(self):
        r"""
        Enables the status LED.

        The status LED is the blue LED next to the USB connector. If enabled is is
        on and it flickers if data is transfered. If disabled it is always off.

        The default state is enabled.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickIMUV2.FUNCTION_ENABLE_STATUS_LED, (), '', 0, '')

    def disable_status_led(self):
        r"""
        Disables the status LED.

        The status LED is the blue LED next to the USB connector. If enabled is is
        on and it flickers if data is transfered. If disabled it is always off.

        The default state is enabled.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickIMUV2.FUNCTION_DISABLE_STATUS_LED, (), '', 0, '')

    def is_status_led_enabled(self):
        r"""
        Returns *true* if the status LED is enabled, *false* otherwise.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMUV2.FUNCTION_IS_STATUS_LED_ENABLED, (), '', 9, '!')

    def get_protocol1_bricklet_name(self, port):
        r"""
        Returns the firmware and protocol version and the name of the Bricklet for a
        given port.

        This functions sole purpose is to allow automatic flashing of v1.x.y Bricklet
        plugins.
        """
        self.check_validity()

        port = create_char(port)

        return GetProtocol1BrickletName(*self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_PROTOCOL1_BRICKLET_NAME, (port,), 'c', 52, 'B 3B 40s'))

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has an
        accuracy of ±15%. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Brick. Calling this function
        on a Brick inside of a stack will reset the whole stack.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickIMUV2.FUNCTION_RESET, (), '', 0, '')

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

        self.ipcon.send_request(self, BrickIMUV2.FUNCTION_WRITE_BRICKLET_PLUGIN, (port, offset, chunk), 'c B 32B', 0, '')

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

        return self.ipcon.send_request(self, BrickIMUV2.FUNCTION_READ_BRICKLET_PLUGIN, (port, offset), 'c B', 40, '32B')

    def get_identity(self):
        r"""
        Returns the UID, the UID where the Brick is connected to,
        the position, the hardware and firmware version as well as the
        device identifier.

        The position is the position in the stack from '0' (bottom) to '8' (top).

        The device identifier numbers can be found :ref:`here <device_identifier>`.
        |device_identifier_constant|
        """
        return GetIdentity(*self.ipcon.send_request(self, BrickIMUV2.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

IMUV2 = BrickIMUV2 # for backward compatibility
