/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "config.h"

#include "bindings/hal_common.h"

bool deadline_elapsed(uint32_t deadline_ms);

void read_efuses(uint32_t *ret_uid_num, char *ret_uid_str, char *ret_passphrase);

int check(int rc, const char *msg);

bool mount_or_format_spiffs(void);

String read_config_version();

int ensure_matching_firmware(TF_TFP *tfp, const char *name, const char *purpose, const uint8_t *firmware, size_t firmware_len, EventLog *logger, bool force);

int compare_version(uint8_t left_major, uint8_t left_minor, uint8_t left_patch,
                    uint8_t right_major, uint8_t right_minor, uint8_t right_patch);

bool clock_synced(struct timeval *out_tv_now);

uint32_t timestamp_minutes();

bool for_file_in(const char *dir, bool (*callback)(File *open_file), bool skip_directories = true);

void remove_directory(const char *path);
