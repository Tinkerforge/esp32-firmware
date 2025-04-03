/* esp32-firmware
 * Copyright (C) 2025 Olaf Lüke <olaf@tinkerforge.com>
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

// SLAC implementation for HomePlug PLC according to ISO 15118-3:2016

// Some parts that were not clear from the spec are loosly based
// on the libslac and pyPLC open source implementations:
// * https://github.com/uhi22/pyPLC
// * https://github.com/EVerest/libslac

#pragma once

#include "module.h"
#include "config.h"

#define SLAC_ETHERNET_FRAME_LENGTH_MAX 1518

// Data lengths
#define SLAC_STATION_ID_LENGTH 17
#define SLAC_RUN_ID_LENGTH 8
#define SLAC_MAC_ADDRESS_LENGTH 6
#define SLAC_SOUND_TARGET_LENGTH 6
#define SLAC_AAG_LIST_LENGTH 58
#define SLAC_NID_LENGTH 7
#define SLAC_NMK_LENGTH 16

// CMs
#define SLAC_MMTYPE_CM_SET_KEY          0x6008
#define SLAC_MMTYPE_CM_SLAC_PARM        0x6064
#define SLAC_MMTYPE_CM_START_ATTEN_CHAR 0x6068
#define SLAC_MMTYPE_CM_ATTEN_CHAR       0x606C
#define SLAC_MMTYPE_CM_MNBC_SOUND       0x6074
#define SLAC_MMTYPE_CM_VALIDATE         0x6078
#define SLAC_MMTYPE_CM_SLAC_MATCH       0x607C
#define SLAC_MMTYPE_CM_ATTEN_PROFILE    0x6084

// Modes
#define SLAC_MMTYPE_MODE_REQUEST      0x0000
#define SLAC_MMTYPE_MODE_CONFIRMATION 0x0001
#define SLAC_MMTYPE_MODE_INDICATION   0x0002
#define SLAC_MMTYPE_MODE_RESPONSE     0x0003
#define SLAC_MMTYPE_MODE_MASK         0x0003

// Qualcomm specific vendor MMEs
#define SLAC_MMTYPE_QUALCOMM_CM_RESET_DEVICE 0xA01C
#define SLAC_MMTYPE_QUALCOMM_LINK_STATUS     0xA0B8
#define SLAC_MMTYPE_QUALCOMM_OP_ATTR         0xA068
#define SLAC_MMTYPE_QUALCOMM_NW_INFO         0xA038
#define SLAC_MMTYPE_QUALCOMM_GET_SW          0xA000

#define SLAC_ETHERNET_TYPE_HOMEPLUG 0x88E1
#define SLAC_ETHERNET_TYPE_IPV6     0x86DD

#define SLAC_ATTENUATION_PROFILE_SOUNDS 10

// TP seems to be "performance time": Time the EV/EVSE has to react to a message
// TT seems to be "timeout time":     Time the EV/EVSE has to wait for the next message
// Constants and Timings (in ms) ISO 15118-3 A.8 [V2G3-A08-01] Table A.1
#define SLAC_C_EV_MATCH_MNBC            10
#define SLAC_C_EV_MATCH_RETRY            2
#define SLAC_C_EV_START_ATTEN_CHAR_INDS  3
#define SLAC_TP_AMP_MAP_EXCHANGE             100_ms
#define SLAC_TP_EV_BATCH_MSG_INTERVAL_MIN     20_ms
#define SLAC_TP_EV_BATCH_MSG_INTERVAL_MAX     50_ms
#define SLAC_TP_EV_MATCH_SESSION             500_ms
#define SLAC_TP_EV_SLAC_INIT                  10_s
#define SLAC_TP_EVSE_AVG_ATTEN_CALC          100_ms
#define SLAC_TP_LINK_READY_NOTIFCATION_MIN   200_ms
#define SLAC_TP_LINK_READY_NOTIFCATION_MAX  1000_ms
#define SLAC_TP_MATCH_RESPONSE               100_ms
#define SLAC_TP_MATCH_SEQUENCE               100_ms
#define SLAC_TT_AMP_AMP_EXCHANGE             200_ms
#define SLAC_TT_EV_ATTEN_RESULTS            1200_ms
#define SLAC_TT_EVSE_MATCH_MNBC              600_ms
#define SLAC_TT_EVSE_MATCH_SESSION            10_s
#define SLAC_TT_EVSE_SLAC_INIT_MIN            20_s
#define SLAC_TT_EVSE_SLAC_INIT_MAX            50_s
#define SLAC_TT_MATCH_JOIN                    12_s
#define SLAC_TT_MATCH_RESPONSE               200_ms
#define SLAC_TT_MATCH_SEQUENCE               400_ms
#define SLAC_TT_MATCHING_REPETITION           10_s
#define SLAC_TT_MACHTING_RATE                400_ms

// Constants and Timings (in ms) ISO 15118-3 8 [V2G3-M08-01] Table 3
#define SLAC_C_CONN_MAX_MATCH           3
#define SLAC_C_EV_VALID_NB_TOGGLES_MIN  1
#define SLAC_C_EV_VALID_NB_TOGGLES_MAX  3
#define SLAC_C_EVSE_MATCH_PARALLEL      5
#define SLAC_C_SEQU_RETRY               2
#define SLAC_T_CONN_INIT_HLC                 200_ms
#define SLAC_T_CONN_RESUME                     6_s
#define SLAC_T_CONN_MAX_COMM                   8_s
#define SLAC_T_CONN_RESETUP                   15_s
#define SLAC_T_VALID_DETECT_TIME             200_ms
#define SLAC_TP_EV_VALID_STATE_DURATION_MIN  200_ms
#define SLAC_TP_EV_VALID_STATE_DURATION_MAX  400_ms
#define SLAC_TP_EV_VALID_TOGGLE_MIN          600_ms
#define SLAC_TP_EV_VALID_TOGGLE_MAX         3500_ms
#define SLAC_TT_EVSE_VALID_TOGGLE           3500_ms
#define SLAC_TP_MATCH_LEAVE                 1000_ms
#define SLAC_T_STEP_EF                         4_s

struct [[gnu::packed]] SLAC_HomeplugMessageHeaderV0 {
    // Ethernet Header
    uint8_t  destination_mac[6];
    uint8_t  source_mac[6];
    uint16_t ethernet_type;

    // Management message version and type
    uint8_t  mm_version;
    uint16_t mm_type;
};

struct [[gnu::packed]] SLAC_HomeplugMessageHeader {
    // Ethernet Header
    uint8_t  destination_mac[6];
    uint8_t  source_mac[6];
    uint16_t ethernet_type;

    // Management message version and type
    uint8_t  mm_version;
    uint16_t mm_type;

    // Fragmentation management number information and message sequence number.
    // Hint: If mm_version is 0, the fmni and fmsn are not present.
    //       It is 0 in the vendor specific qualcomm messages.
    uint8_t fmni;
    uint8_t fmsn;
};


struct [[gnu::packed]] CM_SLACParmRequest {
    SLAC_HomeplugMessageHeader header;
    uint8_t application_type;           // fixed to 0x00, indicating 'pev-evse matching'
    uint8_t security_type;              // fixed to 0x00, indicating 'no security'
    uint8_t run_id[SLAC_RUN_ID_LENGTH]; // indentifier for a matching run
    // cipher fields are missing, because we restrict to security_type = 0x00
};

struct [[gnu::packed]] CM_SLACParmConfirmation {
    SLAC_HomeplugMessageHeader header;
    uint8_t m_sound_target[SLAC_SOUND_TARGET_LENGTH];     // fixed to 0xFFFFFFFFFFFF
    uint8_t num_sounds = SLAC_ATTENUATION_PROFILE_SOUNDS; // number of expected m-sounds (10 is typical)
    uint8_t timeout = 6;                                  // corresponds to TT_EVSE_match_MNBC, in units of 100ms (600ms is typical)
    uint8_t resp_type = 0x01;                             // fixed to 0x01, indicating 'other gp station'
    uint8_t forwarding_sta[SLAC_MAC_ADDRESS_LENGTH];      // ev host mac address
    uint8_t application_type = 0x00;                      // fixed to 0x00, indicating 'pev-evse matching'
    uint8_t security_type = 0x00;                         // fixed to 0x00, indicating 'no security'
    uint8_t run_id[SLAC_RUN_ID_LENGTH];                   // matching run identifier, corresponding to the request
    uint8_t padding[16] = {0};                            // padding to 60 bytes
    // cipher field is missing, because we restrict to security_type = 0x00
};

struct [[gnu::packed]] CM_StartAttenCharIndication {
    SLAC_HomeplugMessageHeader header;
    uint8_t application_type;                        // fixed to 0x00, indicating 'pev-evse matching'
    uint8_t security_type;                           // fixed to 0x00, indicating 'no security'
    uint8_t num_sounds;                              // number of expected m-sounds
    uint8_t timeout;                                 // corresponds to TT_EVSE_match_MNBC
    uint8_t resp_type;                               // fixed to 0x01, indicating 'other gp station'
    uint8_t forwarding_sta[SLAC_MAC_ADDRESS_LENGTH]; // ev host mac address
    uint8_t run_id[SLAC_RUN_ID_LENGTH];              // indentifier for a matching run
};

struct [[gnu::packed]] CM_AttenCharIndication{
    SLAC_HomeplugMessageHeader header;
    uint8_t application_type = 0x00;                      // fixed to 0x00, indicating 'pev-evse matching'
    uint8_t security_type = 0x00;                         // fixed to 0x00, indicating 'no security'
    uint8_t source_address[SLAC_MAC_ADDRESS_LENGTH];      // mac address of EV host, which initiates matching
    uint8_t run_id[SLAC_RUN_ID_LENGTH];                   // indentifier for a matching run
    uint8_t source_id[SLAC_STATION_ID_LENGTH] = {0};      // always all 0
    uint8_t resp_id[SLAC_STATION_ID_LENGTH] = {0};        // always all 0
    uint8_t num_sounds;                                   // number of sounds used for attenuation profile
    struct [[gnu::packed]] {
        uint8_t num_groups = SLAC_AAG_LIST_LENGTH;        // number of OFDM carrier groups
        uint8_t aag[SLAC_AAG_LIST_LENGTH];                // AAG_1 .. AAG_N
    } attenuation_profile;
};

struct [[gnu::packed]] CM_AttenCharResponse {
    SLAC_HomeplugMessageHeader header;
    uint8_t application_type;                        // fixed to 0x00, indicating 'pev-evse matching'
    uint8_t security_type;                           // fixed to 0x00, indicating 'no security'
    uint8_t source_address[SLAC_MAC_ADDRESS_LENGTH]; // mac address of EV host, which initiates matching
    uint8_t run_id[SLAC_RUN_ID_LENGTH];              // indentifier for a matching run
    uint8_t source_id[SLAC_STATION_ID_LENGTH];       // unique id of the station, that sent the m-sounds
    uint8_t resp_id[SLAC_STATION_ID_LENGTH];         // unique id of the station, that is sending this message
    uint8_t result;                                  // fixed to 0x00, indicates successful SLAC process
};

struct [[gnu::packed]] CM_MNBCSoundIndication {
    SLAC_HomeplugMessageHeader header;
    uint8_t application_type;                  // fixed to 0x00, indicating 'pev-evse matching'
    uint8_t security_type;                     // fixed to 0x00, indicating 'no security'
    uint8_t sender_id[SLAC_STATION_ID_LENGTH]; // fixed to 0x00
    uint8_t remaining_sound_count;             // count of remaining sound messages
    uint8_t run_id[SLAC_RUN_ID_LENGTH];        // indentifier for a matching run
    uint8_t _reserved[8];                      // note: this is to pad the run_id, which is defined to be 16 bytes for this message
    uint8_t random[16];                        // random value
};

// note: this message doesn't seem to part of hpgp, it is defined in ISO15118-3
struct [[gnu::packed]] CM_AttenProfileIndication {
    SLAC_HomeplugMessageHeader header;
    uint8_t pev_mac[SLAC_MAC_ADDRESS_LENGTH]; // mac address of the EV host
    uint8_t num_groups;                       // number of OFDM carrier groups
    uint8_t _reserved;
    uint8_t aag[SLAC_AAG_LIST_LENGTH];        // list of average attenuation for each group
};

struct [[gnu::packed]] CM_SLACMatchRequest {
    SLAC_HomeplugMessageHeader header;
    uint8_t application_type;                  // fixed to 0x00, indicating 'pev-evse matching'
    uint8_t security_type;                     // fixed to 0x00, indicating 'no security'
    uint16_t mvf_length;                       // fixed to 0x3e = 62 bytes following
    uint8_t pev_id[SLAC_STATION_ID_LENGTH];    // fixed to 0x00
    uint8_t pev_mac[SLAC_MAC_ADDRESS_LENGTH];  // mac address of the EV host
    uint8_t evse_id[SLAC_STATION_ID_LENGTH];   // EVSE id
    uint8_t evse_mac[SLAC_MAC_ADDRESS_LENGTH]; // mac address of the EVSE
    uint8_t run_id[SLAC_RUN_ID_LENGTH];        // indentifier for a matching run
    uint8_t _reserved[8];                      // note: this is to pad the run_id, which is defined to be 16 bytes for this message
};

struct [[gnu::packed]] CM_SLACMatchConfirmation {
    SLAC_HomeplugMessageHeader header;
    uint8_t application_type = 0x00;            // fixed to 0x00, indicating 'pev-evse matching'
    uint8_t security_type = 0x00;               // fixed to 0x00, indicating 'no security'
    uint16_t mvf_length = 0x56;                 // fixed to 0x56 = 86 bytes following
    uint8_t pev_id[SLAC_STATION_ID_LENGTH];     // fixed to 0x00
    uint8_t pev_mac[SLAC_MAC_ADDRESS_LENGTH];   // mac address of the EV host
    uint8_t evse_id[SLAC_STATION_ID_LENGTH];    // EVSE id
    uint8_t evse_mac[SLAC_MAC_ADDRESS_LENGTH];  // mac address of the EVSE
    uint8_t run_id[SLAC_RUN_ID_LENGTH];         // indentifier for a matching run
    uint8_t _rerserved[8];                      // note: this is to pad the run_id, which is defined to be 16 bytes for this message
    uint8_t nid[SLAC_NID_LENGTH];               // network id derived from the nmk
    uint8_t _reserved2;                         // note: this is to pad the nid, which is defined to be 8 bytes for this message
    uint8_t nmk[SLAC_NMK_LENGTH];               // private nmk of the EVSE
};

struct [[gnu::packed]] CM_ValidateRequest {
    SLAC_HomeplugMessageHeader header;
    uint8_t signal_type; // fixed to 0x00: PEV S2 toggles on control pilot line
    uint8_t timer;       // in the first request response exchange: should be set to 0x00
                         // in the second request response exchange: 0x00 = 100ms, 0x01 = 200ms TT_EVSE_vald_toggle
    uint8_t result;      // in the first request response exchange: should be set to 0x01 = ready
                         // in the second request response exchange: should be set to 0x01 = ready
};

struct [[gnu::packed]] CM_ValidateConfirmation {
    SLAC_HomeplugMessageHeader header;
    uint8_t signal_type; // fixed to 0x00: PEV S2 toggles on control pilot line
    uint8_t toggle_num;  // in the first request response exchange: should be set to 0x00
                         // in the second request response exchange: number of detected BC
                         // edges during TT_EVSE_vald_toggle
    uint8_t result;      // 0x00 = not ready, 0x01 = ready, 0x02 = success, 0x03 = failure, 0x04 = not required
};

struct [[gnu::packed]] CM_SetKeyRequest {
    SLAC_HomeplugMessageHeader header;
    uint8_t  key_type = 0x01;         // fixed to 0x01, indicating NMK
    uint32_t my_nonce = 0x00000000;   // fixed to 0x00000000: encrypted payload not used
    uint32_t your_nonce = 0x00000000; // fixed to 0x00000000: encrypted payload not used
    uint8_t  pid = 0x04;              // fixed to 0x04: HLE protocol
    uint16_t prn = 0x0000;            // fixed to 0x0000: encrypted payload not used
    uint8_t  pmn = 0x00;              // fixed to 0x00: encrypted payload not used
    uint8_t  cco_capability = 0x00;   // CCo capability according to the station role
    uint8_t  nid[SLAC_NID_LENGTH];    // 54 LSBs = NID, 2 MSBs = 0b00
    uint8_t  new_eks = 0x01;          // fixed to 0x01: NMK
    uint8_t  nmk[SLAC_NMK_LENGTH];    // new random NMK
    uint8_t  padding[3] = {0};        // padding to 60 bytes
};

struct [[gnu::packed]] CM_SetKeyConfirmation {
    SLAC_HomeplugMessageHeader header;
    uint8_t result; // 0x00 = success, 0x01 = failure, 0x02 - 0xFF = reserved (Note: 0x01 is expected if used with local modem)
    uint32_t my_nonce;
    uint32_t your_nonce;
    uint8_t pid;
    uint16_t prn;
    uint8_t pmn;
    uint8_t cco_capability;
};

struct [[gnu::packed]] CM_QualcommGetSwRequest {
    SLAC_HomeplugMessageHeaderV0 header;
    uint8_t vendor_mme[3] = {0x00, 0xB0, 0x52}; // Qualcomm vendor MME
    uint8_t padding[40] = {0};                  // padding to 60 bytes
};

struct [[gnu::packed]] CM_QualcommGetSwConfirmation {
    SLAC_HomeplugMessageHeaderV0 header;
    uint8_t vendor_mme[3];
    uint8_t status;
    uint8_t device;
    uint8_t version_length;
    uint8_t version[0x80];
};

struct [[gnu::packed]] CM_QualcommLinkStatusRequest {
    SLAC_HomeplugMessageHeaderV0 header;
    uint8_t vendor_mme[3] = {0x00, 0xB0, 0x52}; // Qualcomm vendor MME
    uint8_t padding[40] = {0};                  // padding to 60 bytes
};

struct [[gnu::packed]] CM_QualcommLinkStatusConfirmation {
    SLAC_HomeplugMessageHeaderV0 header;
    uint8_t vendor_mme[3];
    uint8_t reserved;
    uint8_t link_status;
};

struct [[gnu::packed]] CM_QualcommOpAttrRequest {
    SLAC_HomeplugMessageHeaderV0 header;
    uint8_t vendor_mme[3] = {0x00, 0xB0, 0x52}; // Qualcomm vendor MME
    uint32_t cookie = 0xC00C13;                 // cookie
    uint8_t report_type = 0;                    // report type binary
    uint8_t padding[35] = {0};                  // padding to 60 bytes
};

struct [[gnu::packed]] CM_QualcommOpAttrConfirmation {
    SLAC_HomeplugMessageHeaderV0 header;
    uint8_t vendor_mme[3] = {0x00, 0xB0, 0x52}; // Qualcomm vendor MME
    uint16_t success;    // 0x00 means success
    uint32_t cookie;
    uint8_t report_type; // should be 0x00 (binary)
    uint16_t size;       // should be 118
    uint8_t hw_platform[16];
    uint8_t sw_platform[16];
    uint32_t version_major;
    uint32_t version_minor;
    uint32_t version_pib;
    uint32_t version_build;
    uint32_t reserved;
    uint8_t build_date[8];
    uint8_t release_type[12];
    uint8_t sdram_type;
    uint8_t reserved2;
    uint8_t line_freq_zc;
    uint32_t sdram_size;
    uint8_t authorization_mode;
};


class SLAC final
{
public:
    enum class State : uint8_t {
        ModemReset,
        ModemInitialization,
        CMSetKeyRequest,
        WaitForCMSetKeyConfirmation,
        CMQualcommGetSwRequest,
        WaitForCMQualcommGetSwResponse,
        CMQualcommLinkStatusRequest,
        WaitForCMQualcommLinkStatusResponse,
        CMQualcommOpAttrRequest,
        WaitForCMQualcommOpAttrResponse,
        WaitForSlacParamRequest,
        WaitForStartAttenCharIndication,
        WaitForMNBCSound,
        WaitForAttenChar,
        WaitForSlacMatch,
        WaitForSDP,
        LinkDetected,
    };
    static const char *state_to_string(const State state);

    SLAC(){}
    State state = State::ModemInitialization;
    void state_machine_loop();

    void pre_setup();
    ConfigRoot api_state;

private:
    void fill_header_v0(SLAC_HomeplugMessageHeaderV0 *header, const uint8_t *destination_mac, const uint8_t *source_mac, const uint16_t mm_type);
    void fill_header(SLAC_HomeplugMessageHeader *header, const uint8_t *destination_mac, const uint8_t *source_mac, const uint16_t mm_type);
    void generate_nid_from_nmk(void);
    void generate_new_nmk_and_nid(void);

    void handle_modem_reset(void);
    void handle_modem_initialization(void);
    void handle_cm_set_key_request(void);
    void handle_cm_set_key_confirmation(const CM_SetKeyConfirmation &cm_set_key_confirmation);
    void handle_cm_slac_parm_request(const CM_SLACParmRequest &cm_slac_parm_request);
    void handle_cm_start_atten_char_indication(const CM_StartAttenCharIndication &cm_start_atten_char_indication);
    void handle_cm_mnbc_sound_indication(const CM_MNBCSoundIndication &cm_mnbc_sound_indication);
    void handle_cm_atten_profile_indication(const CM_AttenProfileIndication &cm_atten_profile_indication);
    void handle_cm_atten_char_response(const CM_AttenCharResponse &cm_atten_char_response);
    void handle_cm_slac_match_request(const CM_SLACMatchRequest &cm_slac_match_request);

    void handle_cm_qualcomm_get_sw_request();
    void handle_cm_qualcomm_link_status_request();
    void handle_cm_qualcomm_op_attr_request();
    void handle_cm_qualcomm_get_sw_confirmation(const CM_QualcommGetSwConfirmation &cm_qualcomm_get_sw_confirmation);
    void handle_cm_qualcomm_link_status_confirmation(const CM_QualcommLinkStatusConfirmation &cm_qualcomm_link_status_confirmation);
    void handle_cm_qualcomm_op_attr_confirmation(const CM_QualcommOpAttrConfirmation &cm_qualcomm_op_attr_confirmation);

    void poll_modem(void);

    Option<micros_t> next_timeout = {};

    // for CM_ATTEN_PROFILE.IND
    uint16_t aag_list[SLAC_AAG_LIST_LENGTH] = {0};

    uint8_t nmk[SLAC_NMK_LENGTH] = {0};
    uint8_t nid[SLAC_NID_LENGTH] = {0};

    uint8_t evse_mac[SLAC_MAC_ADDRESS_LENGTH] = {0};
    uint8_t pev_mac[SLAC_MAC_ADDRESS_LENGTH]  = {0};
    uint8_t pev_run_id[SLAC_RUN_ID_LENGTH]    = {0};

    void uint8_to_printable_string(const uint8_t *data, const uint16_t length, char *buffer, const uint16_t buffer_length);
    void log_homeplug_message_header_v0(const SLAC_HomeplugMessageHeaderV0 &header);
    void log_homeplug_message_header(const SLAC_HomeplugMessageHeader &header);
    void log_cm_set_key_request(const CM_SetKeyRequest &cm_set_key_request);
    void log_cm_set_key_confirmation(const CM_SetKeyConfirmation &cm_set_key_confirmation);
    void log_cm_slac_parm_confirmation(const CM_SLACParmConfirmation &cm_slac_parm_confirmation);
    void log_cm_slac_parm_request(const CM_SLACParmRequest &cm_slac_parm_request);
    void log_cm_start_atten_char_indication(const CM_StartAttenCharIndication &cm_start_atten_char_indication);
    void log_cm_mnbc_sound_indication(const CM_MNBCSoundIndication &cm_mnbc_sound_indication);
    void log_cm_atten_char_indication(const CM_AttenCharIndication &cm_atten_char_indication);
    void log_cm_atten_profile_indication(const CM_AttenProfileIndication &cm_atten_profile_indication);
    void log_cm_atten_char_response(const CM_AttenCharResponse &cm_atten_char_response);
    void log_cm_slac_match_confirmation(const CM_SLACMatchConfirmation &cm_slac_match_confirmation);
    void log_cm_slac_match_request(const CM_SLACMatchRequest &cm_slac_match_request);
    void log_cm_qualcomm_get_sw_request(const CM_QualcommGetSwRequest &cm_qualcomm_get_sw_request);
    void log_cm_qualcomm_get_sw_confirmation(const CM_QualcommGetSwConfirmation &cm_qualcomm_get_sw_confirmation);
    void log_cm_qualcomm_link_status_request(const CM_QualcommLinkStatusRequest &cm_qualcomm_link_status_request);
    void log_cm_qualcomm_link_status_confirmation(const CM_QualcommLinkStatusConfirmation &cm_qualcomm_link_status_confirmation);
    void log_cm_qualcomm_op_attr_request(const CM_QualcommOpAttrRequest &cm_qualcomm_op_attr_request);
    void log_cm_qualcomm_op_attr_confirmation(const CM_QualcommOpAttrConfirmation &cm_qualcomm_op_attr_confirmation);

    uint8_t *buffer = nullptr;
    uint16_t current_buffer_length = 0;
};