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

#if MODULE_EM_V1_AVAILABLE()
#include "bindings/bricklet_warp_energy_manager.h"

#define WEM_DATA_STATUS_OK                  TF_WARP_ENERGY_MANAGER_DATA_STATUS_OK
#define WEM_DATA_SDATUS_SD_ERROR            TF_WARP_ENERGY_MANAGER_DATA_STATUS_SD_ERROR
#define WEM_DATA_SDATUS_LFS_ERROR           TF_WARP_ENERGY_MANAGER_DATA_STATUS_LFS_ERROR
#define WEM_DATA_SDATUS_QUEUE_FULL          TF_WARP_ENERGY_MANAGER_DATA_STATUS_QUEUE_FULL
#define WEM_DATA_STATUS_DATE_OUT_OF_RANGE   TF_WARP_ENERGY_MANAGER_DATA_STATUS_DATE_OUT_OF_RANGE
#define WEM_FORMAT_STATUS_OK                TF_WARP_ENERGY_MANAGER_FORMAT_STATUS_OK

#elif MODULE_EM_V2_AVAILABLE()
#include "bindings/bricklet_warp_energy_manager_v2.h"

#define WEM_DATA_STATUS_OK                  TF_WARP_ENERGY_MANAGER_V2_DATA_STATUS_OK
#define WEM_DATA_SDATUS_SD_ERROR            TF_WARP_ENERGY_MANAGER_V2_DATA_STATUS_SD_ERROR
#define WEM_DATA_SDATUS_LFS_ERROR           TF_WARP_ENERGY_MANAGER_V2_DATA_STATUS_LFS_ERROR
#define WEM_DATA_SDATUS_QUEUE_FULL          TF_WARP_ENERGY_MANAGER_V2_DATA_STATUS_QUEUE_FULL
#define WEM_DATA_STATUS_DATE_OUT_OF_RANGE   TF_WARP_ENERGY_MANAGER_V2_DATA_STATUS_DATE_OUT_OF_RANGE
#define WEM_FORMAT_STATUS_OK                TF_WARP_ENERGY_MANAGER_V2_FORMAT_STATUS_OK

#endif
