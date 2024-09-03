/* esp32-firmware
 * Copyright (C) 2024 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include "module_available.h"

#if MODULE_ENERGY_MANAGER_AVAILABLE()
#include "bindings/bricklet_warp_energy_manager.h"

#define TF_EM_DATA_STATUS_OK         TF_WARP_ENERGY_MANAGER_DATA_STATUS_OK
#define TF_EM_DATA_SDATUS_SD_ERROR   TF_WARP_ENERGY_MANAGER_DATA_STATUS_SD_ERROR
#define TF_EM_DATA_SDATUS_LFS_ERROR  TF_WARP_ENERGY_MANAGER_DATA_STATUS_LFS_ERROR
#define TF_EM_DATA_SDATUS_QUEUE_FULL TF_WARP_ENERGY_MANAGER_DATA_STATUS_QUEUE_FULL

#elif 0
#include "bindings/errors.h"

#define TF_EM_DATA_STATUS_OK         42
#define TF_EM_DATA_SDATUS_SD_ERROR   43
#define TF_EM_DATA_SDATUS_LFS_ERROR  44
#define TF_EM_DATA_SDATUS_QUEUE_FULL 45
#endif
