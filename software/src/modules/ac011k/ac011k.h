/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
 * Copyright (C) 2021-2022 Birger Schmidt <bs-warp@netgaroo.com>
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

//#include "bindings/bricklet_evse.h"

#include "config.h"
#include "device_module.h"
#include "web_server.h"
#include "ac011evse_v2.h"

//#define CHARGING_SLOT_COUNT 12
//#define CHARGING_SLOT_COUNT_SUPPORTED_BY_EVSE 20

// #define CHARGING_SLOT_INCOMING_CABLE 0
// #define CHARGING_SLOT_OUTGOING_CABLE 1
// #define CHARGING_SLOT_SHUTDOWN_INPUT 2
// #define CHARGING_SLOT_GP_INPUT 3
// #define CHARGING_SLOT_AUTOSTART_BUTTON 4
// #define CHARGING_SLOT_GLOBAL 5
// #define CHARGING_SLOT_USER 6
// #define CHARGING_SLOT_CHARGE_MANAGER 7
// #define CHARGING_SLOT_EXTERNAL 8
// #define CHARGING_SLOT_MODBUS_TCP 9
// #define CHARGING_SLOT_MODBUS_TCP_ENABLE 10
// #define CHARGING_SLOT_OCPP 11

// #define IEC_STATE_A 0
// #define IEC_STATE_B 1
// #define IEC_STATE_C 2
// #define IEC_STATE_D 3
// #define IEC_STATE_EF 4

// #define CHARGER_STATE_NOT_PLUGGED_IN 0
// #define CHARGER_STATE_WAITING_FOR_RELEASE 1
// #define CHARGER_STATE_READY_TO_CHARGE 2
// #define CHARGER_STATE_CHARGING 3
// #define CHARGER_STATE_ERROR 4

// #define DATA_STORE_PAGE_CHARGE_TRACKER 0
// #define DATA_STORE_PAGE_RECOVERY 15


#define PRIVCOMM_MAGIC      0
#define PRIVCOMM_VERSION    1
#define PRIVCOMM_ADDR       2
#define PRIVCOMM_CMD        3
#define PRIVCOMM_SEQ        4
#define PRIVCOMM_LEN        5
#define PRIVCOMM_PAYLOAD    6
#define PRIVCOMM_CRC        7

#define PayloadStart        8

#define GD_0A_TIME_ANSWER   0x02
#define GD_GET_RTC_ANSWER   0x10
#define GD_SET_RTC_ANSWER   0x14

//void evse_v2_button_recovery_handler();
//#define TF_ESP_PREINIT evse_v2_button_recovery_handler();

class AC011K {
public:
    AC011K(){};
    void pre_setup();
    void setup();
    void register_urls();
    void loop();
    void myloop();
    void register_my_urls();

    // Called in evse_v2_meter setup
    void update_all_data();

    //bool evse_found = false;
    bool initialized = false;

    byte sendSequenceNumber = 1;
    int transactionNumber = 100000;

    /* GD Firmware updater */
    bool firmware_update_running = false;

    /* void update_evse_state(); */
    /* void update_evse_low_level_state(); */
    /* void update_evse_max_charging_current(); */
    /* void update_evse_auto_start_charging(); */
    /* void update_evse_managed(); */
    /* void update_evse_user_calibration(); */
    /* void update_evse_charge_stats(); */
    /* bool is_in_bootloader(int rc); */
    /* bool flash_firmware(); */
    /* bool flash_plugin(int regular_plugin_upto); */
    /* bool wait_for_bootloader_mode(int mode); */

    //void setup_evse();
    void my_setup_evse();
    // String get_evse_debug_header();
    // String get_evse_debug_line();
    // void set_managed_current(uint16_t current);

    void set_user_current(uint16_t current);

    // void set_modbus_current(uint16_t current);
    // void set_modbus_enabled(bool enabled);

    // void set_ocpp_current(uint16_t current);
    // uint16_t get_ocpp_current();

    // bool apply_slot_default(uint8_t slot, uint16_t current, bool enabled, bool clear);
    // void apply_defaults();

    // bool debug = false;

    void log_hex_privcomm_line(byte *data);
    void Serial2write(byte *data, int size);
    void PrivCommSend(byte cmd, uint16_t datasize, byte *data, bool verbose = true);
    void PrivCommAck(byte cmd, byte *data);
    void sendCommand(byte *data, int datasize, byte sendSequenceNumber, bool verbose = true);
    void sendTime(byte cmd, byte action, byte len, byte sendSequenceNumber);
    void GetRTC();
    void SetRTC();
    void SetRTC(timeval time);
    void evse_slot_machine();
    void update_evseStatus(uint8_t evseStatus);
    time_t now();
    void fillTimeGdCommand(byte *datetime);
    uint16_t getPrivCommRxBufferUint16(uint16_t index);
    uint32_t getPrivCommRxBufferUint32(uint16_t index);

    #define PRIV_COMM_BUFFER_MAX_SIZE 1024
    byte PrivCommRxBuffer[PRIV_COMM_BUFFER_MAX_SIZE] = {'0'};
    byte PrivCommTxBuffer[PRIV_COMM_BUFFER_MAX_SIZE] = {0xFA, 0x03, 0x00, 0x00}; // Magic byte, Version, 16 bit Address - always the same

    int bs_evse_start_charging();
    int bs_evse_stop_charging();
    int bs_evse_set_charging_autostart(bool autostart);
    int bs_evse_set_max_charging_current(uint16_t max_current);
    int bs_evse_persist_config();

    const char* timeStr(byte *data);

    void sendChargingLimit1(uint8_t currentLimit, byte sendSequenceNumber);
    void sendChargingLimit2(uint8_t currentLimit, byte sendSequenceNumber);
    void sendChargingLimit3(uint8_t currentLimit, byte sendSequenceNumber);

    // ConfigRoot evse_config;
    ConfigRoot* evse_state;
    // ConfigRoot evse_hardware_configuration;
    ConfigRoot* evse_low_level_state;
    // ConfigRoot evse_energy_meter_values;
    // ConfigRoot evse_energy_meter_errors;
    // ConfigRoot evse_button_state;
    ConfigRoot* evse_slots;
    // ConfigRoot evse_max_charging_current;
    // ConfigRoot evse_indicator_led;
    // ConfigRoot evse_control_pilot_connected;
    // ConfigRoot evse_reset_dc_fault_current_state;
    // ConfigRoot evse_gpio_configuration;
    // ConfigRoot evse_gpio_configuration_update;
    // ConfigRoot evse_button_configuration;
    // ConfigRoot evse_button_configuration_update;
    // ConfigRoot evse_control_pilot_configuration;
    // ConfigRoot evse_control_pilot_configuration_update;
    // ConfigRoot evse_auto_start_charging;
    // ConfigRoot evse_auto_start_charging_update;
    // ConfigRoot evse_global_current;
    // ConfigRoot evse_global_current_update;
    // ConfigRoot evse_current_limit;
    // ConfigRoot evse_management_enabled;
    // ConfigRoot evse_management_enabled_update;
    // ConfigRoot evse_user_current;
    // ConfigRoot evse_user_enabled;
    // ConfigRoot evse_user_enabled_update;
    // ConfigRoot evse_external_enabled;
    // ConfigRoot evse_external_enabled_update;
    // ConfigRoot evse_external_defaults;
    // ConfigRoot evse_external_defaults_update;
    // ConfigRoot evse_management_current;
    // ConfigRoot evse_management_current_update;
    // ConfigRoot evse_external_current;
    // ConfigRoot evse_external_current_update;
    // ConfigRoot evse_external_clear_on_disconnect;
    // ConfigRoot evse_external_clear_on_disconnect_update;
    // ConfigRoot evse_modbus_enabled;
    // ConfigRoot evse_modbus_enabled_update;
    // ConfigRoot evse_ocpp_enabled;
    // ConfigRoot evse_ocpp_enabled_update;
    // ConfigRoot evse_managed;
    // ConfigRoot evse_managed_update;
    // ConfigRoot evse_managed_current;
    // ConfigRoot evse_privcomm;
    // uint32_t last_current_update = 0;
    // bool shutdown_logged = false;

    /* GD Firmware updater */
    bool handle_update_chunk(int command, WebServerRequest request, size_t chunk_index, uint8_t *data, size_t chunk_length);
    bool handle_gd_update_chunk(int command, WebServerRequest request, size_t chunk_index, uint8_t *data, size_t chunk_length, bool final, size_t complete_length);
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
