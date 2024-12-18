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

#include "memory.h"

#include "header_logger.h"

// The memory areas can't be checked with static asserts at compile time
// because their addresses will be determined at link time.
void check_memory_assumptions()
{
    if (&_rodata_end > &_ext_ram_bss_start) header_printfln("memory_tools", "_rodata_end after _ext_ram_bss_start");
    if (&_rodata_end > &_data_start       ) header_printfln("memory_tools", "_rodata_end after _data_start");
    if (&_rodata_end > &_noinit_start     ) header_printfln("memory_tools", "_rodata_end after _noinit_start");
    if (&_rodata_end > &_bss_start        ) header_printfln("memory_tools", "_rodata_end after _bss_start");
    if (&_rodata_end > &_heap_start       ) header_printfln("memory_tools", "_rodata_end after _heap_start");
    if (&_rodata_end > &_iram_start       ) header_printfln("memory_tools", "_rodata_end after _iram_start");
    if (&_rodata_end > &_text_start       ) header_printfln("memory_tools", "_rodata_end after _text_start");
}
