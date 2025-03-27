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

#include "module.h"
#include "config.h"

#include "cbv2g/common/exi_bitstream.h"
#include "cbv2g/iso_20/iso20_CommonMessages_Decoder.h"
#include "cbv2g/iso_20/iso20_CommonMessages_Encoder.h"
#include "cbv2g/iso_20/iso20_AC_Decoder.h"
#include "cbv2g/iso_20/iso20_AC_Encoder.h"

class ISO20 final
{
public:
    ISO20(){}
    void pre_setup();
    void handle_bitstream(exi_bitstream *exi);

    ConfigRoot api_state;

};
