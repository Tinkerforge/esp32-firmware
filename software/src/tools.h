/* esp32-lib
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

#include "config.h"

#include "bindings/hal_common.h"

bool deadline_elapsed(uint32_t deadline_ms);
bool find_uid_by_did(TF_HalContext *hal, uint16_t device_id, char uid[7]);
bool find_uid_by_did_at_port(TF_HalContext *hal, uint16_t device_id, char port, char uid[7]);

String update_config(Config &cfg, String config_name, JsonVariant &json);

void read_efuses(uint32_t *ret_uid_numeric, char *ret_uid_string, char *ret_passphrase_string);

int check(int rc,const char *msg);

bool mount_or_format_spiffs();

String read_or_write_config_version(String &firmware_version);

int ensure_matching_firmware(TF_HalContext *hal, const char *uid, const char* name, const char *purpose, const uint8_t *firmware, size_t firmware_len, EventLog *logger, bool force = false);
