/* esp32-firmware
 * Copyright (C) 2023 Erik Fleckstein <erik@tinkerforge.com>
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

#include <memory>

#include "module.h"
#include "config.h"

#define MAX_CERTS 8
#define MAX_CERT_ID (MAX_CERTS - 1)
#define MAX_CERT_SIZE 4027
#define MAX_CERT_NAME 32

class Certs final : public IModule
{
public:
    Certs(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    std::unique_ptr<unsigned char[]> get_cert(uint8_t cert_id, size_t *out_cert_len);

private:
    void update_state();

    Config state_certs_prototype;
    ConfigRoot state;
    ConfigRoot add;
    ConfigRoot remove;
};
