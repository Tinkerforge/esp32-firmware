/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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

#include <strings.h>
#include "ocpp21/CertStore21.h"

namespace Iso15118RootPolicy {

// HUB20-21-007: SHA-256 issuerKeyHash (RFC 6960), NOT
// certificate/SPKI fingerprint, subject name or a configurable root label.
inline bool productive(const OcppCertHashData21 &root)
{
    static constexpr const char *keys[] = {
        "f3a72a6da6ce3f524cddb0935c5814266318e68b57afa36a1934fedfdb906402",
        "b2fd0b3417aa6f596b2b219915e4f6764e14b2502221771177149500a903f7c7",
        "0333d0fe04eb2d8f598edd0d0e01da3542980d890fcefeaada8eacbb61ff9e62",
    };
    for (const char *key : keys) {
        if (strncasecmp(root.issuer_key_hash, key, sizeof(root.issuer_key_hash)) == 0) {
            return true;
        }
    }
    return false;
}

inline bool allowed(const Ocpp21::CertEntry &entry, bool private_environment)
{
    using Ocpp21::CertGroup;
    if ((entry.group == CertGroup::V2GChain) || (entry.group == CertGroup::V2G20Chain)) {
        return entry.has_anchor && (!private_environment || !productive(entry.anchor_root));
    }
    // Apply by key identity even if a productive root was installed with a
    // different ISO root type. Keep storage/inventory independent of use.
    if ((entry.group == CertGroup::V2GRoot) || (entry.group == CertGroup::OEMRoot) || (entry.group == CertGroup::MORoot)) {
        return !private_environment || !productive(entry.hash);
    }
    return true;
}

}
