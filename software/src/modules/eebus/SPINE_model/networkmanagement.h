/* esp32-firmware
 * Copyright (C) 2025 Julius Dill <julius@tinkerforge.com>
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


#include "build.h"
#include "commondatatypes.h"
#include "tools.h"
#include <ArduinoJson.h> // Include this even though its already included in TFJson.h but compiler got angry with me so just to be safe
#include <TFJson.h>
#include <map>
#include <optional>

struct NetworkManagementDeviceDescriptionDataType {
    std::optional<DeviceAddressType> deviceAddress;
};