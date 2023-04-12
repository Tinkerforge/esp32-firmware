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


/**
 * This is an example for including rust functions into the
 * Tinkerforge esp32-firmware.
 *
 * C++ integration will eventually follow.
 */

mod rust_example {
    use esp_println::println;

    // funtions need to be prefixed with extern "C" and the "no_mangle attribute"
    #[no_mangle]
    extern "C" fn rust_println() -> () {
        println!("This function prints with a newline.");
    }

    #[no_mangle]
    extern "C" fn included_in_header() -> () {
        println!("This functions got included in a header");
    }

    #[no_mangle]
    extern "C" fn takes_int(i: i32) -> () {
        println!("got the number {}", i);
    }

    #[no_mangle]
    extern "C" fn returns_cstring() -> &'static str {
        "This is returned by a rust function\0"
    }

    extern "C" {
        fn c_function() -> ();
    }

    #[no_mangle]
    extern "C" fn call_c_func() -> () {
        unsafe {
            c_function();
        }
    }
}