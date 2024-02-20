/* ***********************************************************
 * This file was automatically generated on 2024-02-20.      *
 *                                                           *
 * C/C++ for Microcontrollers Bindings Version 2.0.4         *
 *                                                           *
 * If you have a bugfix for this file and want to commit it, *
 * please fix the bug in the generator. You can find a link  *
 * to the generators git repository on tinkerforge.com       *
 *************************************************************/


#include "bricklet_imu_v3.h"
#include "base58.h"
#include "endian_convert.h"
#include "errors.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


#if TF_IMPLEMENT_CALLBACKS != 0
static bool tf_imu_v3_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    TF_IMUV3 *imu_v3 = (TF_IMUV3 *)device;
    TF_HALCommon *hal_common = tf_hal_get_common(imu_v3->tfp->spitfp->hal);
    (void)payload;

    switch (fid) {
        case TF_IMU_V3_CALLBACK_ACCELERATION: {
            TF_IMUV3_AccelerationHandler fn = imu_v3->acceleration_handler;
            void *user_data = imu_v3->acceleration_user_data;
            if (fn == NULL) {
                return false;
            }

            int16_t x = tf_packet_buffer_read_int16_t(payload);
            int16_t y = tf_packet_buffer_read_int16_t(payload);
            int16_t z = tf_packet_buffer_read_int16_t(payload);
            hal_common->locked = true;
            fn(imu_v3, x, y, z, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_IMU_V3_CALLBACK_MAGNETIC_FIELD: {
            TF_IMUV3_MagneticFieldHandler fn = imu_v3->magnetic_field_handler;
            void *user_data = imu_v3->magnetic_field_user_data;
            if (fn == NULL) {
                return false;
            }

            int16_t x = tf_packet_buffer_read_int16_t(payload);
            int16_t y = tf_packet_buffer_read_int16_t(payload);
            int16_t z = tf_packet_buffer_read_int16_t(payload);
            hal_common->locked = true;
            fn(imu_v3, x, y, z, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_IMU_V3_CALLBACK_ANGULAR_VELOCITY: {
            TF_IMUV3_AngularVelocityHandler fn = imu_v3->angular_velocity_handler;
            void *user_data = imu_v3->angular_velocity_user_data;
            if (fn == NULL) {
                return false;
            }

            int16_t x = tf_packet_buffer_read_int16_t(payload);
            int16_t y = tf_packet_buffer_read_int16_t(payload);
            int16_t z = tf_packet_buffer_read_int16_t(payload);
            hal_common->locked = true;
            fn(imu_v3, x, y, z, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_IMU_V3_CALLBACK_TEMPERATURE: {
            TF_IMUV3_TemperatureHandler fn = imu_v3->temperature_handler;
            void *user_data = imu_v3->temperature_user_data;
            if (fn == NULL) {
                return false;
            }

            int8_t temperature = tf_packet_buffer_read_int8_t(payload);
            hal_common->locked = true;
            fn(imu_v3, temperature, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_IMU_V3_CALLBACK_LINEAR_ACCELERATION: {
            TF_IMUV3_LinearAccelerationHandler fn = imu_v3->linear_acceleration_handler;
            void *user_data = imu_v3->linear_acceleration_user_data;
            if (fn == NULL) {
                return false;
            }

            int16_t x = tf_packet_buffer_read_int16_t(payload);
            int16_t y = tf_packet_buffer_read_int16_t(payload);
            int16_t z = tf_packet_buffer_read_int16_t(payload);
            hal_common->locked = true;
            fn(imu_v3, x, y, z, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_IMU_V3_CALLBACK_GRAVITY_VECTOR: {
            TF_IMUV3_GravityVectorHandler fn = imu_v3->gravity_vector_handler;
            void *user_data = imu_v3->gravity_vector_user_data;
            if (fn == NULL) {
                return false;
            }

            int16_t x = tf_packet_buffer_read_int16_t(payload);
            int16_t y = tf_packet_buffer_read_int16_t(payload);
            int16_t z = tf_packet_buffer_read_int16_t(payload);
            hal_common->locked = true;
            fn(imu_v3, x, y, z, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_IMU_V3_CALLBACK_ORIENTATION: {
            TF_IMUV3_OrientationHandler fn = imu_v3->orientation_handler;
            void *user_data = imu_v3->orientation_user_data;
            if (fn == NULL) {
                return false;
            }

            int16_t heading = tf_packet_buffer_read_int16_t(payload);
            int16_t roll = tf_packet_buffer_read_int16_t(payload);
            int16_t pitch = tf_packet_buffer_read_int16_t(payload);
            hal_common->locked = true;
            fn(imu_v3, heading, roll, pitch, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_IMU_V3_CALLBACK_QUATERNION: {
            TF_IMUV3_QuaternionHandler fn = imu_v3->quaternion_handler;
            void *user_data = imu_v3->quaternion_user_data;
            if (fn == NULL) {
                return false;
            }

            int16_t w = tf_packet_buffer_read_int16_t(payload);
            int16_t x = tf_packet_buffer_read_int16_t(payload);
            int16_t y = tf_packet_buffer_read_int16_t(payload);
            int16_t z = tf_packet_buffer_read_int16_t(payload);
            hal_common->locked = true;
            fn(imu_v3, w, x, y, z, user_data);
            hal_common->locked = false;
            break;
        }

        case TF_IMU_V3_CALLBACK_ALL_DATA: {
            TF_IMUV3_AllDataHandler fn = imu_v3->all_data_handler;
            void *user_data = imu_v3->all_data_user_data;
            if (fn == NULL) {
                return false;
            }
            size_t _i;
            int16_t acceleration[3]; for (_i = 0; _i < 3; ++_i) acceleration[_i] = tf_packet_buffer_read_int16_t(payload);
            int16_t magnetic_field[3]; for (_i = 0; _i < 3; ++_i) magnetic_field[_i] = tf_packet_buffer_read_int16_t(payload);
            int16_t angular_velocity[3]; for (_i = 0; _i < 3; ++_i) angular_velocity[_i] = tf_packet_buffer_read_int16_t(payload);
            int16_t euler_angle[3]; for (_i = 0; _i < 3; ++_i) euler_angle[_i] = tf_packet_buffer_read_int16_t(payload);
            int16_t quaternion[4]; for (_i = 0; _i < 4; ++_i) quaternion[_i] = tf_packet_buffer_read_int16_t(payload);
            int16_t linear_acceleration[3]; for (_i = 0; _i < 3; ++_i) linear_acceleration[_i] = tf_packet_buffer_read_int16_t(payload);
            int16_t gravity_vector[3]; for (_i = 0; _i < 3; ++_i) gravity_vector[_i] = tf_packet_buffer_read_int16_t(payload);
            int8_t temperature = tf_packet_buffer_read_int8_t(payload);
            uint8_t calibration_status = tf_packet_buffer_read_uint8_t(payload);
            hal_common->locked = true;
            fn(imu_v3, acceleration, magnetic_field, angular_velocity, euler_angle, quaternion, linear_acceleration, gravity_vector, temperature, calibration_status, user_data);
            hal_common->locked = false;
            break;
        }

        default:
            return false;
    }

    return true;
}
#else
static bool tf_imu_v3_callback_handler(void *device, uint8_t fid, TF_PacketBuffer *payload) {
    return false;
}
#endif
int tf_imu_v3_create(TF_IMUV3 *imu_v3, const char *uid_or_port_name, TF_HAL *hal) {
    if (imu_v3 == NULL || hal == NULL) {
        return TF_E_NULL;
    }

    memset(imu_v3, 0, sizeof(TF_IMUV3));

    TF_TFP *tfp;
    int rc = tf_hal_get_attachable_tfp(hal, &tfp, uid_or_port_name, TF_IMU_V3_DEVICE_IDENTIFIER);

    if (rc != TF_E_OK) {
        return rc;
    }

    imu_v3->tfp = tfp;
    imu_v3->tfp->device = imu_v3;
    imu_v3->tfp->cb_handler = tf_imu_v3_callback_handler;
    imu_v3->magic = 0x5446;
    imu_v3->response_expected[0] = 0xFC;
    imu_v3->response_expected[1] = 0x07;
    return TF_E_OK;
}

int tf_imu_v3_destroy(TF_IMUV3 *imu_v3) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }
    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    imu_v3->tfp->cb_handler = NULL;
    imu_v3->tfp->device = NULL;
    imu_v3->tfp = NULL;
    imu_v3->magic = 0;

    return TF_E_OK;
}

int tf_imu_v3_get_response_expected(TF_IMUV3 *imu_v3, uint8_t function_id, bool *ret_response_expected) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_IMU_V3_FUNCTION_SET_SENSOR_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (imu_v3->response_expected[0] & (1 << 0)) != 0;
            }
            break;
        case TF_IMU_V3_FUNCTION_SET_SENSOR_FUSION_MODE:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (imu_v3->response_expected[0] & (1 << 1)) != 0;
            }
            break;
        case TF_IMU_V3_FUNCTION_SET_ACCELERATION_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (imu_v3->response_expected[0] & (1 << 2)) != 0;
            }
            break;
        case TF_IMU_V3_FUNCTION_SET_MAGNETIC_FIELD_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (imu_v3->response_expected[0] & (1 << 3)) != 0;
            }
            break;
        case TF_IMU_V3_FUNCTION_SET_ANGULAR_VELOCITY_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (imu_v3->response_expected[0] & (1 << 4)) != 0;
            }
            break;
        case TF_IMU_V3_FUNCTION_SET_TEMPERATURE_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (imu_v3->response_expected[0] & (1 << 5)) != 0;
            }
            break;
        case TF_IMU_V3_FUNCTION_SET_ORIENTATION_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (imu_v3->response_expected[0] & (1 << 6)) != 0;
            }
            break;
        case TF_IMU_V3_FUNCTION_SET_LINEAR_ACCELERATION_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (imu_v3->response_expected[0] & (1 << 7)) != 0;
            }
            break;
        case TF_IMU_V3_FUNCTION_SET_GRAVITY_VECTOR_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (imu_v3->response_expected[1] & (1 << 0)) != 0;
            }
            break;
        case TF_IMU_V3_FUNCTION_SET_QUATERNION_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (imu_v3->response_expected[1] & (1 << 1)) != 0;
            }
            break;
        case TF_IMU_V3_FUNCTION_SET_ALL_DATA_CALLBACK_CONFIGURATION:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (imu_v3->response_expected[1] & (1 << 2)) != 0;
            }
            break;
        case TF_IMU_V3_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (imu_v3->response_expected[1] & (1 << 3)) != 0;
            }
            break;
        case TF_IMU_V3_FUNCTION_SET_STATUS_LED_CONFIG:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (imu_v3->response_expected[1] & (1 << 4)) != 0;
            }
            break;
        case TF_IMU_V3_FUNCTION_RESET:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (imu_v3->response_expected[1] & (1 << 5)) != 0;
            }
            break;
        case TF_IMU_V3_FUNCTION_WRITE_UID:
            if (ret_response_expected != NULL) {
                *ret_response_expected = (imu_v3->response_expected[1] & (1 << 6)) != 0;
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_imu_v3_set_response_expected(TF_IMUV3 *imu_v3, uint8_t function_id, bool response_expected) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    switch (function_id) {
        case TF_IMU_V3_FUNCTION_SET_SENSOR_CONFIGURATION:
            if (response_expected) {
                imu_v3->response_expected[0] |= (1 << 0);
            } else {
                imu_v3->response_expected[0] &= ~(1 << 0);
            }
            break;
        case TF_IMU_V3_FUNCTION_SET_SENSOR_FUSION_MODE:
            if (response_expected) {
                imu_v3->response_expected[0] |= (1 << 1);
            } else {
                imu_v3->response_expected[0] &= ~(1 << 1);
            }
            break;
        case TF_IMU_V3_FUNCTION_SET_ACCELERATION_CALLBACK_CONFIGURATION:
            if (response_expected) {
                imu_v3->response_expected[0] |= (1 << 2);
            } else {
                imu_v3->response_expected[0] &= ~(1 << 2);
            }
            break;
        case TF_IMU_V3_FUNCTION_SET_MAGNETIC_FIELD_CALLBACK_CONFIGURATION:
            if (response_expected) {
                imu_v3->response_expected[0] |= (1 << 3);
            } else {
                imu_v3->response_expected[0] &= ~(1 << 3);
            }
            break;
        case TF_IMU_V3_FUNCTION_SET_ANGULAR_VELOCITY_CALLBACK_CONFIGURATION:
            if (response_expected) {
                imu_v3->response_expected[0] |= (1 << 4);
            } else {
                imu_v3->response_expected[0] &= ~(1 << 4);
            }
            break;
        case TF_IMU_V3_FUNCTION_SET_TEMPERATURE_CALLBACK_CONFIGURATION:
            if (response_expected) {
                imu_v3->response_expected[0] |= (1 << 5);
            } else {
                imu_v3->response_expected[0] &= ~(1 << 5);
            }
            break;
        case TF_IMU_V3_FUNCTION_SET_ORIENTATION_CALLBACK_CONFIGURATION:
            if (response_expected) {
                imu_v3->response_expected[0] |= (1 << 6);
            } else {
                imu_v3->response_expected[0] &= ~(1 << 6);
            }
            break;
        case TF_IMU_V3_FUNCTION_SET_LINEAR_ACCELERATION_CALLBACK_CONFIGURATION:
            if (response_expected) {
                imu_v3->response_expected[0] |= (1 << 7);
            } else {
                imu_v3->response_expected[0] &= ~(1 << 7);
            }
            break;
        case TF_IMU_V3_FUNCTION_SET_GRAVITY_VECTOR_CALLBACK_CONFIGURATION:
            if (response_expected) {
                imu_v3->response_expected[1] |= (1 << 0);
            } else {
                imu_v3->response_expected[1] &= ~(1 << 0);
            }
            break;
        case TF_IMU_V3_FUNCTION_SET_QUATERNION_CALLBACK_CONFIGURATION:
            if (response_expected) {
                imu_v3->response_expected[1] |= (1 << 1);
            } else {
                imu_v3->response_expected[1] &= ~(1 << 1);
            }
            break;
        case TF_IMU_V3_FUNCTION_SET_ALL_DATA_CALLBACK_CONFIGURATION:
            if (response_expected) {
                imu_v3->response_expected[1] |= (1 << 2);
            } else {
                imu_v3->response_expected[1] &= ~(1 << 2);
            }
            break;
        case TF_IMU_V3_FUNCTION_SET_WRITE_FIRMWARE_POINTER:
            if (response_expected) {
                imu_v3->response_expected[1] |= (1 << 3);
            } else {
                imu_v3->response_expected[1] &= ~(1 << 3);
            }
            break;
        case TF_IMU_V3_FUNCTION_SET_STATUS_LED_CONFIG:
            if (response_expected) {
                imu_v3->response_expected[1] |= (1 << 4);
            } else {
                imu_v3->response_expected[1] &= ~(1 << 4);
            }
            break;
        case TF_IMU_V3_FUNCTION_RESET:
            if (response_expected) {
                imu_v3->response_expected[1] |= (1 << 5);
            } else {
                imu_v3->response_expected[1] &= ~(1 << 5);
            }
            break;
        case TF_IMU_V3_FUNCTION_WRITE_UID:
            if (response_expected) {
                imu_v3->response_expected[1] |= (1 << 6);
            } else {
                imu_v3->response_expected[1] &= ~(1 << 6);
            }
            break;
        default:
            return TF_E_INVALID_PARAMETER;
    }

    return TF_E_OK;
}

int tf_imu_v3_set_response_expected_all(TF_IMUV3 *imu_v3, bool response_expected) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    memset(imu_v3->response_expected, response_expected ? 0xFF : 0, 2);

    return TF_E_OK;
}

int tf_imu_v3_get_acceleration(TF_IMUV3 *imu_v3, int16_t *ret_x, int16_t *ret_y, int16_t *ret_z) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_ACCELERATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 6) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_x != NULL) { *ret_x = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_y != NULL) { *ret_y = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_z != NULL) { *ret_z = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 6) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_get_magnetic_field(TF_IMUV3 *imu_v3, int16_t *ret_x, int16_t *ret_y, int16_t *ret_z) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_MAGNETIC_FIELD, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 6) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_x != NULL) { *ret_x = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_y != NULL) { *ret_y = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_z != NULL) { *ret_z = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 6) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_get_angular_velocity(TF_IMUV3 *imu_v3, int16_t *ret_x, int16_t *ret_y, int16_t *ret_z) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_ANGULAR_VELOCITY, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 6) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_x != NULL) { *ret_x = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_y != NULL) { *ret_y = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_z != NULL) { *ret_z = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 6) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_get_temperature(TF_IMUV3 *imu_v3, int8_t *ret_temperature) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_get_orientation(TF_IMUV3 *imu_v3, int16_t *ret_heading, int16_t *ret_roll, int16_t *ret_pitch) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_ORIENTATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 6) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_heading != NULL) { *ret_heading = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_roll != NULL) { *ret_roll = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_pitch != NULL) { *ret_pitch = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 6) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_get_linear_acceleration(TF_IMUV3 *imu_v3, int16_t *ret_x, int16_t *ret_y, int16_t *ret_z) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_LINEAR_ACCELERATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 6) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_x != NULL) { *ret_x = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_y != NULL) { *ret_y = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_z != NULL) { *ret_z = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 6) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_get_gravity_vector(TF_IMUV3 *imu_v3, int16_t *ret_x, int16_t *ret_y, int16_t *ret_z) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_GRAVITY_VECTOR, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 6) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_x != NULL) { *ret_x = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_y != NULL) { *ret_y = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_z != NULL) { *ret_z = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 6) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_get_quaternion(TF_IMUV3 *imu_v3, int16_t *ret_w, int16_t *ret_x, int16_t *ret_y, int16_t *ret_z) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_QUATERNION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 8) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_w != NULL) { *ret_w = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_x != NULL) { *ret_x = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_y != NULL) { *ret_y = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
            if (ret_z != NULL) { *ret_z = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 8) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_get_all_data(TF_IMUV3 *imu_v3, int16_t ret_acceleration[3], int16_t ret_magnetic_field[3], int16_t ret_angular_velocity[3], int16_t ret_euler_angle[3], int16_t ret_quaternion[4], int16_t ret_linear_acceleration[3], int16_t ret_gravity_vector[3], int8_t *ret_temperature, uint8_t *ret_calibration_status) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_ALL_DATA, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 46) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_acceleration != NULL) { for (_i = 0; _i < 3; ++_i) ret_acceleration[_i] = tf_packet_buffer_read_int16_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 6); }
            if (ret_magnetic_field != NULL) { for (_i = 0; _i < 3; ++_i) ret_magnetic_field[_i] = tf_packet_buffer_read_int16_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 6); }
            if (ret_angular_velocity != NULL) { for (_i = 0; _i < 3; ++_i) ret_angular_velocity[_i] = tf_packet_buffer_read_int16_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 6); }
            if (ret_euler_angle != NULL) { for (_i = 0; _i < 3; ++_i) ret_euler_angle[_i] = tf_packet_buffer_read_int16_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 6); }
            if (ret_quaternion != NULL) { for (_i = 0; _i < 4; ++_i) ret_quaternion[_i] = tf_packet_buffer_read_int16_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 8); }
            if (ret_linear_acceleration != NULL) { for (_i = 0; _i < 3; ++_i) ret_linear_acceleration[_i] = tf_packet_buffer_read_int16_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 6); }
            if (ret_gravity_vector != NULL) { for (_i = 0; _i < 3; ++_i) ret_gravity_vector[_i] = tf_packet_buffer_read_int16_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 6); }
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_calibration_status != NULL) { *ret_calibration_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 46) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_save_calibration(TF_IMUV3 *imu_v3, bool *ret_calibration_done) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_SAVE_CALIBRATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_calibration_done != NULL) { *ret_calibration_done = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_set_sensor_configuration(TF_IMUV3 *imu_v3, uint8_t magnetometer_rate, uint8_t gyroscope_range, uint8_t gyroscope_bandwidth, uint8_t accelerometer_range, uint8_t accelerometer_bandwidth) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_imu_v3_get_response_expected(imu_v3, TF_IMU_V3_FUNCTION_SET_SENSOR_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_SET_SENSOR_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(imu_v3->tfp);

    _send_buf[0] = (uint8_t)magnetometer_rate;
    _send_buf[1] = (uint8_t)gyroscope_range;
    _send_buf[2] = (uint8_t)gyroscope_bandwidth;
    _send_buf[3] = (uint8_t)accelerometer_range;
    _send_buf[4] = (uint8_t)accelerometer_bandwidth;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_get_sensor_configuration(TF_IMUV3 *imu_v3, uint8_t *ret_magnetometer_rate, uint8_t *ret_gyroscope_range, uint8_t *ret_gyroscope_bandwidth, uint8_t *ret_accelerometer_range, uint8_t *ret_accelerometer_bandwidth) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_SENSOR_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_magnetometer_rate != NULL) { *ret_magnetometer_rate = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_gyroscope_range != NULL) { *ret_gyroscope_range = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_gyroscope_bandwidth != NULL) { *ret_gyroscope_bandwidth = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_accelerometer_range != NULL) { *ret_accelerometer_range = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_accelerometer_bandwidth != NULL) { *ret_accelerometer_bandwidth = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_set_sensor_fusion_mode(TF_IMUV3 *imu_v3, uint8_t mode) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_imu_v3_get_response_expected(imu_v3, TF_IMU_V3_FUNCTION_SET_SENSOR_FUSION_MODE, &_response_expected);
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_SET_SENSOR_FUSION_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(imu_v3->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_get_sensor_fusion_mode(TF_IMUV3 *imu_v3, uint8_t *ret_mode) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_SENSOR_FUSION_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_set_acceleration_callback_configuration(TF_IMUV3 *imu_v3, uint32_t period, bool value_has_to_change) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_imu_v3_get_response_expected(imu_v3, TF_IMU_V3_FUNCTION_SET_ACCELERATION_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_SET_ACCELERATION_CALLBACK_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(imu_v3->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_get_acceleration_callback_configuration(TF_IMUV3 *imu_v3, uint32_t *ret_period, bool *ret_value_has_to_change) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_ACCELERATION_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_set_magnetic_field_callback_configuration(TF_IMUV3 *imu_v3, uint32_t period, bool value_has_to_change) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_imu_v3_get_response_expected(imu_v3, TF_IMU_V3_FUNCTION_SET_MAGNETIC_FIELD_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_SET_MAGNETIC_FIELD_CALLBACK_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(imu_v3->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_get_magnetic_field_callback_configuration(TF_IMUV3 *imu_v3, uint32_t *ret_period, bool *ret_value_has_to_change) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_MAGNETIC_FIELD_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_set_angular_velocity_callback_configuration(TF_IMUV3 *imu_v3, uint32_t period, bool value_has_to_change) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_imu_v3_get_response_expected(imu_v3, TF_IMU_V3_FUNCTION_SET_ANGULAR_VELOCITY_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_SET_ANGULAR_VELOCITY_CALLBACK_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(imu_v3->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_get_angular_velocity_callback_configuration(TF_IMUV3 *imu_v3, uint32_t *ret_period, bool *ret_value_has_to_change) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_ANGULAR_VELOCITY_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_set_temperature_callback_configuration(TF_IMUV3 *imu_v3, uint32_t period, bool value_has_to_change) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_imu_v3_get_response_expected(imu_v3, TF_IMU_V3_FUNCTION_SET_TEMPERATURE_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_SET_TEMPERATURE_CALLBACK_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(imu_v3->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_get_temperature_callback_configuration(TF_IMUV3 *imu_v3, uint32_t *ret_period, bool *ret_value_has_to_change) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_TEMPERATURE_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_set_orientation_callback_configuration(TF_IMUV3 *imu_v3, uint32_t period, bool value_has_to_change) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_imu_v3_get_response_expected(imu_v3, TF_IMU_V3_FUNCTION_SET_ORIENTATION_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_SET_ORIENTATION_CALLBACK_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(imu_v3->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_get_orientation_callback_configuration(TF_IMUV3 *imu_v3, uint32_t *ret_period, bool *ret_value_has_to_change) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_ORIENTATION_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_set_linear_acceleration_callback_configuration(TF_IMUV3 *imu_v3, uint32_t period, bool value_has_to_change) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_imu_v3_get_response_expected(imu_v3, TF_IMU_V3_FUNCTION_SET_LINEAR_ACCELERATION_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_SET_LINEAR_ACCELERATION_CALLBACK_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(imu_v3->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_get_linear_acceleration_callback_configuration(TF_IMUV3 *imu_v3, uint32_t *ret_period, bool *ret_value_has_to_change) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_LINEAR_ACCELERATION_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_set_gravity_vector_callback_configuration(TF_IMUV3 *imu_v3, uint32_t period, bool value_has_to_change) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_imu_v3_get_response_expected(imu_v3, TF_IMU_V3_FUNCTION_SET_GRAVITY_VECTOR_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_SET_GRAVITY_VECTOR_CALLBACK_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(imu_v3->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_get_gravity_vector_callback_configuration(TF_IMUV3 *imu_v3, uint32_t *ret_period, bool *ret_value_has_to_change) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_GRAVITY_VECTOR_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_set_quaternion_callback_configuration(TF_IMUV3 *imu_v3, uint32_t period, bool value_has_to_change) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_imu_v3_get_response_expected(imu_v3, TF_IMU_V3_FUNCTION_SET_QUATERNION_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_SET_QUATERNION_CALLBACK_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(imu_v3->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_get_quaternion_callback_configuration(TF_IMUV3 *imu_v3, uint32_t *ret_period, bool *ret_value_has_to_change) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_QUATERNION_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_set_all_data_callback_configuration(TF_IMUV3 *imu_v3, uint32_t period, bool value_has_to_change) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_imu_v3_get_response_expected(imu_v3, TF_IMU_V3_FUNCTION_SET_ALL_DATA_CALLBACK_CONFIGURATION, &_response_expected);
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_SET_ALL_DATA_CALLBACK_CONFIGURATION, 5, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(imu_v3->tfp);

    period = tf_leconvert_uint32_to(period); memcpy(_send_buf + 0, &period, 4);
    _send_buf[4] = value_has_to_change ? 1 : 0;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_get_all_data_callback_configuration(TF_IMUV3 *imu_v3, uint32_t *ret_period, bool *ret_value_has_to_change) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_ALL_DATA_CALLBACK_CONFIGURATION, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 5) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_period != NULL) { *ret_period = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_value_has_to_change != NULL) { *ret_value_has_to_change = tf_packet_buffer_read_bool(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 5) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_get_spitfp_error_count(TF_IMUV3 *imu_v3, uint32_t *ret_error_count_ack_checksum, uint32_t *ret_error_count_message_checksum, uint32_t *ret_error_count_frame, uint32_t *ret_error_count_overflow) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_SPITFP_ERROR_COUNT, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 16) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_error_count_ack_checksum != NULL) { *ret_error_count_ack_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_message_checksum != NULL) { *ret_error_count_message_checksum = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_frame != NULL) { *ret_error_count_frame = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
            if (ret_error_count_overflow != NULL) { *ret_error_count_overflow = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 16) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_set_bootloader_mode(TF_IMUV3 *imu_v3, uint8_t mode, uint8_t *ret_status) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_SET_BOOTLOADER_MODE, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(imu_v3->tfp);

    _send_buf[0] = (uint8_t)mode;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_get_bootloader_mode(TF_IMUV3 *imu_v3, uint8_t *ret_mode) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_BOOTLOADER_MODE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_mode != NULL) { *ret_mode = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_set_write_firmware_pointer(TF_IMUV3 *imu_v3, uint32_t pointer) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_imu_v3_get_response_expected(imu_v3, TF_IMU_V3_FUNCTION_SET_WRITE_FIRMWARE_POINTER, &_response_expected);
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_SET_WRITE_FIRMWARE_POINTER, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(imu_v3->tfp);

    pointer = tf_leconvert_uint32_to(pointer); memcpy(_send_buf + 0, &pointer, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_write_firmware(TF_IMUV3 *imu_v3, const uint8_t data[64], uint8_t *ret_status) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_WRITE_FIRMWARE, 64, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(imu_v3->tfp);

    memcpy(_send_buf + 0, data, 64);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_status != NULL) { *ret_status = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_set_status_led_config(TF_IMUV3 *imu_v3, uint8_t config) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_imu_v3_get_response_expected(imu_v3, TF_IMU_V3_FUNCTION_SET_STATUS_LED_CONFIG, &_response_expected);
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_SET_STATUS_LED_CONFIG, 1, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(imu_v3->tfp);

    _send_buf[0] = (uint8_t)config;

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_get_status_led_config(TF_IMUV3 *imu_v3, uint8_t *ret_config) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_STATUS_LED_CONFIG, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 1) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_config != NULL) { *ret_config = tf_packet_buffer_read_uint8_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 1) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_get_chip_temperature(TF_IMUV3 *imu_v3, int16_t *ret_temperature) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_CHIP_TEMPERATURE, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 2) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_temperature != NULL) { *ret_temperature = tf_packet_buffer_read_int16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 2) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_reset(TF_IMUV3 *imu_v3) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_imu_v3_get_response_expected(imu_v3, TF_IMU_V3_FUNCTION_RESET, &_response_expected);
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_RESET, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_write_uid(TF_IMUV3 *imu_v3, uint32_t uid) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_imu_v3_get_response_expected(imu_v3, TF_IMU_V3_FUNCTION_WRITE_UID, &_response_expected);
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_WRITE_UID, 4, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(imu_v3->tfp);

    uid = tf_leconvert_uint32_to(uid); memcpy(_send_buf + 0, &uid, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_read_uid(TF_IMUV3 *imu_v3, uint32_t *ret_uid) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_READ_UID, 0, _response_expected);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 4) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { *ret_uid = tf_packet_buffer_read_uint32_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 4); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 4) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

int tf_imu_v3_get_identity(TF_IMUV3 *imu_v3, char ret_uid[8], char ret_connected_uid[8], char *ret_position, uint8_t ret_hardware_version[3], uint8_t ret_firmware_version[3], uint16_t *ret_device_identifier) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = imu_v3->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = true;
    tf_tfp_prepare_send(imu_v3->tfp, TF_IMU_V3_FUNCTION_GET_IDENTITY, 0, _response_expected);

    size_t _i;
    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(imu_v3->tfp, _response_expected, _deadline, &_error_code, &_length, TF_NEW_PACKET);

    if (_result < 0) {
        return _result;
    }


    if (_result & TF_TICK_PACKET_RECEIVED) {
        TF_PacketBuffer *_recv_buf = tf_tfp_get_receive_buffer(imu_v3->tfp);
        if (_error_code != 0 || _length != 25) {
            tf_packet_buffer_remove(_recv_buf, _length);
        } else {
            if (ret_uid != NULL) { tf_packet_buffer_pop_n(_recv_buf, (uint8_t *)ret_uid, 8);} else { tf_packet_buffer_remove(_recv_buf, 8); }
            if (ret_connected_uid != NULL) { tf_packet_buffer_pop_n(_recv_buf, (uint8_t *)ret_connected_uid, 8);} else { tf_packet_buffer_remove(_recv_buf, 8); }
            if (ret_position != NULL) { *ret_position = tf_packet_buffer_read_char(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 1); }
            if (ret_hardware_version != NULL) { for (_i = 0; _i < 3; ++_i) ret_hardware_version[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 3); }
            if (ret_firmware_version != NULL) { for (_i = 0; _i < 3; ++_i) ret_firmware_version[_i] = tf_packet_buffer_read_uint8_t(_recv_buf);} else { tf_packet_buffer_remove(_recv_buf, 3); }
            if (ret_device_identifier != NULL) { *ret_device_identifier = tf_packet_buffer_read_uint16_t(_recv_buf); } else { tf_packet_buffer_remove(_recv_buf, 2); }
        }
        tf_tfp_packet_processed(imu_v3->tfp);
    }


    if (_result & TF_TICK_TIMEOUT) {
        _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);
        (void) _result;
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(imu_v3->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 25) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}
#if TF_IMPLEMENT_CALLBACKS != 0
int tf_imu_v3_register_acceleration_callback(TF_IMUV3 *imu_v3, TF_IMUV3_AccelerationHandler handler, void *user_data) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    imu_v3->acceleration_handler = handler;
    imu_v3->acceleration_user_data = user_data;

    return TF_E_OK;
}


int tf_imu_v3_register_magnetic_field_callback(TF_IMUV3 *imu_v3, TF_IMUV3_MagneticFieldHandler handler, void *user_data) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    imu_v3->magnetic_field_handler = handler;
    imu_v3->magnetic_field_user_data = user_data;

    return TF_E_OK;
}


int tf_imu_v3_register_angular_velocity_callback(TF_IMUV3 *imu_v3, TF_IMUV3_AngularVelocityHandler handler, void *user_data) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    imu_v3->angular_velocity_handler = handler;
    imu_v3->angular_velocity_user_data = user_data;

    return TF_E_OK;
}


int tf_imu_v3_register_temperature_callback(TF_IMUV3 *imu_v3, TF_IMUV3_TemperatureHandler handler, void *user_data) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    imu_v3->temperature_handler = handler;
    imu_v3->temperature_user_data = user_data;

    return TF_E_OK;
}


int tf_imu_v3_register_linear_acceleration_callback(TF_IMUV3 *imu_v3, TF_IMUV3_LinearAccelerationHandler handler, void *user_data) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    imu_v3->linear_acceleration_handler = handler;
    imu_v3->linear_acceleration_user_data = user_data;

    return TF_E_OK;
}


int tf_imu_v3_register_gravity_vector_callback(TF_IMUV3 *imu_v3, TF_IMUV3_GravityVectorHandler handler, void *user_data) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    imu_v3->gravity_vector_handler = handler;
    imu_v3->gravity_vector_user_data = user_data;

    return TF_E_OK;
}


int tf_imu_v3_register_orientation_callback(TF_IMUV3 *imu_v3, TF_IMUV3_OrientationHandler handler, void *user_data) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    imu_v3->orientation_handler = handler;
    imu_v3->orientation_user_data = user_data;

    return TF_E_OK;
}


int tf_imu_v3_register_quaternion_callback(TF_IMUV3 *imu_v3, TF_IMUV3_QuaternionHandler handler, void *user_data) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    imu_v3->quaternion_handler = handler;
    imu_v3->quaternion_user_data = user_data;

    return TF_E_OK;
}


int tf_imu_v3_register_all_data_callback(TF_IMUV3 *imu_v3, TF_IMUV3_AllDataHandler handler, void *user_data) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    imu_v3->all_data_handler = handler;
    imu_v3->all_data_user_data = user_data;

    return TF_E_OK;
}
#endif
int tf_imu_v3_callback_tick(TF_IMUV3 *imu_v3, uint32_t timeout_us) {
    if (imu_v3 == NULL) {
        return TF_E_NULL;
    }

    if (imu_v3->magic != 0x5446 || imu_v3->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *hal = imu_v3->tfp->spitfp->hal;

    return tf_tfp_callback_tick(imu_v3->tfp, tf_hal_current_time_us(hal) + timeout_us);
}

#ifdef __cplusplus
}
#endif
