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

#pragma once

#include <mbedtls/aes.h>
#include <mbedtls/bignum.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/pk.h>
#include <mbedtls/sha256.h>

class mbedtls_aes_raii final
{
public:
    inline  mbedtls_aes_raii() {esp_aes_init(&ctx);}
    inline ~mbedtls_aes_raii() {esp_aes_free(&ctx);}

    mbedtls_aes_raii           (const mbedtls_aes_raii &) = delete;
    mbedtls_aes_raii &operator=(const mbedtls_aes_raii &) = delete;

    esp_aes_context ctx;
};

class mbedtls_ctr_drbg_raii final
{
public:
    inline  mbedtls_ctr_drbg_raii() {mbedtls_ctr_drbg_init(&ctx);}
    inline ~mbedtls_ctr_drbg_raii() {mbedtls_ctr_drbg_free(&ctx);}

    mbedtls_ctr_drbg_raii           (const mbedtls_ctr_drbg_raii &) = delete;
    mbedtls_ctr_drbg_raii &operator=(const mbedtls_ctr_drbg_raii &) = delete;

    mbedtls_ctr_drbg_context ctx;
};

class mbedtls_entropy_raii final
{
public:
    inline  mbedtls_entropy_raii() {mbedtls_entropy_init(&ctx);}
    inline ~mbedtls_entropy_raii() {mbedtls_entropy_free(&ctx);}

    mbedtls_entropy_raii           (const mbedtls_entropy_raii &) = delete;
    mbedtls_entropy_raii &operator=(const mbedtls_entropy_raii &) = delete;

    mbedtls_entropy_context ctx;
};

class mbedtls_mpi_raii final
{
public:
    inline  mbedtls_mpi_raii() {mbedtls_mpi_init(&mpi);}
    inline ~mbedtls_mpi_raii() {mbedtls_mpi_free(&mpi);}

    mbedtls_mpi_raii           (const mbedtls_mpi_raii &) = delete;
    mbedtls_mpi_raii &operator=(const mbedtls_mpi_raii &) = delete;

    mbedtls_mpi mpi;
};

class mbedtls_pk_raii final
{
public:
    inline  mbedtls_pk_raii() {mbedtls_pk_init(&ctx);}
    inline ~mbedtls_pk_raii() {mbedtls_pk_free(&ctx);}

    mbedtls_pk_raii           (const mbedtls_pk_raii &) = delete;
    mbedtls_pk_raii &operator=(const mbedtls_pk_raii &) = delete;

    mbedtls_pk_context ctx;
};

class mbedtls_sha256_raii final
{
public:
    inline  mbedtls_sha256_raii() {mbedtls_sha256_init(&ctx);}
    inline ~mbedtls_sha256_raii() {mbedtls_sha256_free(&ctx);}

    mbedtls_sha256_raii           (const mbedtls_sha256_raii &) = delete;
    mbedtls_sha256_raii &operator=(const mbedtls_sha256_raii &) = delete;

    mbedtls_sha256_context ctx;
};
