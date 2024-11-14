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
GetAccelerationCallbackConfiguration = namedtuple('AccelerationCallbackConfiguration', ['period', 'value_has_to_change'])
GetMagneticFieldCallbackConfiguration = namedtuple('MagneticFieldCallbackConfiguration', ['period', 'value_has_to_change'])
GetAngularVelocityCallbackConfiguration = namedtuple('AngularVelocityCallbackConfiguration', ['period', 'value_has_to_change'])
GetTemperatureCallbackConfiguration = namedtuple('TemperatureCallbackConfiguration', ['period', 'value_has_to_change'])
GetOrientationCallbackConfiguration = namedtuple('OrientationCallbackConfiguration', ['period', 'value_has_to_change'])
GetLinearAccelerationCallbackConfiguration = namedtuple('LinearAccelerationCallbackConfiguration', ['period', 'value_has_to_change'])
GetGravityVectorCallbackConfiguration = namedtuple('GravityVectorCallbackConfiguration', ['period', 'value_has_to_change'])
GetQuaternionCallbackConfiguration = namedtuple('QuaternionCallbackConfiguration', ['period', 'value_has_to_change'])
GetAllDataCallbackConfiguration = namedtuple('AllDataCallbackConfiguration', ['period', 'value_has_to_change'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletIMUV3(Device):
    r"""
    Full fledged AHRS with 9 degrees of freedom
    """

    DEVICE_IDENTIFIER = 2161
    DEVICE_DISPLAY_NAME = 'IMU Bricklet 3.0'
    DEVICE_URL_PART = 'imu_v3' # internal

    CALLBACK_ACCELERATION = 33
    CALLBACK_MAGNETIC_FIELD = 34
    CALLBACK_ANGULAR_VELOCITY = 35
    CALLBACK_TEMPERATURE = 36
    CALLBACK_LINEAR_ACCELERATION = 37
    CALLBACK_GRAVITY_VECTOR = 38
    CALLBACK_ORIENTATION = 39
    CALLBACK_QUATERNION = 40
    CALLBACK_ALL_DATA = 41


    FUNCTION_GET_ACCELERATION = 1
    FUNCTION_GET_MAGNETIC_FIELD = 2
    FUNCTION_GET_ANGULAR_VELOCITY = 3
    FUNCTION_GET_TEMPERATURE = 4
    FUNCTION_GET_ORIENTATION = 5
    FUNCTION_GET_LINEAR_ACCELERATION = 6
    FUNCTION_GET_GRAVITY_VECTOR = 7
    FUNCTION_GET_QUATERNION = 8
    FUNCTION_GET_ALL_DATA = 9
    FUNCTION_SAVE_CALIBRATION = 10
    FUNCTION_SET_SENSOR_CONFIGURATION = 11
    FUNCTION_GET_SENSOR_CONFIGURATION = 12
    FUNCTION_SET_SENSOR_FUSION_MODE = 13
    FUNCTION_GET_SENSOR_FUSION_MODE = 14
    FUNCTION_SET_ACCELERATION_CALLBACK_CONFIGURATION = 15
    FUNCTION_GET_ACCELERATION_CALLBACK_CONFIGURATION = 16
    FUNCTION_SET_MAGNETIC_FIELD_CALLBACK_CONFIGURATION = 17
    FUNCTION_GET_MAGNETIC_FIELD_CALLBACK_CONFIGURATION = 18
    FUNCTION_SET_ANGULAR_VELOCITY_CALLBACK_CONFIGURATION = 19
    FUNCTION_GET_ANGULAR_VELOCITY_CALLBACK_CONFIGURATION = 20
    FUNCTION_SET_TEMPERATURE_CALLBACK_CONFIGURATION = 21
    FUNCTION_GET_TEMPERATURE_CALLBACK_CONFIGURATION = 22
    FUNCTION_SET_ORIENTATION_CALLBACK_CONFIGURATION = 23
    FUNCTION_GET_ORIENTATION_CALLBACK_CONFIGURATION = 24
    FUNCTION_SET_LINEAR_ACCELERATION_CALLBACK_CONFIGURATION = 25
    FUNCTION_GET_LINEAR_ACCELERATION_CALLBACK_CONFIGURATION = 26
    FUNCTION_SET_GRAVITY_VECTOR_CALLBACK_CONFIGURATION = 27
    FUNCTION_GET_GRAVITY_VECTOR_CALLBACK_CONFIGURATION = 28
    FUNCTION_SET_QUATERNION_CALLBACK_CONFIGURATION = 29
    FUNCTION_GET_QUATERNION_CALLBACK_CONFIGURATION = 30
    FUNCTION_SET_ALL_DATA_CALLBACK_CONFIGURATION = 31
    FUNCTION_GET_ALL_DATA_CALLBACK_CONFIGURATION = 32
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
        Device.__init__(self, uid, ipcon, BrickletIMUV3.DEVICE_IDENTIFIER, BrickletIMUV3.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletIMUV3.FUNCTION_GET_ACCELERATION] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_GET_MAGNETIC_FIELD] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_GET_ANGULAR_VELOCITY] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_GET_TEMPERATURE] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_GET_ORIENTATION] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_GET_LINEAR_ACCELERATION] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_GET_GRAVITY_VECTOR] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_GET_QUATERNION] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_GET_ALL_DATA] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_SAVE_CALIBRATION] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_SET_SENSOR_CONFIGURATION] = BrickletIMUV3.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIMUV3.FUNCTION_GET_SENSOR_CONFIGURATION] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_SET_SENSOR_FUSION_MODE] = BrickletIMUV3.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIMUV3.FUNCTION_GET_SENSOR_FUSION_MODE] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_SET_ACCELERATION_CALLBACK_CONFIGURATION] = BrickletIMUV3.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_GET_ACCELERATION_CALLBACK_CONFIGURATION] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_SET_MAGNETIC_FIELD_CALLBACK_CONFIGURATION] = BrickletIMUV3.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_GET_MAGNETIC_FIELD_CALLBACK_CONFIGURATION] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_SET_ANGULAR_VELOCITY_CALLBACK_CONFIGURATION] = BrickletIMUV3.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_GET_ANGULAR_VELOCITY_CALLBACK_CONFIGURATION] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_SET_TEMPERATURE_CALLBACK_CONFIGURATION] = BrickletIMUV3.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_GET_TEMPERATURE_CALLBACK_CONFIGURATION] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_SET_ORIENTATION_CALLBACK_CONFIGURATION] = BrickletIMUV3.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_GET_ORIENTATION_CALLBACK_CONFIGURATION] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_SET_LINEAR_ACCELERATION_CALLBACK_CONFIGURATION] = BrickletIMUV3.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_GET_LINEAR_ACCELERATION_CALLBACK_CONFIGURATION] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_SET_GRAVITY_VECTOR_CALLBACK_CONFIGURATION] = BrickletIMUV3.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_GET_GRAVITY_VECTOR_CALLBACK_CONFIGURATION] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_SET_QUATERNION_CALLBACK_CONFIGURATION] = BrickletIMUV3.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_GET_QUATERNION_CALLBACK_CONFIGURATION] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_SET_ALL_DATA_CALLBACK_CONFIGURATION] = BrickletIMUV3.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_GET_ALL_DATA_CALLBACK_CONFIGURATION] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_SET_BOOTLOADER_MODE] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_GET_BOOTLOADER_MODE] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_SET_WRITE_FIRMWARE_POINTER] = BrickletIMUV3.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIMUV3.FUNCTION_WRITE_FIRMWARE] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_SET_STATUS_LED_CONFIG] = BrickletIMUV3.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIMUV3.FUNCTION_GET_STATUS_LED_CONFIG] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_GET_CHIP_TEMPERATURE] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_RESET] = BrickletIMUV3.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIMUV3.FUNCTION_WRITE_UID] = BrickletIMUV3.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletIMUV3.FUNCTION_READ_UID] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletIMUV3.FUNCTION_GET_IDENTITY] = BrickletIMUV3.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletIMUV3.CALLBACK_ACCELERATION] = (14, 'h h h')
        self.callback_formats[BrickletIMUV3.CALLBACK_MAGNETIC_FIELD] = (14, 'h h h')
        self.callback_formats[BrickletIMUV3.CALLBACK_ANGULAR_VELOCITY] = (14, 'h h h')
        self.callback_formats[BrickletIMUV3.CALLBACK_TEMPERATURE] = (9, 'b')
        self.callback_formats[BrickletIMUV3.CALLBACK_LINEAR_ACCELERATION] = (14, 'h h h')
        self.callback_formats[BrickletIMUV3.CALLBACK_GRAVITY_VECTOR] = (14, 'h h h')
        self.callback_formats[BrickletIMUV3.CALLBACK_ORIENTATION] = (14, 'h h h')
        self.callback_formats[BrickletIMUV3.CALLBACK_QUATERNION] = (16, 'h h h h')
        self.callback_formats[BrickletIMUV3.CALLBACK_ALL_DATA] = (54, '3h 3h 3h 3h 4h 3h 3h b B')

        ipcon.add_device(self)

    def get_acceleration(self):
        r"""
        Returns the calibrated acceleration from the accelerometer for the
        x, y and z axis. The acceleration is in the range configured with
        :func:`Set Sensor Configuration`.

        If you want to get the acceleration periodically, it is recommended
        to use the :cb:`Acceleration` callback and set the period with
        :func:`Set Acceleration Callback Configuration`.
        """
        self.check_validity()

        return GetAcceleration(*self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_ACCELERATION, (), '', 14, 'h h h'))

    def get_magnetic_field(self):
        r"""
        Returns the calibrated magnetic field from the magnetometer for the
        x, y and z axis.

        If you want to get the magnetic field periodically, it is recommended
        to use the :cb:`Magnetic Field` callback and set the period with
        :func:`Set Magnetic Field Callback Configuration`.
        """
        self.check_validity()

        return GetMagneticField(*self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_MAGNETIC_FIELD, (), '', 14, 'h h h'))

    def get_angular_velocity(self):
        r"""
        Returns the calibrated angular velocity from the gyroscope for the
        x, y and z axis. The angular velocity is in the range configured with
        :func:`Set Sensor Configuration`.

        If you want to get the angular velocity periodically, it is recommended
        to use the :cb:`Angular Velocity` acallback nd set the period with
        :func:`Set Angular Velocity Callback Configuration`.
        """
        self.check_validity()

        return GetAngularVelocity(*self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_ANGULAR_VELOCITY, (), '', 14, 'h h h'))

    def get_temperature(self):
        r"""
        Returns the temperature of the IMU Brick.
        The temperature is measured in the core of the BNO055 IC, it is not the
        ambient temperature
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_TEMPERATURE, (), '', 9, 'b')

    def get_orientation(self):
        r"""
        Returns the current orientation (heading, roll, pitch) of the IMU Brick as
        independent Euler angles. Note that Euler angles always
        experience a `gimbal lock <https://en.wikipedia.org/wiki/Gimbal_lock>`__.
        We recommend that you use quaternions instead, if you need the absolute
        orientation.

        If you want to get the orientation periodically, it is recommended
        to use the :cb:`Orientation` callback and set the period with
        :func:`Set Orientation Callback Configuration`.
        """
        self.check_validity()

        return GetOrientation(*self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_ORIENTATION, (), '', 14, 'h h h'))

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
        :func:`Set Linear Acceleration Callback Configuration`.
        """
        self.check_validity()

        return GetLinearAcceleration(*self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_LINEAR_ACCELERATION, (), '', 14, 'h h h'))

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
        :func:`Set Gravity Vector Callback Configuration`.
        """
        self.check_validity()

        return GetGravityVector(*self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_GRAVITY_VECTOR, (), '', 14, 'h h h'))

    def get_quaternion(self):
        r"""
        Returns the current orientation (w, x, y, z) of the IMU Brick as
        `quaternions <https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation>`__.

        You have to divide the return values by 16383 (14 bit) to get
        the usual range of -1.0 to +1.0 for quaternions.

        If you want to get the quaternions periodically, it is recommended
        to use the :cb:`Quaternion` callback and set the period with
        :func:`Set Quaternion Callback Configuration`.
        """
        self.check_validity()

        return GetQuaternion(*self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_QUATERNION, (), '', 16, 'h h h h'))

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
        :func:`Set All Data Callback Configuration`.
        """
        self.check_validity()

        return GetAllData(*self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_ALL_DATA, (), '', 54, '3h 3h 3h 3h 4h 3h 3h b B'))

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

        return self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_SAVE_CALIBRATION, (), '', 9, '!')

    def set_sensor_configuration(self, magnetometer_rate, gyroscope_range, gyroscope_bandwidth, accelerometer_range, accelerometer_bandwidth):
        r"""
        Sets the available sensor configuration for the Magnetometer, Gyroscope and
        Accelerometer. The Accelerometer Range is user selectable in all fusion modes,
        all other configurations are auto-controlled in fusion mode.
        """
        self.check_validity()

        magnetometer_rate = int(magnetometer_rate)
        gyroscope_range = int(gyroscope_range)
        gyroscope_bandwidth = int(gyroscope_bandwidth)
        accelerometer_range = int(accelerometer_range)
        accelerometer_bandwidth = int(accelerometer_bandwidth)

        self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_SET_SENSOR_CONFIGURATION, (magnetometer_rate, gyroscope_range, gyroscope_bandwidth, accelerometer_range, accelerometer_bandwidth), 'B B B B B', 0, '')

    def get_sensor_configuration(self):
        r"""
        Returns the sensor configuration as set by :func:`Set Sensor Configuration`.
        """
        self.check_validity()

        return GetSensorConfiguration(*self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_SENSOR_CONFIGURATION, (), '', 13, 'B B B B B'))

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
        """
        self.check_validity()

        mode = int(mode)

        self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_SET_SENSOR_FUSION_MODE, (mode,), 'B', 0, '')

    def get_sensor_fusion_mode(self):
        r"""
        Returns the sensor fusion mode as set by :func:`Set Sensor Fusion Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_SENSOR_FUSION_MODE, (), '', 9, 'B')

    def set_acceleration_callback_configuration(self, period, value_has_to_change):
        r"""
        The period is the period with which the :cb:`Acceleration` callback
        is triggered periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change within the
        period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.
        """
        self.check_validity()

        period = int(period)
        value_has_to_change = bool(value_has_to_change)

        self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_SET_ACCELERATION_CALLBACK_CONFIGURATION, (period, value_has_to_change), 'I !', 0, '')

    def get_acceleration_callback_configuration(self):
        r"""
        Returns the callback configuration as set by :func:`Set Acceleration Callback Configuration`.
        """
        self.check_validity()

        return GetAccelerationCallbackConfiguration(*self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_ACCELERATION_CALLBACK_CONFIGURATION, (), '', 13, 'I !'))

    def set_magnetic_field_callback_configuration(self, period, value_has_to_change):
        r"""
        The period is the period with which the :cb:`Magnetic Field` callback
        is triggered periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change within the
        period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.
        """
        self.check_validity()

        period = int(period)
        value_has_to_change = bool(value_has_to_change)

        self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_SET_MAGNETIC_FIELD_CALLBACK_CONFIGURATION, (period, value_has_to_change), 'I !', 0, '')

    def get_magnetic_field_callback_configuration(self):
        r"""
        Returns the callback configuration as set by :func:`Set Magnetic Field Callback Configuration`.
        """
        self.check_validity()

        return GetMagneticFieldCallbackConfiguration(*self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_MAGNETIC_FIELD_CALLBACK_CONFIGURATION, (), '', 13, 'I !'))

    def set_angular_velocity_callback_configuration(self, period, value_has_to_change):
        r"""
        The period is the period with which the :cb:`Angular Velocity` callback
        is triggered periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change within the
        period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.
        """
        self.check_validity()

        period = int(period)
        value_has_to_change = bool(value_has_to_change)

        self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_SET_ANGULAR_VELOCITY_CALLBACK_CONFIGURATION, (period, value_has_to_change), 'I !', 0, '')

    def get_angular_velocity_callback_configuration(self):
        r"""
        Returns the callback configuration as set by :func:`Set Angular Velocity Callback Configuration`.
        """
        self.check_validity()

        return GetAngularVelocityCallbackConfiguration(*self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_ANGULAR_VELOCITY_CALLBACK_CONFIGURATION, (), '', 13, 'I !'))

    def set_temperature_callback_configuration(self, period, value_has_to_change):
        r"""
        The period is the period with which the :cb:`Temperature` callback
        is triggered periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change within the
        period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.
        """
        self.check_validity()

        period = int(period)
        value_has_to_change = bool(value_has_to_change)

        self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_SET_TEMPERATURE_CALLBACK_CONFIGURATION, (period, value_has_to_change), 'I !', 0, '')

    def get_temperature_callback_configuration(self):
        r"""
        Returns the callback configuration as set by :func:`Set Temperature Callback Configuration`.
        """
        self.check_validity()

        return GetTemperatureCallbackConfiguration(*self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_TEMPERATURE_CALLBACK_CONFIGURATION, (), '', 13, 'I !'))

    def set_orientation_callback_configuration(self, period, value_has_to_change):
        r"""
        The period is the period with which the :cb:`Orientation` callback
        is triggered periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change within the
        period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.
        """
        self.check_validity()

        period = int(period)
        value_has_to_change = bool(value_has_to_change)

        self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_SET_ORIENTATION_CALLBACK_CONFIGURATION, (period, value_has_to_change), 'I !', 0, '')

    def get_orientation_callback_configuration(self):
        r"""
        Returns the callback configuration as set by :func:`Set Orientation Callback Configuration`.
        """
        self.check_validity()

        return GetOrientationCallbackConfiguration(*self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_ORIENTATION_CALLBACK_CONFIGURATION, (), '', 13, 'I !'))

    def set_linear_acceleration_callback_configuration(self, period, value_has_to_change):
        r"""
        The period is the period with which the :cb:`Linear Acceleration` callback
        is triggered periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change within the
        period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.
        """
        self.check_validity()

        period = int(period)
        value_has_to_change = bool(value_has_to_change)

        self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_SET_LINEAR_ACCELERATION_CALLBACK_CONFIGURATION, (period, value_has_to_change), 'I !', 0, '')

    def get_linear_acceleration_callback_configuration(self):
        r"""
        Returns the callback configuration as set by :func:`Set Linear Acceleration Callback Configuration`.
        """
        self.check_validity()

        return GetLinearAccelerationCallbackConfiguration(*self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_LINEAR_ACCELERATION_CALLBACK_CONFIGURATION, (), '', 13, 'I !'))

    def set_gravity_vector_callback_configuration(self, period, value_has_to_change):
        r"""
        The period is the period with which the :cb:`Gravity Vector` callback
        is triggered periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change within the
        period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.
        """
        self.check_validity()

        period = int(period)
        value_has_to_change = bool(value_has_to_change)

        self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_SET_GRAVITY_VECTOR_CALLBACK_CONFIGURATION, (period, value_has_to_change), 'I !', 0, '')

    def get_gravity_vector_callback_configuration(self):
        r"""
        Returns the callback configuration as set by :func:`Set Gravity Vector Callback Configuration`.
        """
        self.check_validity()

        return GetGravityVectorCallbackConfiguration(*self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_GRAVITY_VECTOR_CALLBACK_CONFIGURATION, (), '', 13, 'I !'))

    def set_quaternion_callback_configuration(self, period, value_has_to_change):
        r"""
        The period is the period with which the :cb:`Quaternion` callback
        is triggered periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change within the
        period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.
        """
        self.check_validity()

        period = int(period)
        value_has_to_change = bool(value_has_to_change)

        self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_SET_QUATERNION_CALLBACK_CONFIGURATION, (period, value_has_to_change), 'I !', 0, '')

    def get_quaternion_callback_configuration(self):
        r"""
        Returns the callback configuration as set by :func:`Set Quaternion Callback Configuration`.
        """
        self.check_validity()

        return GetQuaternionCallbackConfiguration(*self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_QUATERNION_CALLBACK_CONFIGURATION, (), '', 13, 'I !'))

    def set_all_data_callback_configuration(self, period, value_has_to_change):
        r"""
        The period is the period with which the :cb:`All Data` callback
        is triggered periodically. A value of 0 turns the callback off.

        If the `value has to change`-parameter is set to true, the callback is only
        triggered after the value has changed. If the value didn't change within the
        period, the callback is triggered immediately on change.

        If it is set to false, the callback is continuously triggered with the period,
        independent of the value.
        """
        self.check_validity()

        period = int(period)
        value_has_to_change = bool(value_has_to_change)

        self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_SET_ALL_DATA_CALLBACK_CONFIGURATION, (period, value_has_to_change), 'I !', 0, '')

    def get_all_data_callback_configuration(self):
        r"""
        Returns the callback configuration as set by :func:`Set All Data Callback Configuration`.
        """
        self.check_validity()

        return GetAllDataCallbackConfiguration(*self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_ALL_DATA_CALLBACK_CONFIGURATION, (), '', 13, 'I !'))

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

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_SPITFP_ERROR_COUNT, (), '', 24, 'I I I I'))

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

        return self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_SET_BOOTLOADER_MODE, (mode,), 'B', 9, 'B')

    def get_bootloader_mode(self):
        r"""
        Returns the current bootloader mode, see :func:`Set Bootloader Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_BOOTLOADER_MODE, (), '', 9, 'B')

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

        self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_SET_WRITE_FIRMWARE_POINTER, (pointer,), 'I', 0, '')

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

        return self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_WRITE_FIRMWARE, (data,), '64B', 9, 'B')

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

        self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_SET_STATUS_LED_CONFIG, (config,), 'B', 0, '')

    def get_status_led_config(self):
        r"""
        Returns the configuration as set by :func:`Set Status LED Config`
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_STATUS_LED_CONFIG, (), '', 9, 'B')

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has bad
        accuracy. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Bricklet. All configurations
        will be lost.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_RESET, (), '', 0, '')

    def write_uid(self, uid):
        r"""
        Writes a new UID into flash. If you want to set a new UID
        you have to decode the Base58 encoded UID string into an
        integer first.

        We recommend that you use Brick Viewer to change the UID.
        """
        self.check_validity()

        uid = int(uid)

        self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_WRITE_UID, (uid,), 'I', 0, '')

    def read_uid(self):
        r"""
        Returns the current UID as an integer. Encode as
        Base58 to get the usual string version.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_READ_UID, (), '', 12, 'I')

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
        return GetIdentity(*self.ipcon.send_request(self, BrickletIMUV3.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

IMUV3 = BrickletIMUV3 # for backward compatibility
