/*
 * Copyright (C) 2020 Erik Fleckstein <erik@tinkerforge.com>
 *
 * Redistribution and use in source and binary forms of this file,
 * with or without modification, are permitted. See the Creative
 * Commons Zero (CC0 1.0) License for more details.
 */

#ifndef TF_SPITFP_H
#define TF_SPITFP_H

#include <stdbool.h>
#include <stdint.h>

#include "packet_buffer.h"
#include "macros.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TF_SPITFPStateMachine {
    uint32_t deadline_us;

    union {
        struct { uint8_t _unused; } idle;

        struct {
            uint8_t seq_num_to_send;
            bool packet_received;
            uint8_t bytes_to_send;
            uint8_t send_buf_offset;
        } transceive;

        struct {
            uint32_t inner_deadline_us;
            uint8_t seq_num;
            bool packet_received;
        } wait_for_ack;

        struct { uint8_t _unused; } receive;

        struct { uint8_t _unused; } build_ack;
    } info;

    uint8_t state;
} TF_SPITFPStateMachine;

struct TF_HAL;

typedef struct TF_SPITFP {
    struct TF_HAL *hal;

    uint8_t last_sequence_number_seen;
    uint8_t last_sequence_number_acked;
    uint8_t last_sequence_number_sent;
    uint8_t last_sequence_number_given_to_tfp;

    uint32_t error_count_checksum;
    uint32_t error_count_frame;

    uint8_t port_id;

    uint8_t send_buf[TF_SPITFP_MAX_MESSAGE_LENGTH];
    TF_PacketBuffer recv_buf;

    struct TF_SPITFPStateMachine state;
} TF_SPITFP;

void tf_spitfp_create(TF_SPITFP *spitfp, struct TF_HAL *hal, uint8_t port_id);

uint8_t *tf_spitfp_get_send_payload_buffer(TF_SPITFP *spitfp) TF_ATTRIBUTE_NONNULL_ALL;
TF_PacketBuffer *tf_spitfp_get_receive_buffer(TF_SPITFP *spitfp) TF_ATTRIBUTE_NONNULL_ALL;
#define TF_RETRANSMISSION -2
#define TF_NEW_PACKET -1
uint8_t tf_spitfp_build_packet(TF_SPITFP *spitfp, int8_t seq_num) TF_ATTRIBUTE_NONNULL_ALL;
void tf_spitfp_packet_processed(TF_SPITFP *spitfp) TF_ATTRIBUTE_NONNULL_ALL;

#define TF_TICK_PACKET_RECEIVED 1
#define TF_TICK_PACKET_SENT 2
#define TF_TICK_AGAIN 4
#define TF_TICK_TIMEOUT 8
#define TF_TICK_SLEEP 16
#define TF_TICK_IN_TRANSCEIVE 32

int tf_spitfp_tick(TF_SPITFP *spitfp, uint32_t deadline_us) TF_ATTRIBUTE_NONNULL_ALL TF_ATTRIBUTE_WARN_UNUSED_RESULT;

#ifdef __cplusplus
}
#endif

#endif
