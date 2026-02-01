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

const char dev_cert_chain_pem[] =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIB2TCCAX+gAwIBAgICMDwwCgYIKoZIzj0EAwIwRTESMBAGA1UEAwwJQ1BPU3Vi\n"
    "Q0EyMQ0wCwYDVQQKDARXQVJQMQswCQYDVQQGEwJERTETMBEGCgmSJomT8ixkARkW\n"
    "A1YyRzAeFw0yNjAxMzEyMzU5MTFaFw0yNjA0MDEyMzU5MTFaMEQxETAPBgNVBAMM\n"
    "CFNFQ0NDZXJ0MQ0wCwYDVQQKDARXQVJQMQswCQYDVQQGEwJERTETMBEGCgmSJomT\n"
    "8ixkARkWA0NQTzBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABFDhizmHj8B4LXJ/\n"
    "erfilpdlfGfUSKIxCGINBw+/9RV6Q+pbL36mbORtLbd6Qaegg8ShKKCjEmW2ruR+\n"
    "Kw8Z4bajYDBeMAwGA1UdEwEB/wQCMAAwDgYDVR0PAQH/BAQDAgOIMB0GA1UdDgQW\n"
    "BBQX1V0KkTm2jrSyBHBKlMo1Iu1yczAfBgNVHSMEGDAWgBT/5zZ1NJfteF/Mp3cU\n"
    "RtnaFg6KojAKBggqhkjOPQQDAgNIADBFAiEAg3FuP9yPaVxYOrVqsUIl2Z5Ixcym\n"
    "xE9KsBeRkPJqjPECIHLC7jRnGRMgZ51D45cmQlQ9xJr8twin0ptRK/7dTxZJ\n"
    "-----END CERTIFICATE-----\n"
    "-----BEGIN CERTIFICATE-----\n"
    "MIIB4DCCAYagAwIBAgICMDswCgYIKoZIzj0EAwIwRTESMBAGA1UEAwwJQ1BPU3Vi\n"
    "Q0ExMQ0wCwYDVQQKDARXQVJQMQswCQYDVQQGEwJERTETMBEGCgmSJomT8ixkARkW\n"
    "A1YyRzAeFw0yNjAxMzEyMzU5MTFaFw0yNzAxMzEyMzU5MTFaMEUxEjAQBgNVBAMM\n"
    "CUNQT1N1YkNBMjENMAsGA1UECgwEV0FSUDELMAkGA1UEBhMCREUxEzARBgoJkiaJ\n"
    "k/IsZAEZFgNWMkcwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAAQ0UC+/7ezUUx+w\n"
    "f4Y+qssSWyfby00IVIvz/Xe889MgThUCY7Mm2b3Qc3nqDjWNTTBGtlEMdpPJHvog\n"
    "cffmzx+Ko2YwZDASBgNVHRMBAf8ECDAGAQH/AgEAMA4GA1UdDwEB/wQEAwIBBjAd\n"
    "BgNVHQ4EFgQU/+c2dTSX7XhfzKd3FEbZ2hYOiqIwHwYDVR0jBBgwFoAURHVADvhh\n"
    "4FilF2qFmkaLHZPXysAwCgYIKoZIzj0EAwIDSAAwRQIhAPtu+OZU0YXaRFBFm9Zj\n"
    "VucMLfTx35GHYOCUl92xvdKBAiAGqOLHFJyKd+Rb3W7Z6DaEHl4SAVKw/GpsPJhx\n"
    "Dtg+TQ==\n"
    "-----END CERTIFICATE-----\n"
    "-----BEGIN CERTIFICATE-----\n"
    "MIIB3zCCAYagAwIBAgICMDowCgYIKoZIzj0EAwIwRTESMBAGA1UEAwwJVjJHUm9v\n"
    "dENBMQ0wCwYDVQQKDARXQVJQMQswCQYDVQQGEwJERTETMBEGCgmSJomT8ixkARkW\n"
    "A1YyRzAeFw0yNjAxMzEyMzU5MTFaFw0zMDAxMzAyMzU5MTFaMEUxEjAQBgNVBAMM\n"
    "CUNQT1N1YkNBMTENMAsGA1UECgwEV0FSUDELMAkGA1UEBhMCREUxEzARBgoJkiaJ\n"
    "k/IsZAEZFgNWMkcwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAASaPafslVd7kPwM\n"
    "w9F32/hPnShYDuPAKweUFlHGPdBA5VMUcY5BolXtL2vA1dxHo6LM0cE2vlP8s842\n"
    "T5gBFTe9o2YwZDASBgNVHRMBAf8ECDAGAQH/AgEBMA4GA1UdDwEB/wQEAwIBBjAd\n"
    "BgNVHQ4EFgQURHVADvhh4FilF2qFmkaLHZPXysAwHwYDVR0jBBgwFoAUthvQAQkq\n"
    "ljjJG4ADyg3uK99/Qj4wCgYIKoZIzj0EAwIDRwAwRAIgMQnLmCL192pv1B/4fpwK\n"
    "X1Oau5xQXSxO5gzwScXZ1KECIB0ipMkxnRcpLah7ViOQTjYArwmBgW9wVF5Hysn8\n"
    "S04g\n"
    "-----END CERTIFICATE-----\n";

// SECC leaf private key (unencrypted EC key)
const char dev_private_key_pem[] =
    "-----BEGIN EC PRIVATE KEY-----\n"
    "MHcCAQEEIBZX6LB0MLEsoDIWjFo9EC9/YJfTMulZi4Vm8IrR6KvBoAoGCCqGSM49\n"
    "AwEHoUQDQgAEUOGLOYePwHgtcn96t+KWl2V8Z9RIojEIYg0HD7/1FXpD6lsvfqZs\n"
    "5G0tt3pBp6CDxKEooKMSZbau5H4rDxnhtg==\n"
    "-----END EC PRIVATE KEY-----\n";
