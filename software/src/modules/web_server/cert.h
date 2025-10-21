/* esp32-firmware
 * Copyright (C) 2025 Olaf Lüke <olaf@tinkerforge.com>
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

#include <cstdint>

#include "mbedtls/x509_crt.h"

class Cert
{
private:
    bool fill_cert(mbedtls_x509write_cert *mbed_cert);
    bool key_and_sign(mbedtls_x509write_cert *mbed_cert);
    bool write_cert_and_key();
    bool generate();

public:
    Cert() {};

    bool read();
    void log();

    unsigned char crt[768];
    unsigned char key[512];
    uint16_t crt_length = 0;
    uint16_t key_length = 0;
};
