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

// Section start and end labels have no type and cannot be accessed safely, but their addresses can be used.
struct _LabelWithNoData;

// 0x3f400000 - 0x3f7fffff flash rodata
extern _LabelWithNoData _rodata_start;
extern _LabelWithNoData _rodata_end;

// 0x3f800000 - 0x3fbfffff PSRAM
extern _LabelWithNoData _ext_ram_bss_start;

// 0x3ffae000 - 0x3fffffff SRAM
extern _LabelWithNoData _data_start;
extern _LabelWithNoData _noinit_start;
extern _LabelWithNoData _bss_start;
extern _LabelWithNoData _heap_start;

// 0x40080000 - 0x4009ffff IRAM
extern _LabelWithNoData _iram_start;

// 0x400c2000 - 0x40bfffff flash text
extern _LabelWithNoData _text_start;

inline bool address_is_in_rodata(const void *ptr)
{
    // Everything in rodata has lower addresses than all other memories.
    // Checking against the upper address is enough.
    return ptr < reinterpret_cast<const void *>(&_rodata_end);
}

void check_memory_assumptions();
