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

#include <TFGenericTCPClientPool.h>

#include "generic_tcp_client_connector_base.h"

class GenericTCPClientPoolConnector : protected GenericTCPClientConnectorBase
{
protected:
    GenericTCPClientPoolConnector(const char *event_log_prefix_override_, TFGenericTCPClientPool *pool_) :
        GenericTCPClientConnectorBase(event_log_prefix_override_), pool(pool_) {}

private:
    void connect_internal() override;
    void disconnect_internal() override;

    TFGenericTCPClientPool *pool;
};
