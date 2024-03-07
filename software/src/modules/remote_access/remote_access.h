/* esp32-firmware
 * Copyright (C) 2024 Frederic Henrichs <frederic@tinkerforge.com>
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

#include "module.h"
#include "config.h"

#include <WireGuard-ESP32.h>

class RemoteAccess final : public IModule {
public:
    RemoteAccess() {};

    void pre_setup() override;
    void setup() override;
    void register_urls() override;

private:
    void resolve_management();
    void login();
    void connect_management();

    WireGuard management;

    String jwt;

    ConfigRoot config;
    ConfigRoot management_connection;
};
