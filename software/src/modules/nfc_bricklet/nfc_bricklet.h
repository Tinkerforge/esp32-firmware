/* esp32-firmware
 * Copyright (C) 2020-2026 Erik Fleckstein <erik@tinkerforge.com>
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

#include "device_module.h"
#include "bindings/bricklet_nfc.h"
#include "options.h"
#include "modules/nfc/nfc.h"

class NFCBricklet final : public DeviceModule<TF_NFC,
                                              tf_nfc_create,
                                              tf_nfc_get_bootloader_mode,
                                              tf_nfc_reset,
                                              tf_nfc_destroy,
                                              OPTIONS_PRODUCT_ID_IS_WARP2() || OPTIONS_PRODUCT_ID_IS_WARP3() || OPTIONS_PRODUCT_ID_IS_WARP4() || OPTIONS_PRODUCT_ID_IS_ELTAKO()>,
                          public INfcBackend
{
public:
    NFCBricklet();

    void setup() override;

    void setup_nfc();

    // INfcBackend implementation
    bool get_tag_id(uint8_t index, uint8_t *tag_type, uint8_t *tag_id, uint8_t *tag_id_length, uint32_t *last_seen) override;
    void check_state() override;
};
