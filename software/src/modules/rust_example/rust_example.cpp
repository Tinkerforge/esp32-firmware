/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
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

#include "rust_example.h"
#include "modules.h"

/**
 * Functions from the rust library can be prototyped via the
 * extern "C" prefix.
*/
extern "C" void rust_println();

void RustExample::pre_setup() {}

void RustExample::setup() {
    rust_println();
    included_in_header();
    takes_int(5);
    logger.printfln("%s", returns_cstring());
    call_c_func();
    initialized = true;
}

void RustExample::register_urls() {}

void RustExample::loop() {}

extern "C" void c_function() {
    logger.printfln("This function was called from rust");
}

extern "C" void rust_logger(const char *str) {
    logger.printfln(str);
}