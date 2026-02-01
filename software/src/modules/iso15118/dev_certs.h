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

// Embedded test certificates for development/testing
// Generated with: src/modules/iso15118/tools/certs/generate_certs.sh
// Certificate chain: SECC Leaf -> CPO Sub-CA2 -> CPO Sub-CA1

// -----------------------------------------------------------------------------
// ISO 15118-2 certificates (secp256r1/prime256v1, TLS 1.2)
// [V2G2-006] ECC-based using secp256r1 curve
// [V2G2-007] Key length for ECC shall be 256 bit
// -----------------------------------------------------------------------------
extern const char dev_cert_chain_pem_iso2[];
extern const char dev_private_key_pem_iso2[];

// -----------------------------------------------------------------------------
// ISO 15118-20 certificates (secp521r1, TLS 1.3)
// [V2G20-2674] Primary: secp521r1 with ECDSA signature algorithm
// [V2G20-2675] Key length for ECC shall be 521 bit
// -----------------------------------------------------------------------------
extern const char dev_cert_chain_pem_iso20[];
extern const char dev_private_key_pem_iso20[];

// -----------------------------------------------------------------------------
// Backwards compatibility aliases (default to ISO 15118-2)
// -----------------------------------------------------------------------------
#define dev_cert_chain_pem  dev_cert_chain_pem_iso2
#define dev_private_key_pem dev_private_key_pem_iso2
