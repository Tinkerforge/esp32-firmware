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

#include "module.h"
#include "config.h"
#include "options.h"

class DeviceName final : public IModule
{
public:
    DeviceName(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    void updateDisplayType();
#if OPTIONS_PRODUCT_ID_IS_WARP() || OPTIONS_PRODUCT_ID_IS_WARP2() || OPTIONS_PRODUCT_ID_IS_WARP3() || OPTIONS_PRODUCT_ID_IS_WARP4() || OPTIONS_PRODUCT_ID_IS_ELTAKO()
    String get20CharDisplayType();
#endif

    ConfigRoot name;
    ConfigRoot display_name;
};
