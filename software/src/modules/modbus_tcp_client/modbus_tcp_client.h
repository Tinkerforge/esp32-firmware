/* esp32-firmware
 * Copyright (C) 2024 Matthias Bolte <matthias@tinkerforge.com>
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

#include <TFModbusTCPClientPool.h>

#include "module.h"
#include "tools.h"

class ModbusTCPClient final : public IModule
{
public:
    ModbusTCPClient() {}

    void setup() override;
    void loop() override;

    TFModbusTCPClientPool *get_pool();

private:
    TFModbusTCPClientPool pool;
    dns_gethostbyname_addrtype_lwip_ctx_async_data dns_data;
};
