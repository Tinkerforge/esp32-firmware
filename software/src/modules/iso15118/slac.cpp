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

#include "slac.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"

#include <mbedtls/sha256.h>

#include "qca700x.h"

const uint8_t slac_mac_plc_peer[SLAC_MAC_ADDRESS_LENGTH]  = {0x00, 0xB0, 0x52, 0x00, 0x00, 0x01};
const uint8_t slac_mac_broadcast[SLAC_MAC_ADDRESS_LENGTH] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void SLAC::pre_setup()
{
    api_state = Config::Object({
        {"state", Config::Uint8(0)},
        {"modem_found", Config::Bool(false)},
        {"modem_initialization_tries", Config::Uint8(0)},
        {"atten_char_indication_tries", Config::Uint8(0)},
        // DIN/TS 70121:2024-11 Annex C (informative)
        // typical value ranges:
        // < 20: not valid
        // 20-35: reasonable range for robust link
        // > 40: poor signal quality
        {"attenuation_profile", Config::Array({
                Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0),
                Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0),
                Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0),
                Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0),
                Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0),
                Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0),
                Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0),
                Config::Uint8(0), Config::Uint8(0)
            }, Config::get_prototype_uint8_0(), SLAC_AAG_LIST_LENGTH, SLAC_AAG_LIST_LENGTH, Config::type_id<Config::ConfUint>())
        },
        {"received_aag_lists", Config::Uint8(0)},
        {"received_sounds", Config::Uint8(0)},
        {"nmk", Config::Array({
                Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0),
                Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0)
            }, Config::get_prototype_uint8_0(), SLAC_NMK_LENGTH, SLAC_NMK_LENGTH, Config::type_id<Config::ConfUint>())
        },
        {"nid", Config::Array({
                Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0),
            }, Config::get_prototype_uint8_0(), SLAC_NID_LENGTH, SLAC_NID_LENGTH, Config::type_id<Config::ConfUint>())
        },
        {"evse_mac", Config::Array({
                Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0)
            }, Config::get_prototype_uint8_0(), SLAC_MAC_ADDRESS_LENGTH, SLAC_MAC_ADDRESS_LENGTH, Config::type_id<Config::ConfUint>())
        },
        {"evse_mac_modem", Config::Array({
                Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0)
            }, Config::get_prototype_uint8_0(), SLAC_MAC_ADDRESS_LENGTH, SLAC_MAC_ADDRESS_LENGTH, Config::type_id<Config::ConfUint>())
        },
        {"pev_mac", Config::Array({
                Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0)
            }, Config::get_prototype_uint8_0(), SLAC_MAC_ADDRESS_LENGTH, SLAC_MAC_ADDRESS_LENGTH, Config::type_id<Config::ConfUint>())
        },
        {"pev_mac_modem", Config::Array({
                Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0)
            }, Config::get_prototype_uint8_0(), SLAC_MAC_ADDRESS_LENGTH, SLAC_MAC_ADDRESS_LENGTH, Config::type_id<Config::ConfUint>())
        },
        {"pev_run_id", Config::Array({
                Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0)
            }, Config::get_prototype_uint8_0(), SLAC_RUN_ID_LENGTH, SLAC_RUN_ID_LENGTH, Config::type_id<Config::ConfUint>())
        }
    });
}


// NID generation by pbkdf1 (5x sha256 hashing of NMK)
// See HPGP 4.4.3.1
// We use security level value 0b00 as defined in ISO 15118-3 A.9.4.1 [V2G3-A09-93]
void SLAC::generate_nid_from_nmk(void)
{
    uint8_t hash_new[32];
	mbedtls_sha256(nmk, 16, hash_new, 0);
    uint8_t hash_old[32];
    for (uint8_t i = 0; i < 5 - 1; ++i) {
		memcpy(hash_old, hash_new, 32);
		mbedtls_sha256(hash_old, 32, hash_new, 0);
    }

    memcpy(nid, hash_new, 6);
    nid[6] = 0b00 | (hash_new[6] >> 4);

    for (size_t i = 0; i < SLAC_NID_LENGTH; i++) {
        api_state.get("nid")->get(i)->updateUint(nid[i]);
    }
}

void SLAC::generate_new_nmk_and_nid(void)
{
    for (uint8_t i = 0; i < SLAC_NMK_LENGTH; i++) {
        nmk[i] = random(256);
        api_state.get("nmk")->get(i)->updateUint(nmk[i]);
    }

    generate_nid_from_nmk();
}

void SLAC::fill_header_v0(SLAC_HomeplugMessageHeaderV0 *header, const uint8_t *destination_mac, const uint8_t *source_mac, const uint16_t mm_type)
{
    memcpy(header->destination_mac, destination_mac, SLAC_MAC_ADDRESS_LENGTH);
    memcpy(header->source_mac,      source_mac,      SLAC_MAC_ADDRESS_LENGTH);

    header->ethernet_type = htons(SLAC_ETHERNET_TYPE_HOMEPLUG);
    header->mm_version    = 0x00 ;
    header->mm_type       = mm_type;
}

void SLAC::fill_header(SLAC_HomeplugMessageHeader *header, const uint8_t *destination_mac, const uint8_t *source_mac, const uint16_t mm_type)
{
    memcpy(header->destination_mac, destination_mac, SLAC_MAC_ADDRESS_LENGTH);
    memcpy(header->source_mac,      source_mac,      SLAC_MAC_ADDRESS_LENGTH);

    header->ethernet_type = htons(SLAC_ETHERNET_TYPE_HOMEPLUG);
    header->mm_version    = 0x01;
    header->mm_type       = mm_type;
    header->fmni          = 0;
    header->fmsn          = 0;
}

// Modem reset is specific to the QCA700x modem, not part of the ISO 15118-3 standard
void SLAC::handle_modem_reset(void)
{
    logger.printfln("Reset QCA700X Modem");
    const uint16_t spi_config = qca700x.read_register(QCA700X_SPI_REG_SPI_CONFIG) | QCA700X_SPI_INT_CPU_ON;
    qca700x.write_register(QCA700X_SPI_REG_SPI_CONFIG, spi_config);

    next_timeout = {};
    state = SLAC::State::ModemInitialization;
}

void SLAC::handle_modem_initialization(void)
{
    if (api_state.get("modem_initialization_tries")->asUint() > 200) {
        return;
    }

    next_timeout = {};
    api_state.get("atten_char_indication_tries")->updateUint(0);
    api_state.get("received_aag_lists")->updateUint(0);
    api_state.get("received_sounds")->updateUint(0);
    // TODO: Clear rest of api_state?

    std::fill_n(aag_list, SLAC_AAG_LIST_LENGTH, 0);

    // To check if QCA700x is ready we read the signature and check the write space
    const uint16_t signature = qca700x.read_register(QCA700X_SPI_REG_SIGNATURE);
    if (signature != QCA700X_SPI_GOOD_SIGNATURE) {
        const uint8_t modem_initialization_tries = api_state.get("modem_initialization_tries")->asUint() + 1;
        api_state.get("modem_initialization_tries")->updateUint(modem_initialization_tries);
        if ((modem_initialization_tries % 50) == 0) {
            logger.printfln("QCA700X modem not found for %u tries: Signature is %d (expected %d)", modem_initialization_tries, signature, QCA700X_SPI_GOOD_SIGNATURE);

            state = SLAC::State::ModemReset;
        }
        if (modem_initialization_tries == 200) {
            logger.printfln("QCA700X modem not found after 200 tries, giving up");
        }
        return;
    }
    api_state.get("modem_found")->updateBool(true);

    const uint16_t write_space = qca700x.read_register(QCA700X_SPI_REG_WRBUF_SPC_AVA);
    if (write_space != QCA700X_BUFFER_SIZE) {
        logger.printfln("QCA700X modem not ready: Write space is %d (expected %d)", write_space, QCA700X_BUFFER_SIZE);
        state = SLAC::State::ModemReset;
        return;
    }

    // Use QCA700x MAC adress for EVSE (it is derived from ethernet MAC)
    memcpy(evse_mac, qca700x.mac, SLAC_MAC_ADDRESS_LENGTH);
    for (size_t i = 0; i < SLAC_MAC_ADDRESS_LENGTH; i++) {
        api_state.get("evse_mac")->get(i)->updateUint(evse_mac[i]);
    }

    // Initialize buffer for modem communication once we now that the modem is available
    // If the modem is never used, the buffer is not allocated
    if (buffer == nullptr) {
        buffer = (uint8_t *)calloc_psram_or_dram(QCA700X_BUFFER_SIZE + QCA700X_HW_PKT_SIZE + 1, sizeof(uint8_t));
    }

    logger.printfln("QCA700X modem found and initialized");

    state = SLAC::State::CMSetKeyRequest;
}

// ISO 15118-3 A.9.5.2 Table A.8
void SLAC::handle_cm_set_key_request(void)
{

    // Generate a new NMK for each new session
    // The NID is derived from NMK
    generate_new_nmk_and_nid();

    CM_SetKeyRequest cm_set_key_request;
    fill_header(&cm_set_key_request.header, slac_mac_plc_peer, evse_mac, SLAC_MMTYPE_CM_SET_KEY | SLAC_MMTYPE_MODE_REQUEST);
    memcpy(cm_set_key_request.nmk, nmk, SLAC_NMK_LENGTH);
    memcpy(cm_set_key_request.nid, nid, SLAC_NID_LENGTH);

    uint8_t *data = reinterpret_cast<uint8_t*>(&cm_set_key_request);
    qca700x.write_burst(data, sizeof(cm_set_key_request));
    //write(qca700x.tap, data, sizeof(cm_set_key_request));

    log_cm_set_key_request(cm_set_key_request);
    next_timeout = now_us() + 100_ms; // This is internal communication with the modem, there is not timing defined in the spec. 100ms seems reasonable.
    state = SLAC::State::WaitForCMSetKeyConfirmation;
}

void SLAC::handle_cm_set_key_confirmation(const CM_SetKeyConfirmation &cm_set_key_confirmation)
{
    if (cm_set_key_confirmation.result != 0x01) {
        logger.printfln("CM_SET_KEY.CNF result unexpected: %02x", cm_set_key_confirmation.result);
        state = SLAC::State::ModemReset;
        return;
    }

    // This is the first time we see the MAC address of our modem, we save it
    for (size_t i = 0; i < SLAC_MAC_ADDRESS_LENGTH; i++) {
        api_state.get("evse_mac_modem")->get(i)->updateUint(cm_set_key_confirmation.header.source_mac[i]);
    }

    // There is no timeout here, we indefinitely wait for the CM_SLAC_PARAM.REQ from the EV
    // TODO 1: How much power does this draw? We could turn the modem off by default and only turn it on when we detect the EV (i.e. EVSE changes from state A to B)
    // TODO 2: According to [V2G3 M06-07] we should change to state E/F after the EVSE changes from state A to B and there is no CM_SLAC_PARAM.REQ from the EV.
    //         This is not implemented yet.
    next_timeout = {};

    state = SLAC::State::WaitForSlacParamRequest;
    log_cm_set_key_confirmation(cm_set_key_confirmation);
}

// ISO 15118-3 A.9.1.2 Table A.2
void SLAC::handle_cm_slac_parm_request(const CM_SLACParmRequest &cm_slac_parm_request)
{
    // This is the first time we see the MAC and run_id of the PEV, we save it
    memcpy(pev_mac, cm_slac_parm_request.header.source_mac, SLAC_MAC_ADDRESS_LENGTH);
    for (size_t i = 0; i < SLAC_MAC_ADDRESS_LENGTH; i++) {
        api_state.get("pev_mac")->get(i)->updateUint(pev_mac[i]);
    }
    memcpy(pev_run_id, cm_slac_parm_request.run_id, SLAC_RUN_ID_LENGTH);
    for (size_t i = 0; i < SLAC_RUN_ID_LENGTH; i++) {
        api_state.get("pev_run_id")->get(i)->updateUint(pev_run_id[i]);
    }

    CM_SLACParmConfirmation cm_slac_parm_confirmation;
    fill_header(&cm_slac_parm_confirmation.header, pev_mac, evse_mac, SLAC_MMTYPE_CM_SLAC_PARM | SLAC_MMTYPE_MODE_CONFIRMATION);
    memcpy(cm_slac_parm_confirmation.m_sound_target, slac_mac_broadcast, SLAC_MAC_ADDRESS_LENGTH);
    memcpy(cm_slac_parm_confirmation.forwarding_sta, pev_mac, SLAC_MAC_ADDRESS_LENGTH);
    memcpy(cm_slac_parm_confirmation.run_id, pev_run_id, SLAC_RUN_ID_LENGTH);

    qca700x.write_burst(reinterpret_cast<uint8_t*>(&cm_slac_parm_confirmation), sizeof(cm_slac_parm_confirmation));

    // Wait for CM_START_ATTEN_CHAR.IND from the EV
    next_timeout = now_us() + SLAC_TT_MATCH_SEQUENCE;
    state = SLAC::State::WaitForStartAttenCharIndication;

    log_cm_slac_parm_request(cm_slac_parm_request);
    log_cm_slac_parm_confirmation(cm_slac_parm_confirmation);
}

// ISO 15118-3 A.9.2.2 Table A.4
void SLAC::handle_cm_start_atten_char_indication(const CM_StartAttenCharIndication &cm_start_atten_char_indication)
{
    if (cm_start_atten_char_indication.application_type != 0x00) {
        logger.printfln("CM_START_ATTEN_CHAR.IND application_type mismatch");
        // ISO 15118-3 A.9.2.3.3 [V2G3-A09-41]
        return;
    }

    if (cm_start_atten_char_indication.security_type != 0x00) {
        logger.printfln("CM_START_ATTEN_CHAR.IND security_type mismatch");
        // ISO 15118-3 A.9.2.3.3 [V2G3-A09-41]
        return;
    }

    if (cm_start_atten_char_indication.resp_type != 0x01) {
        logger.printfln("CM_START_ATTEN_CHAR.IND resp_type mismatch");
        // ISO 15118-3 A.9.2.3.3 [V2G3-A09-41]
        return;
    }

    api_state.get("received_sounds")->updateUint(0);
    api_state.get("received_aag_lists")->updateUint(0);
    api_state.get("atten_char_indication_tries")->updateUint(0);
    std::fill_n(aag_list, SLAC_AAG_LIST_LENGTH, 0);

    // Wait for CM_MNBC_SOUND.IND from the EV
    // The CM_START_ATTEN_CHAR.IND will be sent three times and we set the MATCH_MNBC timeout the first time
    if (state == SLAC::State::WaitForStartAttenCharIndication) {
        next_timeout = now_us() + SLAC_TT_EVSE_MATCH_MNBC;
    }
    state = SLAC::State::WaitForMNBCSound;

    log_cm_start_atten_char_indication(cm_start_atten_char_indication);
}

// ISO 15118-3 A.9.2.2 Table A.4
void SLAC::handle_cm_mnbc_sound_indication(const CM_MNBCSoundIndication &cm_mnbc_sound_indication)
{
    // The sound indication is send three times by the EV.
    // There does not seem to be any requirement to check the sound indications for anything (ISO 15118-3 A.9.2.3.3).
    api_state.get("received_sounds")->updateUint(api_state.get("received_sounds")->asUint() + 1);

    log_cm_mnbc_sound_indication(cm_mnbc_sound_indication);
}

// ISO 15118-3 A.9.2.2 Table A.4
void SLAC::handle_cm_atten_profile_indication(const CM_AttenProfileIndication &cm_atten_profile_indication)
{
    if (memcmp(cm_atten_profile_indication.pev_mac, pev_mac, SLAC_MAC_ADDRESS_LENGTH) != 0) {
        logger.printfln("CM_ATTEN_PROFILE.IND pev_mac mismatch");
        // Ignore profile indication from other EVs
        return;
    }

    if (cm_atten_profile_indication.num_groups != SLAC_AAG_LIST_LENGTH) {
        logger.printfln("CM_ATTEN_PROFILE.IND num_groups mismatch: %d vs %d", cm_atten_profile_indication.num_groups, SLAC_AAG_LIST_LENGTH);
        // This should be a fixed value of SLAC_AAG_LIST_LENGTH
        return;
    }

    for (uint8_t i = 0; i < SLAC_AAG_LIST_LENGTH; i++) {
        aag_list[i] += cm_atten_profile_indication.aag[i];
    }

    log_cm_atten_profile_indication(cm_atten_profile_indication);

    const uint8_t received_aag_lists = api_state.get("received_aag_lists")->asUint() + 1;
    api_state.get("received_aag_lists")->updateUint(received_aag_lists);
    if (received_aag_lists == SLAC_C_EV_MATCH_MNBC) {
        // We have received all 10 attenuation profiles
        // We can now calculate the average attenuation profile and send the CM_ATTEN_CHAR.IND to the EV

        CM_AttenCharIndication cm_atten_char_indication;
        fill_header(&cm_atten_char_indication.header, pev_mac, evse_mac, SLAC_MMTYPE_CM_ATTEN_CHAR | SLAC_MMTYPE_MODE_INDICATION);
        cm_atten_char_indication.num_sounds = received_aag_lists;
        memcpy(cm_atten_char_indication.source_address, pev_mac, SLAC_MAC_ADDRESS_LENGTH);
        memcpy(cm_atten_char_indication.run_id, pev_run_id, SLAC_RUN_ID_LENGTH);

        // ISO 15118-3 A.9.2.1 [V2G3-A09-20]
        for (uint8_t i = 0; i < SLAC_AAG_LIST_LENGTH; i++) {
            cm_atten_char_indication.attenuation_profile.aag[i] = aag_list[i] / received_aag_lists;
            api_state.get("attenuation_profile")->get(i)->updateUint(cm_atten_char_indication.attenuation_profile.aag[i]);
        }

        qca700x.write_burst(reinterpret_cast<uint8_t*>(&cm_atten_char_indication), sizeof(cm_atten_char_indication));

        next_timeout = now_us() + SLAC_TT_MATCH_RESPONSE;
        state = SLAC::State::WaitForAttenChar;
        api_state.get("atten_char_indication_tries")->updateUint(api_state.get("atten_char_indication_tries")->asUint() + 1);
        log_cm_atten_char_indication(cm_atten_char_indication);
    }
}

// ISO 15118-3 A.9.2.2 Table A.4
void SLAC::handle_cm_atten_char_response(const CM_AttenCharResponse &cm_atten_char_response)
{
    if (cm_atten_char_response.application_type != 0x00) {
        logger.printfln("CM_ATTEN_CHAR.RSP application_type mismatch");
        // ISO 15118-3 A.9.2.3.3 [V2G3-A09-47]
        return;
    }

    if (cm_atten_char_response.security_type != 0x00) {
        logger.printfln("CM_ATTEN_CHAR.RSP security_type mismatch");
        // ISO 15118-3 A.9.2.3.3 [V2G3-A09-47]
        return;
    }

    if (cm_atten_char_response.result != 0x00) {
        logger.printfln("CM_ATTEN_CHAR.RSP result mismatch");
        // ISO 15118-3 A.9.2.3.3 [V2G3-A09-47]
        return;
    }

    if (memcmp(pev_mac, cm_atten_char_response.source_address, SLAC_MAC_ADDRESS_LENGTH) != 0) {
        logger.printfln("CM_ATTEN_CHAR.RSP source_address mismatch");
        // ISO 15118-3 A.9.2.3.3 [V2G3-A09-47]
        return;
    }

    if (memcmp(pev_run_id, cm_atten_char_response.run_id, SLAC_RUN_ID_LENGTH) != 0) {
        logger.printfln("CM_ATTEN_CHAR.RSP run_id mismatch");
        // ISO 15118-3 A.9.2.3.3 [V2G3-A09-47]
        return;
    }

    logger.printfln("SLAC process sucessful");
    next_timeout = now_us() + SLAC_TT_EVSE_MATCH_SESSION;
    state = SLAC::State::WaitForSlacMatch;

    log_cm_atten_char_response(cm_atten_char_response);
}

// ISO 15118-3 A.9.4.2 Table A.7
void SLAC::handle_cm_slac_match_request(const CM_SLACMatchRequest &cm_slac_match_request)
{
    if (memcmp(pev_mac, cm_slac_match_request.header.source_mac, SLAC_MAC_ADDRESS_LENGTH) != 0) {
        logger.printfln("CM_SLAC_MATCH.REQ source_mac mismatch");
        // ISO 15118-3 A.9.3.3 [V2G3-A09-98]
        return;
    }

    if (memcmp(pev_run_id, cm_slac_match_request.run_id, SLAC_RUN_ID_LENGTH) != 0) {
        logger.printfln("CM_SLAC_MATCH.REQ run_id mismatch");
        // ISO 15118-3 A.9.3.3 [V2G3-A09-98]
        return;
    }

    CM_SLACMatchConfirmation cm_slac_match_confirmation;
    fill_header(&cm_slac_match_confirmation.header, pev_mac, evse_mac, SLAC_MMTYPE_CM_SLAC_MATCH | SLAC_MMTYPE_MODE_CONFIRMATION);
    memcpy(cm_slac_match_confirmation.pev_id, cm_slac_match_request.pev_id, SLAC_STATION_ID_LENGTH);
    memcpy(cm_slac_match_confirmation.pev_mac, cm_slac_match_request.pev_mac, SLAC_MAC_ADDRESS_LENGTH);
    memcpy(cm_slac_match_confirmation.evse_id, cm_slac_match_request.evse_id, SLAC_STATION_ID_LENGTH);
    memcpy(cm_slac_match_confirmation.evse_mac, cm_slac_match_request.evse_mac, SLAC_MAC_ADDRESS_LENGTH);
    memcpy(cm_slac_match_confirmation.run_id, cm_slac_match_request.run_id, SLAC_RUN_ID_LENGTH);
    memcpy(cm_slac_match_confirmation.nid, nid, SLAC_NID_LENGTH);
    memcpy(cm_slac_match_confirmation.nmk, nmk, SLAC_NMK_LENGTH);

    qca700x.write_burst(reinterpret_cast<uint8_t*>(&cm_slac_match_confirmation), sizeof(cm_slac_match_confirmation));
    // Here we are done with SLAC. We now wait for "Link detected", which basically means we wait for the first IPV6/SDP packet from the EV.
    next_timeout = now_us() + SLAC_TT_MATCH_JOIN + SLAC_TP_LINK_READY_NOTIFCATION_MAX;
    state = SLAC::State::WaitForSDP;

    log_cm_slac_match_request(cm_slac_match_request);
    log_cm_slac_match_confirmation(cm_slac_match_confirmation);
}

void SLAC::handle_cm_qualcomm_get_sw_request()
{
    CM_QualcommGetSwRequest cm_qualcomm_get_sw_request;
    fill_header_v0(&cm_qualcomm_get_sw_request.header, slac_mac_broadcast, evse_mac, SLAC_MMTYPE_QUALCOMM_GET_SW | SLAC_MMTYPE_MODE_REQUEST);
    qca700x.write_burst(reinterpret_cast<uint8_t*>(&cm_qualcomm_get_sw_request), sizeof(cm_qualcomm_get_sw_request));

    log_cm_qualcomm_get_sw_request(cm_qualcomm_get_sw_request);
    next_timeout = now_us() + 2500_ms;
    state = SLAC::State::WaitForCMQualcommGetSwResponse;
}

void SLAC::handle_cm_qualcomm_link_status_request()
{
    CM_QualcommLinkStatusRequest cm_qualcomm_link_status_request;
    fill_header_v0(&cm_qualcomm_link_status_request.header, slac_mac_broadcast, evse_mac, SLAC_MMTYPE_QUALCOMM_LINK_STATUS | SLAC_MMTYPE_MODE_REQUEST);
    qca700x.write_burst(reinterpret_cast<uint8_t*>(&cm_qualcomm_link_status_request), sizeof(cm_qualcomm_link_status_request));

    log_cm_qualcomm_link_status_request(cm_qualcomm_link_status_request);
    next_timeout = now_us() + 2500_ms;
    state = SLAC::State::WaitForCMQualcommLinkStatusResponse;
}

void SLAC::handle_cm_qualcomm_op_attr_request()
{
    CM_QualcommOpAttrRequest cm_qualcomm_op_attr_request;
    fill_header_v0(&cm_qualcomm_op_attr_request.header, slac_mac_broadcast, evse_mac, SLAC_MMTYPE_QUALCOMM_OP_ATTR | SLAC_MMTYPE_MODE_REQUEST);
    qca700x.write_burst(reinterpret_cast<uint8_t*>(&cm_qualcomm_op_attr_request), sizeof(cm_qualcomm_op_attr_request));

    log_cm_qualcomm_op_attr_request(cm_qualcomm_op_attr_request);
    next_timeout = now_us() + 2500_ms;
    state = SLAC::State::WaitForCMQualcommOpAttrResponse;
}

void SLAC::handle_cm_qualcomm_get_sw_confirmation(const CM_QualcommGetSwConfirmation &cm_qualcomm_get_sw_confirmation)
{
    log_cm_qualcomm_get_sw_confirmation(cm_qualcomm_get_sw_confirmation);
    next_timeout = now_us() + 100_ms;
    state = SLAC::State::CMQualcommLinkStatusRequest;
}

void SLAC::handle_cm_qualcomm_link_status_confirmation(const CM_QualcommLinkStatusConfirmation &cm_qualcomm_link_status_confirmation)
{
    log_cm_qualcomm_link_status_confirmation(cm_qualcomm_link_status_confirmation);
    next_timeout = now_us() + 100_ms;
    state = SLAC::State::CMQualcommOpAttrRequest;
}

void SLAC::handle_cm_qualcomm_op_attr_confirmation(const CM_QualcommOpAttrConfirmation &cm_qualcomm_op_attr_confirmation)
{
    log_cm_qualcomm_op_attr_confirmation(cm_qualcomm_op_attr_confirmation);
    next_timeout = {};
    state = SLAC::State::WaitForSlacParamRequest;
}


void SLAC::poll_modem(void)
{
    // Poll modem for data
    uint16_t length = qca700x.read_burst(buffer, QCA700X_BUFFER_SIZE + QCA700X_HW_PKT_SIZE);
    //int16_t length = read(qca700x.tap, buffer, QCA700X_BUFFER_SIZE + QCA700X_HW_PKT_SIZE);

    while (length > 0) {
        int16_t ethernet_frame_length = qca700x.check_receive_frame(buffer, length);
        if (ethernet_frame_length < 0) {
            state = SLAC::State::ModemReset;
            break;
        }
        SLAC_HomeplugMessageHeader *header = reinterpret_cast<SLAC_HomeplugMessageHeader*>(buffer + QCA700X_RECV_HEADER_SIZE);

        switch (ntohs(header->ethernet_type)) {
            case SLAC_ETHERNET_TYPE_HOMEPLUG: {
                const uint8_t *message_buffer = buffer + QCA700X_RECV_HEADER_SIZE;
                const uint16_t mm_type = header->mm_type;
                switch(mm_type) {
                    case SLAC_MMTYPE_CM_SET_KEY           | SLAC_MMTYPE_MODE_CONFIRMATION: handle_cm_set_key_confirmation(*reinterpret_cast<const CM_SetKeyConfirmation*>(message_buffer));                          break;
                    case SLAC_MMTYPE_CM_SLAC_PARM         | SLAC_MMTYPE_MODE_REQUEST:      handle_cm_slac_parm_request(*reinterpret_cast<const CM_SLACParmRequest*>(message_buffer));                                break;
                    case SLAC_MMTYPE_CM_START_ATTEN_CHAR  | SLAC_MMTYPE_MODE_INDICATION:   handle_cm_start_atten_char_indication(*reinterpret_cast<const CM_StartAttenCharIndication*>(message_buffer));             break;
                    case SLAC_MMTYPE_CM_MNBC_SOUND        | SLAC_MMTYPE_MODE_INDICATION:   handle_cm_mnbc_sound_indication(*reinterpret_cast<const CM_MNBCSoundIndication*>(message_buffer));                        break;
                    case SLAC_MMTYPE_CM_ATTEN_PROFILE     | SLAC_MMTYPE_MODE_INDICATION:   handle_cm_atten_profile_indication(*reinterpret_cast<const CM_AttenProfileIndication*>(message_buffer));                  break;
                    case SLAC_MMTYPE_CM_ATTEN_CHAR        | SLAC_MMTYPE_MODE_RESPONSE:     handle_cm_atten_char_response(*reinterpret_cast<const CM_AttenCharResponse*>(message_buffer));                            break;
                    case SLAC_MMTYPE_CM_SLAC_MATCH        | SLAC_MMTYPE_MODE_REQUEST:      handle_cm_slac_match_request(*reinterpret_cast<const CM_SLACMatchRequest*>(message_buffer));                              break;
                    case SLAC_MMTYPE_QUALCOMM_GET_SW      | SLAC_MMTYPE_MODE_CONFIRMATION: handle_cm_qualcomm_get_sw_confirmation(*reinterpret_cast<const CM_QualcommGetSwConfirmation*>(message_buffer));           break;
                    case SLAC_MMTYPE_QUALCOMM_LINK_STATUS | SLAC_MMTYPE_MODE_CONFIRMATION: handle_cm_qualcomm_link_status_confirmation(*reinterpret_cast<const CM_QualcommLinkStatusConfirmation*>(message_buffer)); break;
                    case SLAC_MMTYPE_QUALCOMM_OP_ATTR     | SLAC_MMTYPE_MODE_CONFIRMATION: handle_cm_qualcomm_op_attr_confirmation(*reinterpret_cast<const CM_QualcommOpAttrConfirmation*>(message_buffer));         break;
                    default: logger.printfln("Unhandled mm_type: %04x", mm_type); break;
                }
                break;
            }

            case SLAC_ETHERNET_TYPE_IPV6: {
                //uint8_t sdp_test_data1[78] = {0x33, 0x33, 0xff, 0x07, 0xd0, 0x56, 0x00, 0x7d, 0xfa, 0x07, 0xd0, 0x56, 0x86, 0xdd, 0x60, 0x00, 0x00, 0x00, 0x00, 0x18, 0x3a, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0x07, 0xd0, 0x56, 0x87, 0x00, 0xdf, 0xed, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x7d, 0xfa, 0xff, 0xfe, 0x07, 0xd0, 0x56};
                //uint8_t sdp_test_data2[72] = {0x33, 0x33, 0x00, 0x00, 0x00, 0x01, 0x00, 0x7d, 0xfa, 0x09, 0xf5, 0xa4, 0x86, 0xdd, 0x60, 0x00, 0x00, 0x00, 0x00, 0x12, 0x11, 0x40, 0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x7d, 0xfa, 0xff, 0xfe, 0x09, 0xf5, 0xa4, 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x3b, 0x0e, 0x00, 0x12, 0x84, 0x0a, 0x01, 0xfe, 0x90, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00};

                // If we are in state WaitForSDP and we get our first IPv6 package, we know that
                // the IPv6 connection is established and we can issue a link up to the higher layer.
                if (state == SLAC::State::WaitForSDP) {
                    qca700x.link_up();
                    state = SLAC::State::LinkDetected;
                }

                logger.printfln("IPv6 packet received: %d", ethernet_frame_length);
                iso15118.trace_array("IPv6 packet", buffer + QCA700X_RECV_HEADER_SIZE, ethernet_frame_length);

                qca700x.received_data_to_netif(buffer + QCA700X_RECV_HEADER_SIZE, ethernet_frame_length);
                break;
            }

            default: {
                logger.printfln("Unknown ethernet type: %04x", header->ethernet_type);
                break;
            }
        }

        // there might be more data still in the buffer. Check if there is another packet.
        const uint16_t frame_length     = ethernet_frame_length + QCA700X_RECV_HEADER_SIZE + QCA700X_RECV_FOOTER_SIZE;
        const int16_t  remaining_length = length - frame_length;
        if (remaining_length >= QCA700X_RECV_BUFFER_MIN_SIZE) {
            // Copy remaining data to the beginning of the buffer
            memcpy(buffer, buffer+frame_length, remaining_length);
            length = remaining_length;
        } else {
            length = 0;
        }
    }
}

void SLAC::state_machine_loop()
{
    // Explicitely only poll the modem for IPv6 packets if link is detected
    // TODO: If we use l2tap the modem polling can be done in qca700x code directly and we can just return here.
    if (state == SLAC::State::LinkDetected) {
        poll_modem();
        api_state.get("state")->updateUint(static_cast<std::underlying_type<State>::type>(state));
        return;
    }

    switch(state) {
        // First handle states were we initiate a message
        case SLAC::State::ModemReset:          handle_modem_reset();          break;
        case SLAC::State::ModemInitialization: handle_modem_initialization(); break;
        case SLAC::State::CMSetKeyRequest:     handle_cm_set_key_request();   break;

        // Then handle some qualcomm QCA700x specific requests
        // These will probably not be part of a final release firmware, but we can use them for testing stuff
        case SLAC::State::CMQualcommGetSwRequest:      handle_cm_qualcomm_get_sw_request();      break;
        case SLAC::State::CMQualcommLinkStatusRequest: handle_cm_qualcomm_link_status_request(); break;
        case SLAC::State::CMQualcommOpAttrRequest:     handle_cm_qualcomm_op_attr_request();     break;

        // Then handle states were we expect a response by reading from modem
        default: poll_modem(); break;
    }

    // Then handle timeouts of expected responses
    if (next_timeout.is_some() && deadline_elapsed(next_timeout.unwrap())) {
        logger.printfln("SLAC: Timeout in state %s", state_to_string(state));
        // As long as we have received some sounds we will do the average attenuation profile calculation
        // and move on to the next state, even after timeout. ISO 15118-3 A.9.2.3.3 [V2G3-A09-43].
        const uint8_t received_aag_lists          = api_state.get("received_aag_lists")->asUint();
        const uint8_t atten_char_indication_tries = api_state.get("atten_char_indication_tries")->asUint();
        if (((state == SLAC::State::WaitForMNBCSound) && (received_aag_lists > 0)) ||
             // This is also re-send up to SLAC_C_EV_MATCH_RETRY tries. ISO 15118-3 A.9.2.3.3 [V2G3-A09-46].
            ((state == SLAC::State::WaitForAttenChar) && (atten_char_indication_tries < SLAC_C_EV_MATCH_RETRY))) {
            CM_AttenCharIndication cm_atten_char_indication;
            fill_header(&cm_atten_char_indication.header, pev_mac, evse_mac, SLAC_MMTYPE_CM_ATTEN_CHAR | SLAC_MMTYPE_MODE_INDICATION);
            cm_atten_char_indication.num_sounds = received_aag_lists;
            memcpy(cm_atten_char_indication.source_address, pev_mac, SLAC_MAC_ADDRESS_LENGTH);
            memcpy(cm_atten_char_indication.run_id, pev_run_id, SLAC_RUN_ID_LENGTH);

            // ISO 15118-3 A.9.2.1 [V2G3-A09-20]
            for (uint8_t i = 0; i < SLAC_AAG_LIST_LENGTH; i++) {
                cm_atten_char_indication.attenuation_profile.aag[i] = aag_list[i] / received_aag_lists;
                api_state.get("attenuation_profile")->get(i)->updateUint(cm_atten_char_indication.attenuation_profile.aag[i]);
            }

            qca700x.write_burst(reinterpret_cast<uint8_t*>(&cm_atten_char_indication), sizeof(cm_atten_char_indication));

            next_timeout = now_us() + SLAC_TT_MATCH_RESPONSE;
            state = SLAC::State::WaitForAttenChar;
            api_state.get("atten_char_indication_tries")->updateUint(atten_char_indication_tries + 1);
            log_cm_atten_char_indication(cm_atten_char_indication);
        // For the vendor specific requests we just go to the next state if there is no response.
        // We should not fail if they don't work.
        } else if (state == SLAC::State::WaitForCMQualcommGetSwResponse) {
            next_timeout = {};
            state = SLAC::State::CMQualcommLinkStatusRequest;
        } else if (state == SLAC::State::WaitForCMQualcommLinkStatusResponse) {
            next_timeout = {};
            state = SLAC::State::CMQualcommOpAttrRequest;
        } else if (state == SLAC::State::WaitForCMQualcommOpAttrResponse) {
            next_timeout = {};
            state = SLAC::State::WaitForSlacParamRequest;
        } else {
            // TODO: I think the EVSE should change to state E/F and then back to 5% in case of a timeout.
            //       Maybe we can just do the 5% -> E/F -> 5% thing in handle_mode_reset?
            //       Otherwise it maybe makes more sense for us to let the upper layers decide what to do if SLAC process fails.
            handle_modem_reset();
        }
    }

    api_state.get("state")->updateUint(static_cast<std::underlying_type<State>::type>(state));
}


// -- Logging functions --
const char *SLAC::state_to_string(const SLAC::State state)
{
    switch(state) {
        case SLAC::State::ModemReset:                          return "ModemReset";
        case SLAC::State::ModemInitialization:                 return "ModemInitialization";
        case SLAC::State::CMSetKeyRequest:                     return "CMSetKeyRequest";
        case SLAC::State::WaitForCMSetKeyConfirmation:         return "WaitForCMSetKeyConfirmation";
        case SLAC::State::CMQualcommGetSwRequest:              return "CMQualcommGetSwRequest";
        case SLAC::State::WaitForCMQualcommGetSwResponse:      return "WaitForCMQualcommGetSwResponse";
        case SLAC::State::CMQualcommLinkStatusRequest:         return "CMQualcommLinkStatusRequest";
        case SLAC::State::WaitForCMQualcommLinkStatusResponse: return "WaitForCMQualcommLinkStatusResponse";
        case SLAC::State::CMQualcommOpAttrRequest:             return "CMQualcommOpAttrRequest";
        case SLAC::State::WaitForCMQualcommOpAttrResponse:     return "WaitForCMQualcommOpAttrResponse";
        case SLAC::State::WaitForSlacParamRequest:             return "WaitForSlacParamRequest";
        case SLAC::State::WaitForStartAttenCharIndication:     return "WaitForStartAttenCharIndication";
        case SLAC::State::WaitForMNBCSound:                    return "WaitForMNBCSound";
        case SLAC::State::WaitForAttenChar:                    return "WaitForAttenChar";
        case SLAC::State::WaitForSlacMatch:                    return "WaitForSlacMatch";
        case SLAC::State::WaitForSDP:                          return "WaitForSDP";
        case SLAC::State::LinkDetected:                        return "LinkDetected";
    }

    return "Unknown";
}

void SLAC::uint8_to_printable_string(const uint8_t *data, const uint16_t length, char *buffer, const uint16_t buffer_length)
{
    uint16_t i = 0;
    for (i = 0; (i < length) && (i < (buffer_length - 1)); i++) {
        if (data[i] >= 32 && data[i] <= 126) {
            buffer[i] = data[i];
        } else {
            buffer[i] = '.';
        }
    }
    buffer[i] = '\0';
}

void SLAC::log_homeplug_message_header_v0(const SLAC_HomeplugMessageHeaderV0 &header)
{
    iso15118.trace("  Ethernet Header:");
    iso15118.trace("    destination_mac: %02x %02x %02x %02x %02x %02x", header.destination_mac[0], header.destination_mac[1], header.destination_mac[2], header.destination_mac[3], header.destination_mac[4], header.destination_mac[5]);
    iso15118.trace("    source_mac:      %02x %02x %02x %02x %02x %02x", header.source_mac[0], header.source_mac[1], header.source_mac[2], header.source_mac[3], header.source_mac[4], header.source_mac[5]);
    iso15118.trace("    ethernet_type:   %04x", header.ethernet_type);
    iso15118.trace("  Homeplug Message Header:");
    iso15118.trace("    mm_version: %02x", header.mm_version);
    iso15118.trace("    mm_type:    %04x", header.mm_type);
}

void SLAC::log_homeplug_message_header(const SLAC_HomeplugMessageHeader &header)
{
    iso15118.trace("  Ethernet Header:");
    iso15118.trace("    destination_mac: %02x %02x %02x %02x %02x %02x", header.destination_mac[0], header.destination_mac[1], header.destination_mac[2], header.destination_mac[3], header.destination_mac[4], header.destination_mac[5]);
    iso15118.trace("    source_mac:      %02x %02x %02x %02x %02x %02x", header.source_mac[0], header.source_mac[1], header.source_mac[2], header.source_mac[3], header.source_mac[4], header.source_mac[5]);
    iso15118.trace("    ethernet_type:   %04x", header.ethernet_type);
    iso15118.trace("  Homeplug Message Header:");
    iso15118.trace("    mm_version: %02x", header.mm_version);
    iso15118.trace("    mm_type:    %04x", header.mm_type);
    iso15118.trace("    fmni:       %02x", header.fmni);
    iso15118.trace("    fmsn:       %02x", header.fmsn);
}

void SLAC::log_cm_set_key_request(const CM_SetKeyRequest &cm_set_key_request)
{
    iso15118.trace("CM_SET_KEY.REQ:");
    log_homeplug_message_header(cm_set_key_request.header);
    iso15118.trace("  key_type:       %02x", cm_set_key_request.key_type);
    iso15118.trace("  my_nonce:       %08lx", cm_set_key_request.my_nonce);
    iso15118.trace("  your_nonce:     %08lx", cm_set_key_request.your_nonce);
    iso15118.trace("  pid:            %02x", cm_set_key_request.pid);
    iso15118.trace("  prn:            %04x", cm_set_key_request.prn);
    iso15118.trace("  pmn:            %02x", cm_set_key_request.pmn);
    iso15118.trace("  cco_capability: %02x", cm_set_key_request.cco_capability);
    iso15118.trace("  nid:            %02x %02x %02x %02x %02x %02x %02x", cm_set_key_request.nid[0], cm_set_key_request.nid[1], cm_set_key_request.nid[2], cm_set_key_request.nid[3], cm_set_key_request.nid[4], cm_set_key_request.nid[5], cm_set_key_request.nid[6]);
    iso15118.trace("  new_eks:        %02x", cm_set_key_request.new_eks);
    iso15118.trace("  nmk:            %02x %02x %02x %02x %02x %02x %02x %02x...", cm_set_key_request.nmk[0], cm_set_key_request.nmk[1], cm_set_key_request.nmk[2], cm_set_key_request.nmk[3], cm_set_key_request.nmk[4], cm_set_key_request.nmk[5], cm_set_key_request.nmk[6], cm_set_key_request.nmk[7]);
}

void SLAC::log_cm_set_key_confirmation(const CM_SetKeyConfirmation &cm_set_key_confirmation)
{
    iso15118.trace("CM_SET_KEY.CNF:");
    log_homeplug_message_header(cm_set_key_confirmation.header);
    iso15118.trace("  result:      %02x", cm_set_key_confirmation.result);
    iso15118.trace("  my_nonce:    %08lx", cm_set_key_confirmation.my_nonce);
    iso15118.trace("  your_nonce:  %08lx", cm_set_key_confirmation.your_nonce);
    iso15118.trace("  pid:         %02x", cm_set_key_confirmation.pid);
    iso15118.trace("  prn:         %04x", cm_set_key_confirmation.prn);
    iso15118.trace("  pmn:         %02x", cm_set_key_confirmation.pmn);
    iso15118.trace("  cco_capability: %02x", cm_set_key_confirmation.cco_capability);
}

void SLAC::log_cm_slac_parm_confirmation(const CM_SLACParmConfirmation &cm_slac_parm_confirmation)
{
    iso15118.trace("CM_SLAC_PARM.CNF:");
    log_homeplug_message_header(cm_slac_parm_confirmation.header);
    iso15118.trace("  m_sound_target:   %02x %02x %02x %02x %02x %02x", cm_slac_parm_confirmation.m_sound_target[0], cm_slac_parm_confirmation.m_sound_target[1], cm_slac_parm_confirmation.m_sound_target[2], cm_slac_parm_confirmation.m_sound_target[3], cm_slac_parm_confirmation.m_sound_target[4], cm_slac_parm_confirmation.m_sound_target[5]);
    iso15118.trace("  num_sounds:       %02x", cm_slac_parm_confirmation.num_sounds);
    iso15118.trace("  timeout:          %02x", cm_slac_parm_confirmation.timeout);
    iso15118.trace("  resp_type:        %02x", cm_slac_parm_confirmation.resp_type);
    iso15118.trace("  forwarding_sta:   %02x %02x %02x %02x %02x %02x", cm_slac_parm_confirmation.forwarding_sta[0], cm_slac_parm_confirmation.forwarding_sta[1], cm_slac_parm_confirmation.forwarding_sta[2], cm_slac_parm_confirmation.forwarding_sta[3], cm_slac_parm_confirmation.forwarding_sta[4], cm_slac_parm_confirmation.forwarding_sta[5]);
    iso15118.trace("  application_type: %02x", cm_slac_parm_confirmation.application_type);
    iso15118.trace("  security_type:    %02x", cm_slac_parm_confirmation.security_type);
    iso15118.trace("  run_id:           %02x %02x %02x %02x %02x %02x %02x %02x", cm_slac_parm_confirmation.run_id[0], cm_slac_parm_confirmation.run_id[1], cm_slac_parm_confirmation.run_id[2], cm_slac_parm_confirmation.run_id[3], cm_slac_parm_confirmation.run_id[4], cm_slac_parm_confirmation.run_id[5], cm_slac_parm_confirmation.run_id[6], cm_slac_parm_confirmation.run_id[7]);
}
void SLAC::log_cm_slac_parm_request(const CM_SLACParmRequest &cm_slac_parm_request)
{
    iso15118.trace("CM_SLAC_PARM.REQ:");
    log_homeplug_message_header(cm_slac_parm_request.header);
    iso15118.trace("  application_type: %02x", cm_slac_parm_request.application_type);
    iso15118.trace("  security_type:    %02x", cm_slac_parm_request.security_type);
    iso15118.trace("  run_id:           %02x %02x %02x %02x %02x %02x %02x %02x", cm_slac_parm_request.run_id[0], cm_slac_parm_request.run_id[1], cm_slac_parm_request.run_id[2], cm_slac_parm_request.run_id[3], cm_slac_parm_request.run_id[4], cm_slac_parm_request.run_id[5], cm_slac_parm_request.run_id[6], cm_slac_parm_request.run_id[7]);
}

void SLAC::log_cm_start_atten_char_indication(const CM_StartAttenCharIndication &cm_start_atten_char_indication)
{
    iso15118.trace("CM_START_ATTEN_CHAR.IND:");
    log_homeplug_message_header(cm_start_atten_char_indication.header);
    iso15118.trace("  application_type: %02x", cm_start_atten_char_indication.application_type);
    iso15118.trace("  security_type:    %02x", cm_start_atten_char_indication.security_type);
    iso15118.trace("  num_sounds:       %02x", cm_start_atten_char_indication.num_sounds);
    iso15118.trace("  timeout:          %02x", cm_start_atten_char_indication.timeout);
    iso15118.trace("  resp_type:        %02x", cm_start_atten_char_indication.resp_type);
    iso15118.trace("  forwarding_sta:   %02x %02x %02x %02x %02x %02x", cm_start_atten_char_indication.forwarding_sta[0], cm_start_atten_char_indication.forwarding_sta[1], cm_start_atten_char_indication.forwarding_sta[2], cm_start_atten_char_indication.forwarding_sta[3], cm_start_atten_char_indication.forwarding_sta[4], cm_start_atten_char_indication.forwarding_sta[5]);
    iso15118.trace("  run_id:           %02x %02x %02x %02x %02x %02x %02x %02x", cm_start_atten_char_indication.run_id[0], cm_start_atten_char_indication.run_id[1], cm_start_atten_char_indication.run_id[2], cm_start_atten_char_indication.run_id[3], cm_start_atten_char_indication.run_id[4], cm_start_atten_char_indication.run_id[5], cm_start_atten_char_indication.run_id[6], cm_start_atten_char_indication.run_id[7]);
}

void SLAC::log_cm_mnbc_sound_indication(const CM_MNBCSoundIndication &cm_mnbc_sound_indication)
{
    iso15118.trace("CM_MNBC_SOUND.IND:");
    log_homeplug_message_header(cm_mnbc_sound_indication.header);
    iso15118.trace("  application_type:      %02x", cm_mnbc_sound_indication.application_type);
    iso15118.trace("  security_type:         %02x", cm_mnbc_sound_indication.security_type);
    iso15118.trace("  sender_id:             %02x %02x %02x %02x %02x %02x %02x %02x", cm_mnbc_sound_indication.sender_id[0], cm_mnbc_sound_indication.sender_id[1], cm_mnbc_sound_indication.sender_id[2], cm_mnbc_sound_indication.sender_id[3], cm_mnbc_sound_indication.sender_id[4], cm_mnbc_sound_indication.sender_id[5], cm_mnbc_sound_indication.sender_id[6], cm_mnbc_sound_indication.sender_id[7]);
    iso15118.trace("  remaining_sound_count: %02x", cm_mnbc_sound_indication.remaining_sound_count);
    iso15118.trace("  run_id:                %02x %02x %02x %02x %02x %02x %02x %02x", cm_mnbc_sound_indication.run_id[0], cm_mnbc_sound_indication.run_id[1], cm_mnbc_sound_indication.run_id[2], cm_mnbc_sound_indication.run_id[3], cm_mnbc_sound_indication.run_id[4], cm_mnbc_sound_indication.run_id[5], cm_mnbc_sound_indication.run_id[6], cm_mnbc_sound_indication.run_id[7]);
    iso15118.trace("  random:                %02x %02x %02x %02x %02x %02x %02x %02x...", cm_mnbc_sound_indication.random[0], cm_mnbc_sound_indication.random[1], cm_mnbc_sound_indication.random[2], cm_mnbc_sound_indication.random[3], cm_mnbc_sound_indication.random[4], cm_mnbc_sound_indication.random[5], cm_mnbc_sound_indication.random[6], cm_mnbc_sound_indication.random[7]);

}

void SLAC::log_cm_atten_char_indication(const CM_AttenCharIndication &cm_atten_char_indication)
{
    iso15118.trace("CM_ATTEN_CHAR.IND:");
    log_homeplug_message_header(cm_atten_char_indication.header);
    iso15118.trace("  application_type: %02x", cm_atten_char_indication.application_type);
    iso15118.trace("  security_type:    %02x", cm_atten_char_indication.security_type);
    iso15118.trace("  source_address:   %02x %02x %02x %02x %02x %02x", cm_atten_char_indication.source_address[0], cm_atten_char_indication.source_address[1], cm_atten_char_indication.source_address[2], cm_atten_char_indication.source_address[3], cm_atten_char_indication.source_address[4], cm_atten_char_indication.source_address[5]);
    iso15118.trace("  run_id:           %02x %02x %02x %02x %02x %02x %02x %02x", cm_atten_char_indication.run_id[0], cm_atten_char_indication.run_id[1], cm_atten_char_indication.run_id[2], cm_atten_char_indication.run_id[3], cm_atten_char_indication.run_id[4], cm_atten_char_indication.run_id[5], cm_atten_char_indication.run_id[6], cm_atten_char_indication.run_id[7]);
    iso15118.trace("  source_id:        %02x %02x %02x %02x %02x %02x %02x %02x...", cm_atten_char_indication.source_id[0], cm_atten_char_indication.source_id[1], cm_atten_char_indication.source_id[2], cm_atten_char_indication.source_id[3], cm_atten_char_indication.source_id[4], cm_atten_char_indication.source_id[5], cm_atten_char_indication.source_id[6], cm_atten_char_indication.source_id[7]);
    iso15118.trace("  resp_id:          %02x %02x %02x %02x %02x %02x %02x %02x...", cm_atten_char_indication.resp_id[0], cm_atten_char_indication.resp_id[1], cm_atten_char_indication.resp_id[2], cm_atten_char_indication.resp_id[3], cm_atten_char_indication.resp_id[4], cm_atten_char_indication.resp_id[5], cm_atten_char_indication.resp_id[6], cm_atten_char_indication.resp_id[7]);
    iso15118.trace("  num_sounds:       %02x", cm_atten_char_indication.num_sounds);
    iso15118.trace("  attenuation_profile:");
    iso15118.trace("    num_groups: %02x", cm_atten_char_indication.attenuation_profile.num_groups);
    for (uint8_t i = 0; i < SLAC_AAG_LIST_LENGTH-2; i+=8) {
        iso15118.trace("    %02x %02x %02x %02x %02x %02x %02x %02x", cm_atten_char_indication.attenuation_profile.aag[i+0], cm_atten_char_indication.attenuation_profile.aag[i+1], cm_atten_char_indication.attenuation_profile.aag[i+2], cm_atten_char_indication.attenuation_profile.aag[i+3], cm_atten_char_indication.attenuation_profile.aag[i+4], cm_atten_char_indication.attenuation_profile.aag[i+5], cm_atten_char_indication.attenuation_profile.aag[i+6], cm_atten_char_indication.attenuation_profile.aag[i+7]);
    }
    iso15118.trace("    %02x %02x", cm_atten_char_indication.attenuation_profile.aag[SLAC_AAG_LIST_LENGTH-2], cm_atten_char_indication.attenuation_profile.aag[SLAC_AAG_LIST_LENGTH-1]);
}

void SLAC::log_cm_atten_profile_indication(const CM_AttenProfileIndication &cm_atten_profile_indication)
{
    iso15118.trace("CM_ATTEN_PROFILE.IND:");
    log_homeplug_message_header(cm_atten_profile_indication.header);
    iso15118.trace("  pev_mac:    %02x %02x %02x %02x %02x %02x", cm_atten_profile_indication.pev_mac[0], cm_atten_profile_indication.pev_mac[1], cm_atten_profile_indication.pev_mac[2], cm_atten_profile_indication.pev_mac[3], cm_atten_profile_indication.pev_mac[4], cm_atten_profile_indication.pev_mac[5]);
    iso15118.trace("  num_groups: %02x", cm_atten_profile_indication.num_groups);
    for (uint8_t i = 0; i < SLAC_AAG_LIST_LENGTH-2; i+=8) {
        iso15118.trace("  %02x %02x %02x %02x %02x %02x %02x %02x", cm_atten_profile_indication.aag[i+0], cm_atten_profile_indication.aag[i+1], cm_atten_profile_indication.aag[i+2], cm_atten_profile_indication.aag[i+3], cm_atten_profile_indication.aag[i+4], cm_atten_profile_indication.aag[i+5], cm_atten_profile_indication.aag[i+6], cm_atten_profile_indication.aag[i+7]);
    }
    iso15118.trace("  %02x %02x", cm_atten_profile_indication.aag[SLAC_AAG_LIST_LENGTH-2], cm_atten_profile_indication.aag[SLAC_AAG_LIST_LENGTH-1]);
}

void SLAC::log_cm_atten_char_response(const CM_AttenCharResponse &cm_atten_char_response)
{
    iso15118.trace("CM_ATTEN_CHAR.RSP:");
    log_homeplug_message_header(cm_atten_char_response.header);
    iso15118.trace("  application_type: %02x", cm_atten_char_response.application_type);
    iso15118.trace("  security_type:    %02x", cm_atten_char_response.security_type);
    iso15118.trace("  source_address:   %02x %02x %02x %02x %02x %02x", cm_atten_char_response.source_address[0], cm_atten_char_response.source_address[1], cm_atten_char_response.source_address[2], cm_atten_char_response.source_address[3], cm_atten_char_response.source_address[4], cm_atten_char_response.source_address[5]);
    iso15118.trace("  run_id:           %02x %02x %02x %02x %02x %02x %02x %02x", cm_atten_char_response.run_id[0], cm_atten_char_response.run_id[1], cm_atten_char_response.run_id[2], cm_atten_char_response.run_id[3], cm_atten_char_response.run_id[4], cm_atten_char_response.run_id[5], cm_atten_char_response.run_id[6], cm_atten_char_response.run_id[7]);
    iso15118.trace("  source_id:        %02x %02x %02x %02x %02x %02x %02x %02x...", cm_atten_char_response.source_id[0], cm_atten_char_response.source_id[1], cm_atten_char_response.source_id[2], cm_atten_char_response.source_id[3], cm_atten_char_response.source_id[4], cm_atten_char_response.source_id[5], cm_atten_char_response.source_id[6], cm_atten_char_response.source_id[7]);
    iso15118.trace("  resp_id:          %02x %02x %02x %02x %02x %02x %02x %02x...", cm_atten_char_response.resp_id[0], cm_atten_char_response.resp_id[1], cm_atten_char_response.resp_id[2], cm_atten_char_response.resp_id[3], cm_atten_char_response.resp_id[4], cm_atten_char_response.resp_id[5], cm_atten_char_response.resp_id[6], cm_atten_char_response.resp_id[7]);
}

void SLAC::log_cm_slac_match_confirmation(const CM_SLACMatchConfirmation &cm_slac_match_confirmation)
{
    iso15118.trace("CM_SLAC_MATCH.CNF:");
    log_homeplug_message_header(cm_slac_match_confirmation.header);
    iso15118.trace("  application_type: %02x", cm_slac_match_confirmation.application_type);
    iso15118.trace("  security_type:    %02x", cm_slac_match_confirmation.security_type);
    iso15118.trace("  mvf_length:       %04x", cm_slac_match_confirmation.mvf_length);
    iso15118.trace("  pev_id:           %02x %02x %02x %02x %02x %02x %02x %02x...", cm_slac_match_confirmation.pev_id[0], cm_slac_match_confirmation.pev_id[1], cm_slac_match_confirmation.pev_id[2], cm_slac_match_confirmation.pev_id[3], cm_slac_match_confirmation.pev_id[4], cm_slac_match_confirmation.pev_id[5], cm_slac_match_confirmation.pev_id[6], cm_slac_match_confirmation.pev_id[7]);
    iso15118.trace("  pev_mac:          %02x %02x %02x %02x %02x %02x", cm_slac_match_confirmation.pev_mac[0], cm_slac_match_confirmation.pev_mac[1], cm_slac_match_confirmation.pev_mac[2], cm_slac_match_confirmation.pev_mac[3], cm_slac_match_confirmation.pev_mac[4], cm_slac_match_confirmation.pev_mac[5]);
    iso15118.trace("  evse_id:          %02x %02x %02x %02x %02x %02x %02x %02x...", cm_slac_match_confirmation.evse_id[0], cm_slac_match_confirmation.evse_id[1], cm_slac_match_confirmation.evse_id[2], cm_slac_match_confirmation.evse_id[3], cm_slac_match_confirmation.evse_id[4], cm_slac_match_confirmation.evse_id[5], cm_slac_match_confirmation.evse_id[6], cm_slac_match_confirmation.evse_id[7]);
    iso15118.trace("  evse_mac:         %02x %02x %02x %02x %02x %02x", cm_slac_match_confirmation.evse_mac[0], cm_slac_match_confirmation.evse_mac[1], cm_slac_match_confirmation.evse_mac[2], cm_slac_match_confirmation.evse_mac[3], cm_slac_match_confirmation.evse_mac[4], cm_slac_match_confirmation.evse_mac[5]);
    iso15118.trace("  run_id:           %02x %02x %02x %02x %02x %02x %02x %02x...", cm_slac_match_confirmation.run_id[0], cm_slac_match_confirmation.run_id[1], cm_slac_match_confirmation.run_id[2], cm_slac_match_confirmation.run_id[3], cm_slac_match_confirmation.run_id[4], cm_slac_match_confirmation.run_id[5], cm_slac_match_confirmation.run_id[6], cm_slac_match_confirmation.run_id[7]);
    iso15118.trace("  nid:              %02x %02x %02x %02x %02x %02x %02x", cm_slac_match_confirmation.nid[0], cm_slac_match_confirmation.nid[1], cm_slac_match_confirmation.nid[2], cm_slac_match_confirmation.nid[3], cm_slac_match_confirmation.nid[4], cm_slac_match_confirmation.nid[5], cm_slac_match_confirmation.nid[6]);
    iso15118.trace("  nmk:              %02x %02x %02x %02x %02x %02x %02x %02x...", cm_slac_match_confirmation.nmk[0], cm_slac_match_confirmation.nmk[1], cm_slac_match_confirmation.nmk[2], cm_slac_match_confirmation.nmk[3], cm_slac_match_confirmation.nmk[4], cm_slac_match_confirmation.nmk[5], cm_slac_match_confirmation.nmk[6], cm_slac_match_confirmation.nmk[7]);
}

void SLAC::log_cm_slac_match_request(const CM_SLACMatchRequest &cm_slac_match_request)
{
    iso15118.trace("CM_SLAC_MATCH.REQ:");
    log_homeplug_message_header(cm_slac_match_request.header);
    iso15118.trace("  application_type: %02x", cm_slac_match_request.application_type);
    iso15118.trace("  security_type:    %02x", cm_slac_match_request.security_type);
    iso15118.trace("  mvf_length:       %04x", cm_slac_match_request.mvf_length);
    iso15118.trace("  pev_id:           %02x %02x %02x %02x %02x %02x %02x %02x...", cm_slac_match_request.pev_id[0], cm_slac_match_request.pev_id[1], cm_slac_match_request.pev_id[2], cm_slac_match_request.pev_id[3], cm_slac_match_request.pev_id[4], cm_slac_match_request.pev_id[5], cm_slac_match_request.pev_id[6], cm_slac_match_request.pev_id[7]);
    iso15118.trace("  pev_mac:          %02x %02x %02x %02x %02x %02x", cm_slac_match_request.pev_mac[0], cm_slac_match_request.pev_mac[1], cm_slac_match_request.pev_mac[2], cm_slac_match_request.pev_mac[3], cm_slac_match_request.pev_mac[4], cm_slac_match_request.pev_mac[5]);
    iso15118.trace("  evse_id:          %02x %02x %02x %02x %02x %02x %02x %02x...", cm_slac_match_request.evse_id[0], cm_slac_match_request.evse_id[1], cm_slac_match_request.evse_id[2], cm_slac_match_request.evse_id[3], cm_slac_match_request.evse_id[4], cm_slac_match_request.evse_id[5], cm_slac_match_request.evse_id[6], cm_slac_match_request.evse_id[7]);
    iso15118.trace("  evse_mac:         %02x %02x %02x %02x %02x %02x", cm_slac_match_request.evse_mac[0], cm_slac_match_request.evse_mac[1], cm_slac_match_request.evse_mac[2], cm_slac_match_request.evse_mac[3], cm_slac_match_request.evse_mac[4], cm_slac_match_request.evse_mac[5]);
    iso15118.trace("  run_id:           %02x %02x %02x %02x %02x %02x %02x %02x...", cm_slac_match_request.run_id[0], cm_slac_match_request.run_id[1], cm_slac_match_request.run_id[2], cm_slac_match_request.run_id[3], cm_slac_match_request.run_id[4], cm_slac_match_request.run_id[5], cm_slac_match_request.run_id[6], cm_slac_match_request.run_id[7]);
}

void SLAC::log_cm_qualcomm_get_sw_request(const CM_QualcommGetSwRequest &cm_qualcomm_get_sw_request)
{
    iso15118.trace("CM_QUALCOMM_GET_SW.REQ:");
    log_homeplug_message_header_v0(cm_qualcomm_get_sw_request.header);
    iso15118.trace("  vendor_mme: %02x %02x %02x", cm_qualcomm_get_sw_request.vendor_mme[0], cm_qualcomm_get_sw_request.vendor_mme[1], cm_qualcomm_get_sw_request.vendor_mme[2]);
}

void SLAC::log_cm_qualcomm_get_sw_confirmation(const CM_QualcommGetSwConfirmation &log_cm_qualcomm_get_sw_confirmation)
{
    iso15118.trace("CM_QUALCOMM_GET_SW.CNF:");
    log_homeplug_message_header_v0(log_cm_qualcomm_get_sw_confirmation.header);
    iso15118.trace("  vendor_mme:     %02x %02x %02x", log_cm_qualcomm_get_sw_confirmation.vendor_mme[0], log_cm_qualcomm_get_sw_confirmation.vendor_mme[1], log_cm_qualcomm_get_sw_confirmation.vendor_mme[2]);
    iso15118.trace("  status:         %02x", log_cm_qualcomm_get_sw_confirmation.status);
    iso15118.trace("  device:         %02x", log_cm_qualcomm_get_sw_confirmation.device);
    iso15118.trace("  version_length: %02x", log_cm_qualcomm_get_sw_confirmation.version_length);
    char version_str[sizeof(log_cm_qualcomm_get_sw_confirmation.version)+1] = "\0";
    uint8_to_printable_string(log_cm_qualcomm_get_sw_confirmation.version, sizeof(log_cm_qualcomm_get_sw_confirmation.version), version_str, sizeof(version_str));
    iso15118.trace("  version:        %s", version_str);
}

void SLAC::log_cm_qualcomm_link_status_request(const CM_QualcommLinkStatusRequest &cm_qualcomm_link_status_request)
{
    iso15118.trace("CM_QUALCOMM_LINK_STATUS.REQ:");
    log_homeplug_message_header_v0(cm_qualcomm_link_status_request.header);
    iso15118.trace("  vendor_mme: %02x %02x %02x", cm_qualcomm_link_status_request.vendor_mme[0], cm_qualcomm_link_status_request.vendor_mme[1], cm_qualcomm_link_status_request.vendor_mme[2]);
}

void SLAC::log_cm_qualcomm_link_status_confirmation(const CM_QualcommLinkStatusConfirmation &cm_qualcomm_link_status_confirmation)
{
    iso15118.trace("CM_QUALCOMM_LINK_STATUS.CNF:");
    log_homeplug_message_header_v0(cm_qualcomm_link_status_confirmation.header);
    iso15118.trace("  vendor_mme: %02x %02x %02x", cm_qualcomm_link_status_confirmation.vendor_mme[0], cm_qualcomm_link_status_confirmation.vendor_mme[1], cm_qualcomm_link_status_confirmation.vendor_mme[2]);
    iso15118.trace("  link_status: %02x", cm_qualcomm_link_status_confirmation.link_status);
}

void SLAC::log_cm_qualcomm_op_attr_request(const CM_QualcommOpAttrRequest &cm_qualcomm_op_attr_request)
{
    iso15118.trace("CM_QUALCOMM_OP_ATTR.REQ:");
    log_homeplug_message_header_v0(cm_qualcomm_op_attr_request.header);
    iso15118.trace("  vendor_mme: %02x %02x %02x", cm_qualcomm_op_attr_request.vendor_mme[0], cm_qualcomm_op_attr_request.vendor_mme[1], cm_qualcomm_op_attr_request.vendor_mme[2]);
    iso15118.trace("  cookie:     %08lx", cm_qualcomm_op_attr_request.cookie);
    iso15118.trace("  report_type: %02x", cm_qualcomm_op_attr_request.report_type);
}

void SLAC::log_cm_qualcomm_op_attr_confirmation(const CM_QualcommOpAttrConfirmation &cm_qualcomm_op_attr_confirmation)
{
    iso15118.trace("CM_QUALCOMM_OP_ATTR.CNF:");
    log_homeplug_message_header_v0(cm_qualcomm_op_attr_confirmation.header);
    iso15118.trace("  success:            %04x", cm_qualcomm_op_attr_confirmation.success);
    iso15118.trace("  cookie:             %08lx", cm_qualcomm_op_attr_confirmation.cookie);
    iso15118.trace("  report_type:        %02x", cm_qualcomm_op_attr_confirmation.report_type);
    iso15118.trace("  size:               %04x", cm_qualcomm_op_attr_confirmation.size);
    char hw_platform_str[sizeof(cm_qualcomm_op_attr_confirmation.hw_platform)+1] = "\0";
    uint8_to_printable_string(cm_qualcomm_op_attr_confirmation.hw_platform, sizeof(cm_qualcomm_op_attr_confirmation.hw_platform), hw_platform_str, sizeof(hw_platform_str));
    iso15118.trace("  hw_platform:        %s", hw_platform_str);
    char sw_platform_str[sizeof(cm_qualcomm_op_attr_confirmation.sw_platform)+1] = "\0";
    uint8_to_printable_string(cm_qualcomm_op_attr_confirmation.sw_platform, sizeof(cm_qualcomm_op_attr_confirmation.sw_platform), sw_platform_str, sizeof(sw_platform_str));
    iso15118.trace("  sw_platform:        %s", sw_platform_str);
    iso15118.trace("  version_major:      %08lx", cm_qualcomm_op_attr_confirmation.version_major);
    iso15118.trace("  version_minor:      %08lx", cm_qualcomm_op_attr_confirmation.version_minor);
    iso15118.trace("  version_pib:        %08lx", cm_qualcomm_op_attr_confirmation.version_pib);
    iso15118.trace("  version_build:      %08lx", cm_qualcomm_op_attr_confirmation.version_build);
    iso15118.trace("  reserved:           %08lx", cm_qualcomm_op_attr_confirmation.reserved);
    char build_date_str[sizeof(cm_qualcomm_op_attr_confirmation.build_date)+1] = "\0";
    uint8_to_printable_string(cm_qualcomm_op_attr_confirmation.build_date, sizeof(cm_qualcomm_op_attr_confirmation.build_date), build_date_str, sizeof(build_date_str));
    iso15118.trace("  build_date:         %s", build_date_str);
    char release_type_str[sizeof(cm_qualcomm_op_attr_confirmation.release_type)+1] = "\0";
    uint8_to_printable_string(cm_qualcomm_op_attr_confirmation.release_type, sizeof(cm_qualcomm_op_attr_confirmation.release_type), release_type_str, sizeof(release_type_str));
    iso15118.trace("  release_type:       %s", release_type_str);
    iso15118.trace("  sdram_type:         %02x", cm_qualcomm_op_attr_confirmation.sdram_type);
    iso15118.trace("  reserved2:          %02x", cm_qualcomm_op_attr_confirmation.reserved2);
    iso15118.trace("  line_freq_zc:       %02x", cm_qualcomm_op_attr_confirmation.line_freq_zc);
    iso15118.trace("  sdram_size:         %08lx", cm_qualcomm_op_attr_confirmation.sdram_size);
    iso15118.trace("  authorization_mode: %02x", cm_qualcomm_op_attr_confirmation.authorization_mode);
}
