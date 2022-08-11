/*
 * Copyright (C) 2020 Erik Fleckstein <erik@tinkerforge.com>
 *
 * Redistribution and use in source and binary forms of this file,
 * with or without modification, are permitted. See the Creative
 * Commons Zero (CC0 1.0) License for more details.
 */

#ifndef TF_NET_ARDUINO_ESP32_H
#define TF_NET_ARDUINO_ESP32_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../bindings/net_common.h"
#include "../bindings/tfp_header.h"

typedef enum {
    TF_CLIENT_AUTHENTICATION_STATE_DISABLED = 0,
    TF_CLIENT_AUTHENTICATION_STATE_ENABLED,
    TF_CLIENT_AUTHENTICATION_STATE_NONCE_SENT,
    TF_CLIENT_AUTHENTICATION_STATE_DONE
} TF_ClientAuthenticationState;

#define TF_FUNCTION_GET_AUTHENTICATION_NONCE 1
#define TF_FUNCTION_AUTHENTICATE 2

#define TF_GET_AUTHENTICATION_NONCE_REQUEST_LEN  TF_TFP_HEADER_LENGTH
#define TF_GET_AUTHENTICATION_NONCE_RESPONSE_LEN TF_TFP_HEADER_LENGTH + 4

#define TF_AUTHENTICATE_REQUEST_LEN  TF_TFP_HEADER_LENGTH + 4 + 20
#define TF_AUTHENTICATE_RESPONSE_LEN TF_TFP_HEADER_LENGTH

typedef struct {
    uint32_t id;
    uint32_t fd;
    uint32_t last_recv_ms;
    uint32_t last_send_us;
    uint8_t read_buf[80];
    uint8_t read_buf_used;
    uint8_t send_buf[1400];
    uint32_t send_buf_used;
    uint8_t sends_without_progress;

    TF_ClientAuthenticationState auth_state;
    uint32_t auth_nonce;

    bool available_packet_valid;
    TF_TFPHeader available_packet;
} TF_NetClient;

typedef struct TF_Request {
    uint32_t uid_num;
    uint32_t client_id;
    uint8_t fid;
    uint8_t seq_num;
} TF_Request;

typedef struct TF_Net {
    TF_NetClient clients[TF_NET_MAX_CLIENT_COUNT];
    uint8_t clients_used;
    int server_fd;

    TF_Request open_requests[TF_NET_MAX_OPEN_REQUEST_COUNT];
    uint8_t open_request_count;
    uint16_t send_buf_timeout_us;
    uint32_t recv_timeout_ms;
    const char* auth_secret;
    uint32_t next_auth_nonce;
} TF_Net;

int tf_net_create(TF_Net *net, const char* listen_addr, uint16_t port, const char* auth_secret);
int tf_net_destroy(TF_Net *net);

#endif
