/*
 * Copyright (C) 2020 Erik Fleckstein <erik@tinkerforge.com>
 *
 * Redistribution and use in source and binary forms of this file,
 * with or without modification, are permitted. See the Creative
 * Commons Zero (CC0 1.0) License for more details.
 */

#ifndef TF_TFP_H
#define TF_TFP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "spitfp.h"
#include "macros.h"
#include "packet_buffer.h"
#include "tfp_header.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*TF_CallbackHandler)(void *device, uint8_t fid, TF_PacketBuffer *payload);

typedef struct TF_TFP {
    void *hal;
    TF_SPITFP *spitfp;
    void *device;

    uint32_t uid;

    uint32_t error_count_frame;
    uint32_t error_count_unexpected;

    uint8_t next_sequence_number;
    uint8_t waiting_for_fid; // 0 if waiting for nothing
    uint8_t waiting_for_length; // includes tfp, but not spitfp header, (to be comparable against length field in the tfp header); 0 if waiting for nothing
    uint8_t waiting_for_sequence_number; // 0 if waiting for nothing

    TF_CallbackHandler cb_handler;
    bool needs_callback_tick;
} TF_TFP;

// Don't declare the create function here. If we depend on TF_HAL * (even if forward declared) this collides with the
// required forward declaration in hal_common.h.
// We just declare the function in hal_common.c (the only caller), as well as in tfp.c before the implementation to silence the missing prototype warning.
//int tf_tfp_create(TF_TFP *tfp, TF_HAL *hal, uint8_t port_id) TF_ATTRIBUTE_NONNULL_ALL TF_ATTRIBUTE_WARN_UNUSED_RESULT;
int tf_tfp_destroy(TF_TFP *tfp) TF_ATTRIBUTE_NONNULL_ALL TF_ATTRIBUTE_WARN_UNUSED_RESULT;

void tf_tfp_prepare_send(TF_TFP *tfp, uint8_t fid, uint8_t payload_size, uint8_t response_size, bool response_expected) TF_ATTRIBUTE_NONNULL_ALL;
uint8_t *tf_tfp_get_payload_buffer(TF_TFP *tfp) TF_ATTRIBUTE_NONNULL_ALL;

int tf_tfp_transmit_packet(TF_TFP *tfp, bool response_expected, uint32_t deadline_us, uint8_t *error_code) TF_ATTRIBUTE_NONNULL_ALL TF_ATTRIBUTE_WARN_UNUSED_RESULT;
void tf_tfp_packet_processed(TF_TFP *tfp) TF_ATTRIBUTE_NONNULL_ALL;
int tf_tfp_finish_send(TF_TFP *tfp, int previous_result, uint32_t deadline_us) TF_ATTRIBUTE_NONNULL_ALL TF_ATTRIBUTE_WARN_UNUSED_RESULT;

int tf_tfp_get_error(uint8_t error_code) TF_ATTRIBUTE_WARN_UNUSED_RESULT;

int tf_tfp_callback_tick(TF_TFP *tfp, uint32_t deadline_us) TF_ATTRIBUTE_NONNULL_ALL TF_ATTRIBUTE_WARN_UNUSED_RESULT;

void tf_tfp_inject_packet(TF_TFP *tfp, TF_TFPHeader *header, uint8_t *packet) TF_ATTRIBUTE_NONNULL_ALL;

#ifdef __cplusplus
}
#endif

#endif
