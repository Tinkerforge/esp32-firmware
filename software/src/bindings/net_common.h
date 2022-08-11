/*
 * Copyright (C) 2020 Erik Fleckstein <erik@tinkerforge.com>
 *
 * Redistribution and use in source and binary forms of this file,
 * with or without modification, are permitted. See the Creative
 * Commons Zero (CC0 1.0) License for more details.
 */

#ifndef TF_NET_COMMON_H
#define TF_NET_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "config.h"

#include "macros.h"
#include "packet_buffer.h"

#include "tfp_header.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TF_Net TF_Net;

int tf_net_tick(TF_Net *net);
bool tf_net_get_available_tfp_header(TF_Net *net, TF_TFPHeader *header, int *packet_id);
int tf_net_get_packet(TF_Net *net, uint8_t packet_id, uint8_t *buf);
int tf_net_drop_packet(TF_Net *net, uint8_t packet_id);
void tf_net_send_packet(TF_Net *net, TF_TFPHeader *header, uint8_t *buf);

#ifdef __cplusplus
}
#endif

#endif
