/*
 * Copyright (C) 2020 Erik Fleckstein <erik@tinkerforge.com>
 *
 * Redistribution and use in source and binary forms of this file,
 * with or without modification, are permitted. See the Creative
 * Commons Zero (CC0 1.0) License for more details.
 */

#ifndef TF_PACKET_BUFFER_H
#define TF_PACKET_BUFFER_H

#include <stdbool.h>
#include <stdint.h>
#include "macros.h"

//TODO: fix circular includes, then use TF_SPITFP_MAX_MESSAGE_LENGTH + 1
//#include "spitfp.h"
#define TF_PACKET_BUFFER_SIZE 84

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TF_PacketBuffer {
    uint8_t start; // index of first valid element
    uint8_t end; // index of first invalid element
    uint8_t buffer[TF_PACKET_BUFFER_SIZE];
} TF_PacketBuffer;

uint8_t tf_packet_buffer_get_size(TF_PacketBuffer *pb) TF_ATTRIBUTE_NONNULL_ALL;
uint8_t tf_packet_buffer_get_used(TF_PacketBuffer *pb) TF_ATTRIBUTE_NONNULL_ALL;
uint8_t tf_packet_buffer_get_free(TF_PacketBuffer *pb) TF_ATTRIBUTE_NONNULL_ALL;
bool tf_packet_buffer_is_empty(TF_PacketBuffer *pb) TF_ATTRIBUTE_NONNULL_ALL;
bool tf_packet_buffer_is_full(TF_PacketBuffer *pb) TF_ATTRIBUTE_NONNULL_ALL;
bool tf_packet_buffer_push(TF_PacketBuffer *pb, const uint8_t data) TF_ATTRIBUTE_NONNULL_ALL;
void tf_packet_buffer_remove(TF_PacketBuffer *pb, const uint8_t num) TF_ATTRIBUTE_NONNULL_ALL;
bool tf_packet_buffer_pop(TF_PacketBuffer *pb, uint8_t *data) TF_ATTRIBUTE_NONNULL_ALL;
bool tf_packet_buffer_peek(TF_PacketBuffer *pb, uint8_t *data) TF_ATTRIBUTE_NONNULL_ALL;
bool tf_packet_buffer_peek_offset(TF_PacketBuffer *pb, uint8_t *data, uint8_t offset) TF_ATTRIBUTE_NONNULL_ALL;
bool tf_packet_buffer_poke(TF_PacketBuffer *pb, uint8_t data) TF_ATTRIBUTE_NONNULL_ALL;
bool tf_packet_buffer_poke_offset(TF_PacketBuffer *pb, uint8_t data, uint8_t offset) TF_ATTRIBUTE_NONNULL_ALL;
void tf_packet_buffer_create(TF_PacketBuffer *pb) TF_ATTRIBUTE_NONNULL_ALL;
void tf_packet_buffer_print(TF_PacketBuffer *pb) TF_ATTRIBUTE_NONNULL_ALL;
bool tf_packet_buffer_free_array_view(TF_PacketBuffer *pb, uint8_t length, uint8_t **first_chunk, uint8_t *first_len, uint8_t **second_chunk, uint8_t *second_len) TF_ATTRIBUTE_NONNULL_ALL;

void tf_packet_buffer_pop_n(TF_PacketBuffer *pb, uint8_t *dest, uint8_t count) TF_ATTRIBUTE_NONNULL_ALL;
void tf_packet_buffer_peek_offset_n(TF_PacketBuffer *pb, uint8_t *dest, uint8_t count, uint8_t offset) TF_ATTRIBUTE_NONNULL_ALL;

int8_t tf_packet_buffer_read_int8_t(TF_PacketBuffer *pb) TF_ATTRIBUTE_NONNULL_ALL;
uint8_t tf_packet_buffer_read_uint8_t(TF_PacketBuffer *pb) TF_ATTRIBUTE_NONNULL_ALL;
uint16_t tf_packet_buffer_read_uint16_t(TF_PacketBuffer *pb) TF_ATTRIBUTE_NONNULL_ALL;
uint32_t tf_packet_buffer_read_uint32_t(TF_PacketBuffer *pb) TF_ATTRIBUTE_NONNULL_ALL;
uint64_t tf_packet_buffer_read_uint64_t(TF_PacketBuffer *pb) TF_ATTRIBUTE_NONNULL_ALL;
int16_t tf_packet_buffer_read_int16_t(TF_PacketBuffer *pb) TF_ATTRIBUTE_NONNULL_ALL;
int32_t tf_packet_buffer_read_int32_t(TF_PacketBuffer *pb) TF_ATTRIBUTE_NONNULL_ALL;
int64_t tf_packet_buffer_read_int64_t(TF_PacketBuffer *pb) TF_ATTRIBUTE_NONNULL_ALL;
char tf_packet_buffer_read_char(TF_PacketBuffer *pb) TF_ATTRIBUTE_NONNULL_ALL;
float tf_packet_buffer_read_float(TF_PacketBuffer *pb) TF_ATTRIBUTE_NONNULL_ALL;
bool tf_packet_buffer_read_bool(TF_PacketBuffer *pb) TF_ATTRIBUTE_NONNULL_ALL;
void tf_packet_buffer_read_bool_array(TF_PacketBuffer *pb, bool* dest, uint16_t count) TF_ATTRIBUTE_NONNULL_ALL;

int8_t tf_packet_buffer_peek_int8_t(TF_PacketBuffer *pb, uint8_t offset) TF_ATTRIBUTE_NONNULL_ALL;
uint8_t tf_packet_buffer_peek_uint8_t(TF_PacketBuffer *pb, uint8_t offset) TF_ATTRIBUTE_NONNULL_ALL;
uint16_t tf_packet_buffer_peek_uint16_t(TF_PacketBuffer *pb, uint8_t offset) TF_ATTRIBUTE_NONNULL_ALL;
uint32_t tf_packet_buffer_peek_uint32_t(TF_PacketBuffer *pb, uint8_t offset) TF_ATTRIBUTE_NONNULL_ALL;
uint64_t tf_packet_buffer_peek_uint64_t(TF_PacketBuffer *pb, uint8_t offset) TF_ATTRIBUTE_NONNULL_ALL;
int16_t tf_packet_buffer_peek_int16_t(TF_PacketBuffer *pb, uint8_t offset) TF_ATTRIBUTE_NONNULL_ALL;
int32_t tf_packet_buffer_peek_int32_t(TF_PacketBuffer *pb, uint8_t offset) TF_ATTRIBUTE_NONNULL_ALL;
int64_t tf_packet_buffer_peek_int64_t(TF_PacketBuffer *pb, uint8_t offset) TF_ATTRIBUTE_NONNULL_ALL;
char tf_packet_buffer_peek_char(TF_PacketBuffer *pb, uint8_t offset) TF_ATTRIBUTE_NONNULL_ALL;
float tf_packet_buffer_peek_float(TF_PacketBuffer *pb, uint8_t offset) TF_ATTRIBUTE_NONNULL_ALL;
bool tf_packet_buffer_peek_bool(TF_PacketBuffer *pb, uint8_t offset) TF_ATTRIBUTE_NONNULL_ALL;
void tf_packet_buffer_peek_bool_array(TF_PacketBuffer *pb, bool* dest, uint16_t count, uint8_t offset) TF_ATTRIBUTE_NONNULL_ALL;

#ifdef __cplusplus
}
#endif

#endif
