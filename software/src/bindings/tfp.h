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
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*TF_TFP_CallbackHandler)(void *device, uint8_t fid, TF_PacketBuffer *payload);

typedef struct TF_TFP {
    TF_SPITFP *spitfp;
    void *device;

    uint32_t uid_num;

    uint32_t error_count_frame;
    uint32_t error_count_unexpected;

    uint8_t next_sequence_number;
    uint8_t waiting_for_fid; // 0 if waiting for nothing
    uint8_t waiting_for_seq_num; // 0 if waiting for nothing
    bool send_enumerate_request;

    TF_TFP_CallbackHandler cb_handler;

    uint16_t device_id;
#if TF_NET_ENABLE != 0
    uint8_t spitfp_timeout_counter;
#endif
} TF_TFP;

void tf_tfp_create(TF_TFP *tfp, uint32_t uid_num, uint16_t device_id, TF_SPITFP *spitfp) TF_ATTRIBUTE_NONNULL_ALL;

void tf_tfp_prepare_send(TF_TFP *tfp, uint8_t fid, uint8_t payload_size, bool response_expected) TF_ATTRIBUTE_NONNULL_ALL;
uint8_t *tf_tfp_get_send_payload_buffer(TF_TFP *tfp) TF_ATTRIBUTE_NONNULL_ALL;
TF_PacketBuffer *tf_tfp_get_receive_buffer(TF_TFP *tfp) TF_ATTRIBUTE_NONNULL_ALL;

int tf_tfp_send_packet(TF_TFP *tfp, bool response_expected, uint32_t deadline_us, uint8_t *error_code, uint8_t *length) TF_ATTRIBUTE_NONNULL_ALL TF_ATTRIBUTE_WARN_UNUSED_RESULT;
void tf_tfp_packet_processed(TF_TFP *tfp) TF_ATTRIBUTE_NONNULL_ALL;
int tf_tfp_finish_send(TF_TFP *tfp, int previous_result, uint32_t deadline_us) TF_ATTRIBUTE_NONNULL_ALL TF_ATTRIBUTE_WARN_UNUSED_RESULT;

int tf_tfp_get_error(uint8_t error_code) TF_ATTRIBUTE_WARN_UNUSED_RESULT;

int tf_tfp_callback_tick(TF_TFP *tfp, uint32_t deadline_us) TF_ATTRIBUTE_NONNULL_ALL TF_ATTRIBUTE_WARN_UNUSED_RESULT;

void tf_tfp_inject_packet(TF_TFP *tfp, TF_TFPHeader *header, uint8_t *packet) TF_ATTRIBUTE_NONNULL_ALL;

#ifdef __cplusplus
}
#endif

#endif
