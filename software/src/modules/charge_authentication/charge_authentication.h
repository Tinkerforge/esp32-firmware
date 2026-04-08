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

#pragma once

#include "module.h"
#include "config.h"
#include "modules/cm_networking/cm_networking_defs.h"
#include "generated/module_available.h"

#define LAST_AUTH_LIST_LENGTH 3

class ChargeAuthentication final : public IModule {
public:
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    // Finds the user matching the given authentication info.
    // Returns the user_id (>= 0) if a known user was found,
    // or -1 if the tag was not recognized or auth type is unsupported.
    int16_t find_user(const cm_auth_info &info);

    ConfigRoot last_seen_authentications;

private:
};

#include "generated/module_available_end.h"