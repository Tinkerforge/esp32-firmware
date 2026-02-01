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

// Embedded test certificates for development/testing
// Generated with: src/modules/iso15118/tools/certs/generate_certs.sh
// Certificate chain: SECC Leaf -> CPO Sub-CA2 -> CPO Sub-CA1

#include "dev_certs.h"

// =============================================================================
// ISO 15118-2 Certificates (secp256r1/prime256v1, TLS 1.2)
// =============================================================================
// [V2G2-006] ECC-based using secp256r1 curve
// [V2G2-007] Key length for ECC shall be 256 bit

const char dev_cert_chain_pem_iso2[] =
    // SECC Leaf Certificate
    "-----BEGIN CERTIFICATE-----\n"
    "MIIB2DCCAX+gAwIBAgICMDwwCgYIKoZIzj0EAwIwRTESMBAGA1UEAwwJQ1BPU3Vi\n"
    "Q0EyMQ0wCwYDVQQKDARXQVJQMQswCQYDVQQGEwJERTETMBEGCgmSJomT8ixkARkW\n"
    "A1YyRzAeFw0yNjAyMDExNzI3MjJaFw0yNjA0MDIxNzI3MjJaMEQxETAPBgNVBAMM\n"
    "CFNFQ0NDZXJ0MQ0wCwYDVQQKDARXQVJQMQswCQYDVQQGEwJERTETMBEGCgmSJomT\n"
    "8ixkARkWA0NQTzBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABBvZHtQ6unYmFaKs\n"
    "VOGwnIuMnoKvfzOt+eFyrftn7BxrzMrtZoqmh4rzlTEBQ9HPYfxthM8MYxhoNpTe\n"
    "y8WrCuSjYDBeMAwGA1UdEwEB/wQCMAAwDgYDVR0PAQH/BAQDAgOIMB0GA1UdDgQW\n"
    "BBS1PisdZanRHu0fHw3q0JQtTvhE7jAfBgNVHSMEGDAWgBRw5WAWVnyNv4z9xGcA\n"
    "vZ4e846eujAKBggqhkjOPQQDAgNHADBEAiBEU0TZSKaEcye/VnSHmii6ENA/yzVw\n"
    "g3etXakeSbqZ0QIgCgs6IWKkie2dpKg64ykFc5aOFupgEEho5mtoJfY0uuM=\n"
    "-----END CERTIFICATE-----\n"
    // CPO Sub-CA 2
    "-----BEGIN CERTIFICATE-----\n"
    "MIIB3zCCAYagAwIBAgICMDswCgYIKoZIzj0EAwIwRTESMBAGA1UEAwwJQ1BPU3Vi\n"
    "Q0ExMQ0wCwYDVQQKDARXQVJQMQswCQYDVQQGEwJERTETMBEGCgmSJomT8ixkARkW\n"
    "A1YyRzAeFw0yNjAyMDExNzI3MjJaFw0yNzAyMDExNzI3MjJaMEUxEjAQBgNVBAMM\n"
    "CUNQT1N1YkNBMjENMAsGA1UECgwEV0FSUDELMAkGA1UEBhMCREUxEzARBgoJkiaJ\n"
    "k/IsZAEZFgNWMkcwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAAR/osQYTNbSE0Be\n"
    "O6iS1+Gxg1v67sfVlFs+5nzK5iYLxm7FC5lVdL0hPiFlPpDJYEFt4Ih+0KvqBTPf\n"
    "Fyf+rsSzo2YwZDASBgNVHRMBAf8ECDAGAQH/AgEAMA4GA1UdDwEB/wQEAwIBBjAd\n"
    "BgNVHQ4EFgQUcOVgFlZ8jb+M/cRnAL2eHvOOnrowHwYDVR0jBBgwFoAUXqggGtu+\n"
    "k/qNoDXjDMJKLOzliFkwCgYIKoZIzj0EAwIDRwAwRAIgUIDJiaYrSp/JEy+7Rath\n"
    "oOKXpktbyzbXm5LBRb40cYECIFiNInmJfLJTq4nIJP2w47E+4JmsCN3vqiCjbEAu\n"
    "mt1W\n"
    "-----END CERTIFICATE-----\n"
    // CPO Sub-CA 1
    "-----BEGIN CERTIFICATE-----\n"
    "MIIB4DCCAYagAwIBAgICMDowCgYIKoZIzj0EAwIwRTESMBAGA1UEAwwJVjJHUm9v\n"
    "dENBMQ0wCwYDVQQKDARXQVJQMQswCQYDVQQGEwJERTETMBEGCgmSJomT8ixkARkW\n"
    "A1YyRzAeFw0yNjAyMDExNzI3MjJaFw0zMDAxMzExNzI3MjJaMEUxEjAQBgNVBAMM\n"
    "CUNQT1N1YkNBMTENMAsGA1UECgwEV0FSUDELMAkGA1UEBhMCREUxEzARBgoJkiaJ\n"
    "k/IsZAEZFgNWMkcwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAAQyNuNaTdj0LkUq\n"
    "5GGtWrA4Y4jK2+cdHV/XggkBT3BlinB7JxW2752qRuO1DdOoP7B1wvXNHyPV6PPK\n"
    "wOGUNeR8o2YwZDASBgNVHRMBAf8ECDAGAQH/AgEBMA4GA1UdDwEB/wQEAwIBBjAd\n"
    "BgNVHQ4EFgQUXqggGtu+k/qNoDXjDMJKLOzliFkwHwYDVR0jBBgwFoAU18GVSwgb\n"
    "m7isDI4PAlVUG4UidE0wCgYIKoZIzj0EAwIDSAAwRQIgfcl8T9Ix37nfWa8zMbQ1\n"
    "Pw+ifyHOXu4uXoKHzeBE4bcCIQDvzYHHwL8Bdur1qb6SJumC64tfrBAQN5BLDU2X\n"
    "/m2eWQ==\n"
    "-----END CERTIFICATE-----\n";

// SECC leaf private key for ISO 15118-2 (unencrypted EC key, secp256r1)
const char dev_private_key_pem_iso2[] =
    "-----BEGIN EC PRIVATE KEY-----\n"
    "MHcCAQEEINo3/idRllKXKOykZ4JjoQry9hQnX+YyzkEF5Ew9+ezZoAoGCCqGSM49\n"
    "AwEHoUQDQgAEG9ke1Dq6diYVoqxU4bCci4yegq9/M6354XKt+2fsHGvMyu1miqaH\n"
    "ivOVMQFD0c9h/G2EzwxjGGg2lN7LxasK5A==\n"
    "-----END EC PRIVATE KEY-----\n";

// =============================================================================
// ISO 15118-20 Certificates (secp521r1, TLS 1.3)
// =============================================================================
// [V2G20-2674] Primary: secp521r1 with ECDSA signature algorithm
// [V2G20-2675] Key length for ECC shall be 521 bit

const char dev_cert_chain_pem_iso20[] =
    // SECC Leaf Certificate
    "-----BEGIN CERTIFICATE-----\n"
    "MIICYTCCAcKgAwIBAgICV0wwCgYIKoZIzj0EAwIwRTESMBAGA1UEAwwJQ1BPU3Vi\n"
    "Q0EyMQ0wCwYDVQQKDARXQVJQMQswCQYDVQQGEwJERTETMBEGCgmSJomT8ixkARkW\n"
    "A1YyRzAeFw0yNjAyMDExNzI3MjNaFw0yNjA0MDIxNzI3MjNaMEQxETAPBgNVBAMM\n"
    "CFNFQ0NDZXJ0MQ0wCwYDVQQKDARXQVJQMQswCQYDVQQGEwJERTETMBEGCgmSJomT\n"
    "8ixkARkWA0NQTzCBmzAQBgcqhkjOPQIBBgUrgQQAIwOBhgAEADjEUo1mbW484bOh\n"
    "oiLUjRuYrrtj/TVmo8DEnOaqoau/yzKN/wRGNiAXMOZBpqOvxFO2c/f5yK65s+M1\n"
    "cKgZZKEQAHq/p2QAewNVcW9UEBbEBJJYFiUPBbl33my7TkHoZhQ9bSqRPZwHao4U\n"
    "TjDMEojFHnHWGAETYDefyWBDO7RFG7tEo2AwXjAMBgNVHRMBAf8EAjAAMA4GA1Ud\n"
    "DwEB/wQEAwIDiDAdBgNVHQ4EFgQUV2mlCxyb0e4zsk+8UG1NhZBeHaYwHwYDVR0j\n"
    "BBgwFoAU2X8av694TpjAAzI4S5tU47lAKoowCgYIKoZIzj0EAwIDgYwAMIGIAkIA\n"
    "qYk7aEICIU+D1XvMfKFurJramfVJ5tDOr8YYGv7Bc0UNbE7QGIoBee/d2HGLGvS+\n"
    "fnIRRzACoXmZpE14xNSQ03QCQgFROSb/GlKzEpF0S+2bpdyoSfIgwdGHslpLW0aj\n"
    "4Bzj1EVwBAf3SV9cKk7dLsrbvcF3zOKBBkzneYTjCY3PUqcK0w==\n"
    "-----END CERTIFICATE-----\n"
    // CPO Sub-CA 2
    "-----BEGIN CERTIFICATE-----\n"
    "MIICaDCCAcmgAwIBAgICV0swCgYIKoZIzj0EAwIwRTESMBAGA1UEAwwJQ1BPU3Vi\n"
    "Q0ExMQ0wCwYDVQQKDARXQVJQMQswCQYDVQQGEwJERTETMBEGCgmSJomT8ixkARkW\n"
    "A1YyRzAeFw0yNjAyMDExNzI3MjNaFw0yNzAyMDExNzI3MjNaMEUxEjAQBgNVBAMM\n"
    "CUNQT1N1YkNBMjENMAsGA1UECgwEV0FSUDELMAkGA1UEBhMCREUxEzARBgoJkiaJ\n"
    "k/IsZAEZFgNWMkcwgZswEAYHKoZIzj0CAQYFK4EEACMDgYYABAGM6O2BBgJZ80oE\n"
    "hOb01jFumXyng2zkf/0+jcitZaGFOkWJrdB5XqLMu4iWSGLJoyNfuf88iOY6lxqK\n"
    "/cNg62GRkwGdlNpqhQ/XQxAexhKyxiYF0j2FpvmxL+psAttLTrdkVBxWUT4j+hip\n"
    "nmLastD67liGxIbF4tFK5EY0uc0xp2jO9KNmMGQwEgYDVR0TAQH/BAgwBgEB/wIB\n"
    "ADAOBgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYEFNl/Gr+veE6YwAMyOEubVOO5QCqK\n"
    "MB8GA1UdIwQYMBaAFCvNUH9Qf0tj4EbEzMWlpiAGuXERMAoGCCqGSM49BAMCA4GM\n"
    "ADCBiAJCAZ4vyX+HfVU509eac6FKezO/2GXSkXBzyWUWFJg9O9PAIxk4Yvo+knnW\n"
    "WPfol/kvxYI0xmHvwh86vyo9TZgP6Y+BAkIBnf2HTuSCf6F3oKJqbY22XvA4H+FB\n"
    "ra1kC7z/prUT9k48yE2ll3sIXUNU/Bl6kj2sV7NSzSUNvsqC461e5FOnC3E=\n"
    "-----END CERTIFICATE-----\n"
    // CPO Sub-CA 1
    "-----BEGIN CERTIFICATE-----\n"
    "MIICZzCCAcmgAwIBAgICV0owCgYIKoZIzj0EAwIwRTESMBAGA1UEAwwJVjJHUm9v\n"
    "dENBMQ0wCwYDVQQKDARXQVJQMQswCQYDVQQGEwJERTETMBEGCgmSJomT8ixkARkW\n"
    "A1YyRzAeFw0yNjAyMDExNzI3MjNaFw0zMDAxMzExNzI3MjNaMEUxEjAQBgNVBAMM\n"
    "CUNQT1N1YkNBMTENMAsGA1UECgwEV0FSUDELMAkGA1UEBhMCREUxEzARBgoJkiaJ\n"
    "k/IsZAEZFgNWMkcwgZswEAYHKoZIzj0CAQYFK4EEACMDgYYABACsQBDgsh4n6mZJ\n"
    "m7Jdl05wbou6WbFnHAKcRith2OXi1eKXg+YyjGk5MFIrCTw/7xuTgyCZJl6KjnxX\n"
    "DnOGvNVx+QGJUuygnNSpJCrCmccm5Ti7NGu1dmMzyTQhsAYqlWtF9OpTYEt9wJuE\n"
    "NUYC4dvVUDDNAnnAqOSUPTyUp+/HxjvRaaNmMGQwEgYDVR0TAQH/BAgwBgEB/wIB\n"
    "ATAOBgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYEFCvNUH9Qf0tj4EbEzMWlpiAGuXER\n"
    "MB8GA1UdIwQYMBaAFOI8x59+EZ2IxdtPniGtFU8vYURkMAoGCCqGSM49BAMCA4GL\n"
    "ADCBhwJBShSS1VYmwMfa0Ton0ACt7D1/G9IuSLtAy1xKzTY1C1v+jomRAylvE1qa\n"
    "iZEe30oUl2wa3t8nEW1PSJCalKgxUUgCQgCGc+mxlNztbhsXwvANX6aWJEhIAOIU\n"
    "gBG7/0ivhqjsGMS/3syYiWjVp7bjW2QhCNPamUS2hgdaunb+wYtV+z51Lw==\n"
    "-----END CERTIFICATE-----\n";

// SECC leaf private key for ISO 15118-20 (unencrypted EC key, secp521r1)
const char dev_private_key_pem_iso20[] =
    "-----BEGIN EC PRIVATE KEY-----\n"
    "MIHcAgEBBEIATG/JzL+VeE8d2/hq2Dsz1TNERSZgkaiSyjwCpd6n/Uvl/nojBFvs\n"
    "cZPvN5nQNQ1peul072wDzyiGkdVRm5uRwj6gBwYFK4EEACOhgYkDgYYABAA4xFKN\n"
    "Zm1uPOGzoaIi1I0bmK67Y/01ZqPAxJzmqqGrv8syjf8ERjYgFzDmQaajr8RTtnP3\n"
    "+ciuubPjNXCoGWShEAB6v6dkAHsDVXFvVBAWxASSWBYlDwW5d95su05B6GYUPW0q\n"
    "kT2cB2qOFE4wzBKIxR5x1hgBE2A3n8lgQzu0RRu7RA==\n"
    "-----END EC PRIVATE KEY-----\n";
