/* warp-charger
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
 * Copyright (C)      2021 Birger Schmidt <bs-warp@netgaroo.com>
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

#pragma once

#include "bindings/bricklet_evse.h"

#include "config.h"
#include "web_server.h"

class ENplus {
public:
    ENplus();
    void setup();
    void register_urls();
    void loop();

    bool evse_found = false;
    bool initialized = false;

    /* GD Firmware updater */
    bool firmware_update_running = false;

private:
    void setup_evse();
    void update_evse_state();
    void update_evse_low_level_state();
    void update_evse_max_charging_current();
    void update_evse_auto_start_charging();
    void update_evse_managed();
    void update_evse_user_calibration();
    void update_evse_charge_stats();
    bool is_in_bootloader(int rc);
    bool flash_firmware();
    bool flash_plugin(int regular_plugin_upto);
    bool wait_for_bootloader_mode(int mode);
    String get_evse_debug_header();
    String get_evse_debug_line();
    void set_managed_current(uint16_t current);
    String get_hex_privcomm_line(byte *data);
    void Serial2write(byte *data, int size);
    void PrivCommSend(byte cmd, uint16_t datasize, byte *data);
    void PrivCommAck(byte cmd, byte *data);
    void sendCommand(byte *data, int datasize, byte sendSequenceNumber);
    void sendTime(byte cmd, byte action, byte len, byte sendSequenceNumber);
    void sendTimeLong (byte sendSequenceNumber);
    void update_evseStatus(uint8_t evseStatus);

    #define PRIV_COMM_BUFFER_MAX_SIZE 1024
    byte PrivCommRxBuffer[PRIV_COMM_BUFFER_MAX_SIZE] = {'0'};
    byte PrivCommTxBuffer[PRIV_COMM_BUFFER_MAX_SIZE] = {0xFA, 0x03, 0x00, 0x00}; // Magic byte, Version, 16 bit Address - always the same
    char PrivCommHexBuffer[PRIV_COMM_BUFFER_MAX_SIZE*3] = {'0'};

    bool debug = false;

    int bs_evse_start_charging();
    int bs_evse_stop_charging();
    int bs_evse_set_charging_autostart(bool autostart);
    int bs_evse_set_max_charging_current(uint16_t max_current);
    int bs_evse_persist_config();
    int bs_evse_get_state(uint8_t *ret_iec61851_state, uint8_t *ret_vehicle_state, uint8_t *ret_contactor_state, uint8_t *ret_contactor_error, uint8_t *ret_charge_release, uint16_t *ret_allowed_charging_current, uint8_t *ret_error_state, uint8_t *ret_lock_state, uint32_t *ret_time_since_state_change, uint32_t *ret_uptime);

    const char* timeStr(byte *data, uint8_t offset);

    void sendChargingLimit1(time_t t, uint8_t currentLimit, byte sendSequenceNumber);
    void sendChargingLimit2(time_t t, uint8_t currentLimit, byte sendSequenceNumber);
    void sendChargingLimit3(time_t t, uint8_t currentLimit, byte sendSequenceNumber);

    ConfigRoot evse_config;
    ConfigRoot evse_state;
    ConfigRoot evse_hardware_configuration;
    ConfigRoot evse_low_level_state;
    ConfigRoot evse_max_charging_current;
    ConfigRoot evse_auto_start_charging;
    ConfigRoot evse_auto_start_charging_update;
    ConfigRoot evse_current_limit;
    ConfigRoot evse_stop_charging;
    ConfigRoot evse_start_charging;
    ConfigRoot evse_managed;
    ConfigRoot evse_managed_update;
    ConfigRoot evse_managed_current;
    ConfigRoot evse_user_calibration;
    ConfigRoot evse_privcomm;
    uint32_t last_current_update = 0;
    bool shutdown_logged = false;

    /* GD Firmware updater */
    bool handle_update_chunk(int command, WebServerRequest request, size_t chunk_index, uint8_t *data, size_t chunk_length, bool final, size_t complete_length);
    bool handle_update_chunk1(int command, WebServerRequest request, size_t chunk_index, uint8_t *data, size_t chunk_length, bool final, size_t complete_length);
    bool handle_update_chunk2(int command, WebServerRequest request, size_t chunk_index, uint8_t *data, size_t chunk_length, bool final, size_t complete_length);
    uint32_t calculated_checksum = 0;
    uint32_t checksum = 0;
    uint32_t checksum_offset = 0;
    bool update_aborted = false;

};

static const uint16_t crc16_modbus_table[] = {
    0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
    0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
    0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
    0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
    0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
    0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
    0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
    0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
    0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
    0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
    0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
    0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
    0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
    0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
    0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
    0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
    0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
    0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
    0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
    0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
    0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
    0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
    0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
    0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
    0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
    0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,
    0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
    0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
    0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
    0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
    0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
    0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040
};

// evse GD status strings
static const char *evse_status_text[] = {
   "undefined",
   "Available (not engaged)",
   "Preparing (engaged, not started)",
   "Charging (charging ongoing, power output)",
   "Suspended by charger (started but no power available)",
   "Suspended by EV (power available but waiting for the EV response)",
   "Finishing, charging acomplished (RFID stop or EMS control stop)",
   "(Reserved)",
   "(Unavailable)",
   "Fault (charger in fault condition)",
};

static const char *stop_reason_text[] = {
   "undefined",
   "Remote",
   "undefined",
   "EV disconnected",
};

static const char *cmd_0B_text[] = {
    "Reset into application mode",
    "Handshake",
    "Flash erase",
    "Flash write",
    "Flash verify",
    "Reset into boot mode",
};

