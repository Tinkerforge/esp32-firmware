/*
 * Copyright (C) 2020 Erik Fleckstein <erik@tinkerforge.com>
 *
 * Redistribution and use in source and binary forms of this file,
 * with or without modification, are permitted. See the Creative
 * Commons Zero (CC0 1.0) License for more details.
 */

#ifndef TF_HAL_ARDUINO_ESP32_ETHERNET_BRICK_H
#define TF_HAL_ARDUINO_ESP32_ETHERNET_BRICK_H

#include <stdbool.h>
#include <stdint.h>
#include <SPI.h>

#include "bindings/hal_common.h"
#include "bindings/macros.h"

struct TF_HAL {
    SPISettings spi_settings;
    SPIClass *hspi;
    TF_HALCommon hal_common;
};

int tf_hal_create(TF_HAL *hal) TF_ATTRIBUTE_NONNULL_ALL;
int tf_hal_destroy(TF_HAL *hal) TF_ATTRIBUTE_NONNULL_ALL;

#endif
