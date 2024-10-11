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

#include "header_logger.h"
#include "memory.h"

// 0x3f800000 - 0x3fbfffff PSRAM
extern char _ext_ram_bss_start;

// 0x3ffae000 - 0x3fffffff SRAM
extern char _data_start;
extern char _noinit_start;
extern char _bss_start;
extern char _heap_start;

// 0x40080000 - 0x4009ffff IRAM
extern char _iram_start;

// 0x400c2000 - 0x40bfffff flash text
extern char _text_start;

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
