/*
 * Copyright (C) 2020 Erik Fleckstein <erik@tinkerforge.com>
 *
 * Redistribution and use in source and binary forms of this file,
 * with or without modification, are permitted. See the Creative
 * Commons Zero (CC0 1.0) License for more details.
 */

#include "tfp_header.h"

#include <string.h>
#include <stdio.h>

#include "endian_convert.h"
#include "base58.h"

static inline bool header_is_valid(TF_TFPHeader *header) {
    if (header->length < TF_TFP_MIN_MESSAGE_LENGTH)
        return false;

    if (header->length > TF_TFP_MAX_MESSAGE_LENGTH)
        return false;

    return true;
}

static void parse_header(TF_TFPHeader *header) {
    header->response_expected = header->seq_num & 0x08;
    header->options = header->seq_num & 0x07;
    header->seq_num >>= 4;
    header->error_code = header->flags >> 6;
    header->flags &= 0x3F;
}

bool tf_tfp_header_read(TF_TFPHeader *header, TF_PacketBuffer *buf) {
    header->uid_num = tf_packet_buffer_read_uint32_t(buf);
    header->length = tf_packet_buffer_read_uint8_t(buf);
    header->fid = tf_packet_buffer_read_uint8_t(buf);
    header->seq_num = tf_packet_buffer_read_uint8_t(buf);
    header->flags = tf_packet_buffer_read_uint8_t(buf);

    parse_header(header);

    return header_is_valid(header);
}

bool tf_tfp_header_peek(TF_TFPHeader *header, TF_PacketBuffer *buf) {
    header->uid_num = tf_packet_buffer_peek_uint32_t(buf, 0);
    header->length = tf_packet_buffer_peek_uint8_t(buf, 4);
    header->fid = tf_packet_buffer_peek_uint8_t(buf, 5);
    header->seq_num = tf_packet_buffer_peek_uint8_t(buf, 6);
    header->flags = tf_packet_buffer_peek_uint8_t(buf, 7);

    parse_header(header);

    return header_is_valid(header);
}

bool tf_tfp_header_peek_plain(TF_TFPHeader *header, uint8_t *buf) {
    uint32_t uid_num = 0;

    for (int i = 0; i < 4; ++i) {
        *(((uint8_t *)&uid_num) + i) = buf[i];
    }

    header->uid_num = tf_leconvert_uint32_from(uid_num);
    header->length = buf[4];
    header->fid = buf[5];
    header->seq_num = buf[6];
    header->flags = buf[7];

    parse_header(header);

    return header_is_valid(header);
}

void tf_tfp_header_write(TF_TFPHeader *header, uint8_t buf[8]) {
    uint32_t uid_num = tf_leconvert_uint32_to(header->uid_num);

    memcpy(buf, &uid_num, sizeof(uid_num));

    buf[4] = header->length;
    buf[5] = header->fid;
    buf[6] = (uint8_t)((header->seq_num << 4) | ((header->response_expected ? 1 : 0) << 3) | header->options);
    buf[7] = (uint8_t)(header->error_code << 6 | header->flags);
}

void tf_tfp_header_print(TF_TFPHeader *header) {
    char uid_str[7] = {0};

    tf_base58_encode(header->uid_num, uid_str);

    printf("UID %s Len %u FID %u SeqNum %u RespExp %s Opt %u Err %u Flags %u\n", uid_str,
           header->length,
           header->fid,
           header->seq_num,
           header->response_expected ? "true" : "false",
           header->options,
           header->error_code,
           header->flags);
}
