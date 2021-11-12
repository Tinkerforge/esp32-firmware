/*
 * Copyright (C) 2012-2014, 2016, 2018, 2020 Matthias Bolte <matthias@tinkerforge.com>
 * Copyright (C) 2014 Olaf Lüke <olaf@tinkerforge.com>
 * Copyright (C) 2021 Erik Fleckstein <erik@tinkerforge.com>
 *
 * Redistribution and use in source and binary forms of this file,
 * with or without modification, are permitted. See the Creative
 * Commons Zero (CC0 1.0) License for more details.
 */

#ifndef TF_HMAC_H
#define TF_HMAC_H

#include <stdint.h>

#include "sha1.h"

#ifdef __cplusplus
extern "C" {
#endif

void tf_hmac_sha1(const uint8_t *secret, int secret_length,
               const uint8_t *data, int data_length,
               uint8_t digest[TF_SHA1_DIGEST_LENGTH]);

#ifdef __cplusplus
}
#endif

#endif
