/* esp32-firmware
 * Copyright (C) 2026 Frederic Henrichs <frederic@tinkerforge.com>
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

#include "nfc_dummy.h"

#include "generated/module_dependencies.h"

void NfcDummy::setup()
{
    nfc.register_backend(this);
    initialized = true;
}

bool NfcDummy::get_tag_id(uint8_t index, uint8_t *tag_type, uint8_t *tag_id, uint8_t *tag_id_length, uint32_t *last_seen)
{
    return false;
}

void NfcDummy::check_state()
{
}
