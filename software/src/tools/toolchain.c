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

// Discard atexit handlers by overwriting the registration function.
// C++ requires destructors to be called at exit for objects with
// static storage duration, but as the ESP32 firmware never exits,
// the handlers are never called anyway.

int __cxa_atexit(void (* /*func*/) (void *), void * /*arg*/, void * /*dso_handle*/)
{
    return 0;
}

int __register_exitproc(int, void (* /*func*/) (void), void * /*arg*/, void * /*dso_handle*/)
{
    return 0;
}

// Override functions that call any exit handlers.
// This allows the atexit data to be optimized out.

void __cxa_finalize(void *)
{
}

void __call_exitprocs(int, void *)
{
}
