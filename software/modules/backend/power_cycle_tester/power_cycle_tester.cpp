/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "power_cycle_tester.h"

#include "bindings/errors.h"

#include "api.h"
#include "event_log.h"
#include "tools.h"
#include "task_scheduler.h"
#include "modules.h"

#include "bindings/bricklet_rotary_poti_v2.h"

#include "bindings/base58.h"
#include "bindings/endian_convert.h"
#include "bindings/errors.h"

#include <string.h>


extern EventLog logger;

extern TF_HAL hal;
extern TaskScheduler task_scheduler;

extern API api;

PowerCycleTester::PowerCycleTester() : DeviceModule("industrial_dual_relay", "Industrial Dual Relay", "Power Cycle Tester", std::bind(&PowerCycleTester::setup_industrial_dual_relay, this))
{

}

void PowerCycleTester::setup_industrial_dual_relay()
{
    if (!this->DeviceModule::setup_device()) {
        return;
    }

    initialized = true;
}

TF_RotaryPotiV2 poti;

void PowerCycleTester::setup()
{
    logger.printfln("Waiting for 2 seconds...");
    delay(2000);

    setup_industrial_dual_relay();
    if (!device_found)
        return;

    uint16_t device_id = 2140;
    TF_TFP *tfp = tf_hal_get_tfp(&hal, nullptr, nullptr, &device_id, false);

    device_found = true;

    char uid[7] = {0};

    tf_base58_encode(tfp->uid, uid);
    int result = tf_rotary_poti_v2_create(&poti, uid, &hal);

    if (result != TF_E_OK) {
        logger.printfln("Failed to initialize Rotary Poti 2.0 Bricklet (%d).", device_name, result);
    }

    tf_industrial_dual_relay_set_status_led_config(&this->device, 1);
    tf_rotary_poti_v2_set_status_led_config(&poti, 1);
}

void PowerCycleTester::register_urls()
{
    return;
    this->DeviceModule::register_urls();
}

void PowerCycleTester::loop()
{
    this->DeviceModule::loop();
}

#define TF_INDUSTRIAL_DUAL_RELAY_FUNCTION_SET_DELAYED_MONOFLOP 42

int tf_industrial_dual_relay_set_delayed_monoflop(TF_IndustrialDualRelay *industrial_dual_relay, uint8_t channel, bool value, uint32_t time, uint32_t delay) {
    if (industrial_dual_relay == NULL) {
        return TF_E_NULL;
    }

    if (industrial_dual_relay->magic != 0x5446 || industrial_dual_relay->tfp == NULL) {
        return TF_E_NOT_INITIALIZED;
    }

    TF_HAL *_hal = industrial_dual_relay->tfp->spitfp->hal;

    if (tf_hal_get_common(_hal)->locked) {
        return TF_E_LOCKED;
    }

    bool _response_expected = false;
    tf_tfp_prepare_send(industrial_dual_relay->tfp, TF_INDUSTRIAL_DUAL_RELAY_FUNCTION_SET_DELAYED_MONOFLOP, 10, _response_expected);

    uint8_t *_send_buf = tf_tfp_get_send_payload_buffer(industrial_dual_relay->tfp);

    _send_buf[0] = (uint8_t)channel;
    _send_buf[1] = value ? 1 : 0;
    time = tf_leconvert_uint32_to(time); memcpy(_send_buf + 2, &time, 4);
    delay = tf_leconvert_uint32_to(delay); memcpy(_send_buf + 6, &delay, 4);

    uint32_t _deadline = tf_hal_current_time_us(_hal) + tf_hal_get_common(_hal)->timeout;

    uint8_t _error_code = 0;
    uint8_t _length = 0;
    int _result = tf_tfp_send_packet(industrial_dual_relay->tfp, _response_expected, _deadline, &_error_code, &_length);

    if (_result < 0) {
        return _result;
    }

    if (_result & TF_TICK_TIMEOUT) {
        return TF_E_TIMEOUT;
    }

    _result = tf_tfp_finish_send(industrial_dual_relay->tfp, _result, _deadline);

    if (_error_code == 0 && _length != 0) {
        return TF_E_WRONG_RESPONSE_LENGTH;
    }

    if (_result < 0) {
        return _result;
    }

    return tf_tfp_get_error(_error_code);
}

void PowerCycleTester::trigger_on_poti_pos(POTI_POS poti_pos, uint32_t delay)
{
    int16_t pos = 0;
    if (tf_rotary_poti_v2_get_position(&poti, &pos) != TF_E_OK) {
        logger.printfln("Failed to get poti position");
        return;
    }

    int16_t low = 0, high = 0;

    switch (poti_pos) {
        case POTI_POS::MINUS_150:
            low = -200;
            high = -120;
            break;
        case POTI_POS::MINUS_90:
            low = -120;
            high = -67;
            break;
        case POTI_POS::MINUS_45:
            low = -67;
            high = -23;
            break;
        case POTI_POS::ZERO:
            low = -23;
            high = 23;
            break;
        case POTI_POS::PLUS_45:
            low = 23;
            high = 67;
            break;
        case POTI_POS::PLUS_90:
            low = 67;
            high = 120;
            break;
        case POTI_POS::PLUS_150:
            low = 120;
            high = 200;
            break;
    }

    if (pos >= low && pos <= high) {
        logger.printfln("Triggering power cycle in %u ms.", delay);
        check(tf_industrial_dual_relay_set_delayed_monoflop(&this->device, 0, true, 3000, delay), "delayed monoflop");
        //tf_industrial_dual_relay_set_monoflop(&this->device, 0, false, delay);
    }
}
