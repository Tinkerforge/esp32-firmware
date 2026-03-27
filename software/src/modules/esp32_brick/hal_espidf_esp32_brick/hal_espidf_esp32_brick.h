/*
 * Copyright (C) 2020 Erik Fleckstein <erik@tinkerforge.com>
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
 *
 * Redistribution and use in source and binary forms of this file,
 * with or without modification, are permitted. See the Creative
 * Commons Zero (CC0 1.0) License for more details.
 */

#if TF_HAL_ESPIDF == 1

#ifndef TF_HAL_ESPIDF_ESP32_BRICK_H
#define TF_HAL_ESPIDF_ESP32_BRICK_H

#include <stdbool.h>
#include <stdint.h>
#include <driver/spi_master.h>

#include "bindings/hal_common.h"
#include "bindings/macros.h"

struct TF_HAL {
    spi_device_handle_t hspi_dev;
    spi_device_handle_t vspi_dev;
    TF_HALCommon hal_common;
};

int tf_hal_create(TF_HAL *hal) TF_ATTRIBUTE_NONNULL_ALL;
int tf_hal_destroy(TF_HAL *hal) TF_ATTRIBUTE_NONNULL_ALL;

#endif

#endif
