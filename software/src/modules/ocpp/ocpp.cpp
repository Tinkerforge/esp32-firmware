/* esp32-firmware
 * Copyright (C) 2022 Erik Fleckstein <erik@tinkerforge.com>
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

#include "ocpp.h"

#include "build.h"
#include "modules.h"
#include "api.h"
#include "task_scheduler.h"

extern API api;
extern TaskScheduler task_scheduler;

extern char local_uid_str[7];

void Ocpp::pre_setup()
{
    config = Config::Object({
        {"enable", Config::Bool(false)},
        {"url", Config::Str("", 0, 128)}
    });
#ifdef OCPP_STATE_CALLBACKS
    state = Config::Object({
        {"charge_point_state", Config::Uint8(0)},
        {"charge_point_status", Config::Uint8(0)},
        {"next_profile_eval", Config::Int32(0)},

        {"connector_state", Config::Uint8(0)},
        {"connector_status", Config::Uint8(0)},
        {"tag_id", Config::Str("", 0, 21)},
        {"parent_tag_id", Config::Str("", 0, 21)},
        {"tag_expiry_date", Config::Int32(0)},
        {"tag_timeout", Config::Uint32(0)},
        {"cable_timeout", Config::Uint32(0)},
        {"txn_id", Config::Int32(0)},
        {"txn_confirmed_time", Config::Int32(0)},
        {"txn_start_time", Config::Int32(0)},
        {"current", Config::Uint32(0)},
        {"txn_with_invalid_id", Config::Bool(false)},
        {"unavailable_requested", Config::Bool(false)},

        {"message_in_flight_type", Config::Uint8(0)},
        {"message_in_flight_id", Config::Int32(0)},
        {"message_in_flight_len", Config::Uint32(0)},
        {"message_timeout", Config::Uint32(0)},
        {"txn_msg_retry_timeout", Config::Uint32(0)},
        {"message_queue_depth", Config::Uint8(0)},
        {"status_queue_depth", Config::Uint8(0)},
        {"txn_msg_queue_depth", Config::Uint8(0)},
    });

    configuration = Config::Object({
        {"AuthorizeRemoteTxRequests", Config::Str("", 0, 500)},
        {"ClockAlignedDataInterval", Config::Str("", 0, 500)},
        {"ConnectionTimeOut", Config::Str("", 0, 500)},
        {"ConnectorPhaseRotation", Config::Str("", 0, 500)},
        {"ConnectorPhaseRotationMaxLength", Config::Str("", 0, 500)},
        {"GetConfigurationMaxKeys", Config::Str("", 0, 500)},
        {"HeartbeatInterval", Config::Str("", 0, 500)},
        {"LocalAuthorizeOffline", Config::Str("", 0, 500)},
        {"LocalPreAuthorize", Config::Str("", 0, 500)},
        {"MessageTimeout", Config::Str("", 0, 500)},
        {"MeterValuesAlignedData", Config::Str("", 0, 500)},
        {"MeterValuesAlignedDataMaxLength", Config::Str("", 0, 500)},
        {"MeterValuesSampledData", Config::Str("", 0, 500)},
        {"MeterValuesSampledDataMaxLength", Config::Str("", 0, 500)},
        {"MeterValueSampleInterval", Config::Str("", 0, 500)},
        {"NumberOfConnectors", Config::Str("", 0, 500)},
        {"ResetRetries", Config::Str("", 0, 500)},
        {"StopTransactionOnEVSideDisconnect", Config::Str("", 0, 500)},
        {"StopTransactionOnInvalidId", Config::Str("", 0, 500)},
        {"StopTransactionMaxMeterValues", Config::Str("", 0, 500)},
        {"StopTxnAlignedData", Config::Str("", 0, 500)},
        {"StopTxnAlignedDataMaxLength", Config::Str("", 0, 500)},
        {"StopTxnSampledData", Config::Str("", 0, 500)},
        {"StopTxnSampledDataMaxLength", Config::Str("", 0, 500)},
        {"SupportedFeatureProfiles", Config::Str("", 0, 500)},
        {"TransactionMessageAttempts", Config::Str("", 0, 500)},
        {"TransactionMessageRetryInterval", Config::Str("", 0, 500)},
        {"UnlockConnectorOnEVSideDisconnect", Config::Str("", 0, 500)},
        {"WebSocketPingInterval", Config::Str("", 0, 500)},
        {"ChargeProfileMaxStackLevel", Config::Str("", 0, 500)},
        {"ChargingScheduleAllowedChargingRateUnit", Config::Str("", 0, 500)},
        {"ChargingScheduleMaxPeriods", Config::Str("", 0, 500)},
        {"ConnectorSwitch3to1PhaseSupported", Config::Str("", 0, 500)},
        {"MaxChargingProfilesInstalled", Config::Str("", 0, 500)}
    });
#endif
    reset = Config::Null();
}

void Ocpp::setup()
{
    initialized = true;
    api.restorePersistentConfig("ocpp/config", &config);

    config_in_use = config;

    if (!config.get("enable")->asBool() || config.get("url")->asString().length() == 0)
        return;

    cp.start(config.get("url")->asCStr(), (String(BUILD_HOST_PREFIX) + '-' + local_uid_str).c_str());

    task_scheduler.scheduleWithFixedDelay([this](){
        cp.tick();
    }, 100, 100);
}

void Ocpp::register_urls()
{
    api.addPersistentConfig("ocpp/config", &config, {}, 1000);
#ifdef OCPP_STATE_CALLBACKS
    api.addState("ocpp/state", &state, {}, 1000);
    api.addState("ocpp/configuration", &configuration, {}, 1000);
#endif
    api.addCommand("ocpp/reset", &reset, {}, [](){
        remove_directory("/ocpp");
    }, true);
}

void Ocpp::loop()
{

}
