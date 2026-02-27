/* esp32-firmware
 * Copyright (C) 2026 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>
#include <freertos/projdefs.h>
#include <freertos/semphr.h>

constexpr BaseType_t pdFAIL_nowarn        = pdFAIL;         // pdFAIL expands to an old-style cast that is also useless
constexpr BaseType_t pdPASS_nowarn        = pdPASS;         // pdPASS expands to an old-style cast that is also useless
constexpr TickType_t portMAX_DELAY_nowarn = portMAX_DELAY;  // portMAX_DELAY expands to an old-style cast

inline SemaphoreHandle_t xSemaphoreCreateBinaryStatic_nowarn(StaticSemaphore_t *sem)
{
    return xSemaphoreCreateBinaryStatic(sem);               // Macro uses old-style casts
}

inline BaseType_t xSemaphoreGive_nowarn(SemaphoreHandle_t sem_handle)
{
    return xSemaphoreGive(sem_handle);                      // Macro uses an old-style cast that is also useless
}
