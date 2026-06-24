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

#include <sdkconfig.h>

#ifdef CONFIG_SECURE_BOOT

// Base libraries
#include <stdint.h>

// ESP IDF
#include <esp_efuse.h>
#include <esp_efuse_table.h>
#include <esp_random.h>
#include <esp_rom_crc.h>
#include <soc/efuse_struct.h>

// mbedTLS
#include <entropy_poll.h>
#include <mbedtls/bignum.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>

// Other libs
#include <LittleFS.h>
#include <sodium.h>

// This module
#include "./generated/module_dependencies.h"
#include "./secure_boot.h"
#include "./esp32_tools.h"

// Other project things
#include "event_log_prefix.h"
#include "tools.h" // For heap_alloc_array
#include "tools/fs.h"
#include "tools/hexdump.h"
#include "tools/mbedtls.h"
#include "tools/string_builder.h"

#include "gcc_warnings.h"

constexpr const char *cache_dir = "/cache";

/*
static void dump_field(const char *name, const uint8_t *data, size_t data_len)
{
    char buf[800];
    hexdump(data, data_len, buf, sizeof(buf), HexdumpCase::Lower);

    Serial.printf("%13s %s\n", name, buf);
}

[[gnu::noinline]]
[[gnu::unused]]
static void dump_sig_block(const ets_secure_boot_sig_block_t *sig)
{
    dump_field("magic_byte",                                     &sig->magic_byte,   sizeof(sig->magic_byte  ));
    dump_field("version",                                        &sig->version,      sizeof(sig->version     ));
    dump_field("_reserved1",                                     &sig->_reserved1,   sizeof(sig->_reserved1  ));
    dump_field("_reserved2",                                     &sig->_reserved2,   sizeof(sig->_reserved2  ));
    dump_field("image_digest",                                    sig->image_digest, sizeof(sig->image_digest));
    dump_field("key.n",                                           sig->key.n,        sizeof(sig->key.n       ));
    dump_field("key.e",        reinterpret_cast<const uint8_t *>(&sig->key.e),       sizeof(sig->key.e       ));
    dump_field("key.rinv",                                        sig->key.rinv,     sizeof(sig->key.rinv    ));
    dump_field("key.mdash",    reinterpret_cast<const uint8_t *>(&sig->key.mdash),   sizeof(sig->key.mdash   ));
    dump_field("signature",                                       sig->signature,    sizeof(sig->signature   ));
    dump_field("block_crc",    reinterpret_cast<const uint8_t *>(&sig->block_crc),   sizeof(sig->block_crc   ));
    dump_field("_padding",                                        sig->_padding,     sizeof(sig->_padding    ));
}
*/

[[gnu::noinline]]
static void log_mbedtls_error(int error, const char *msg)
{
    char error_buf[128];
    mbedtls_strerror(error, error_buf, sizeof(error_buf));
    logger.printfln("SBv2: %s: -0x%04x (%s)", msg, static_cast<unsigned>(-error), error_buf);
}

static int fallback_hw_rng(void *rng_state, unsigned char *output, size_t len)
{
    size_t olen;
    return mbedtls_hardware_poll(rng_state, output, len, &olen);
}

struct blake_ctr_ctx_t {
    uint8_t key[crypto_kdf_blake2b_KEYBYTES];
    uint32_t subkey_id_counter;
    uint32_t call_counter;
};

static int blake_ctr_drbg_for_rsa_key_func(void *data, unsigned char *output, const size_t len)
{
    static constexpr const char blake_context_for_rsa[crypto_kdf_blake2b_CONTEXTBYTES] = {'r','s','a','_','d','r','b','g'};

    blake_ctr_ctx_t *ctx = static_cast<blake_ctr_ctx_t *>(data);
    size_t offset = 0;

    ctx->call_counter++;

    while (offset < len) {
        const size_t sublen = std::min(len - offset, static_cast<size_t>(crypto_kdf_blake2b_BYTES_MAX));

        const int derive_err = crypto_kdf_blake2b_derive_from_key(output + offset, sublen, ctx->subkey_id_counter, blake_context_for_rsa, ctx->key);

        ctx->subkey_id_counter++;

        if (derive_err != 0) {
            logger.printfln("crypto_kdf_blake2b_derive_from_key failed: %i", derive_err);
            return -1;
        }

        offset += sublen;

        if (ctx->subkey_id_counter % 2048 == 0) {
            if (!esp32_poke_main_thread()) {
                return -1;
            }
        }
    }

    return 0;
}

// mbedTLS-internal function that does exactly what is needed.
// It computes X = R^2 mod N, with R being the maximum value of N + 1.
// It is "unsafe" because it doesn't do any input checks and expects N to be sane.
extern "C" int mbedtls_mpi_core_get_mont_r2_unsafe(mbedtls_mpi *X, const mbedtls_mpi *N);

[[gnu::noinline]]
static bool write_rinv(const mbedtls_rsa_context *rsa, uint8_t *output, size_t len)
{
    // rinv = R * R % N = 2^|N| * 2^|N| % N = 2^(|N| * 2) % N = 2^(3072 * 2) % N

    mbedtls_mpi_raii rinv;

    int err = mbedtls_mpi_core_get_mont_r2_unsafe(&rinv.mpi, &rsa->private_N);

    if (err != 0) {
        log_mbedtls_error(err, "mbedtls_mpi_core_get_mont_r2_unsafe failed");
        return false;
    }

    err = mbedtls_mpi_write_binary_le(&rinv.mpi, output, len);

    if (err != 0) {
        log_mbedtls_error(err, "mbedtls_mpi_write_binary_le failed");
        return false;
    }

    return true;
}

[[gnu::noinline]]
static bool write_mdash(const mbedtls_rsa_context *rsa, uint32_t *mdash)
{
    // M' = -modinv(N, 2^32) & 0xFFFFFFFF

    mbedtls_mpi_raii M;

    mbedtls_mpi_lset(&M.mpi, 1);
    mbedtls_mpi_shift_l(&M.mpi, 32);

    const int err = mbedtls_mpi_inv_mod(&M.mpi, &rsa->private_N, &M.mpi);

    if (err != 0) {
        log_mbedtls_error(err, "mbedtls_mpi_inv_mod failed");
        return false;
    }

    *mdash = -M.mpi.private_p[0];

    return true;
}

[[gnu::noinline]]
static bool write_rsa_pubkey(const mbedtls_rsa_context *rsa, ets_rsa_pubkey_t *pubkey)
{
    // key.n
    int err = mbedtls_mpi_write_binary_le(&rsa->private_N, pubkey->n, sizeof(pubkey->n));

    if (err != 0) {
        log_mbedtls_error(err, "mbedtls_mpi_write_binary_le failed for N");
        return false;
    }

    // key.e
    pubkey->e = rsa->private_E.private_p[0];

    // key.rinv
    if (!write_rinv(rsa, pubkey->rinv, sizeof(pubkey->rinv))) return false;

    // key.mdash
    if (!write_mdash(rsa, &pubkey->mdash)) return false;

    return true;
}

[[gnu::noinline]]
static bool write_sig_block(ets_secure_boot_sig_block_t *sig_block, mbedtls_rsa_context *rsa, mbedtls_f_rng_t *f_rng, void *p_rng)
{
    int err;

    // magic_byte
    sig_block->magic_byte = ETS_SECURE_BOOT_V2_SIGNATURE_MAGIC;

    // version
    sig_block->version    = ESP_SECURE_BOOT_V2_RSA;

    // _reserved1
    sig_block->_reserved1 = 0;

    // _reserved2
    sig_block->_reserved2 = 0;

    // image_digest already set

    // key
    if (!write_rsa_pubkey(rsa, &sig_block->key)) {
        return false;
    }

    // signature
    if (rsa->private_len != sizeof(sig_block->signature)) {
        logger.printfln("Length of RSA private key doesn't match signature size: %zu != %zu", rsa->private_len, sizeof(sig_block->signature));
        return false;
    }

    mbedtls_rsa_set_padding(rsa, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);
    err = mbedtls_rsa_rsassa_pss_sign_ext(rsa, f_rng, p_rng, MBEDTLS_MD_SHA256, ESP_SECURE_BOOT_DIGEST_LEN, sig_block->image_digest, 32, sig_block->signature); // IDF's espsecure module uses a salt length of 32.

    if (err != 0) {
        log_mbedtls_error(err, "RSA PSS signature operation failed");
        return false;
    }

    {
        // Convert signature from BE to LE in-place
        uint8_t *s = sig_block->signature;
        const size_t last_i = sizeof(sig_block->signature) - 1;

        for (size_t i = 0; i < sizeof(sig_block->signature) / 2; i++) {
            uint8_t tmp = s[i];
            s[i] = s[last_i - i];
            s[last_i - i] = tmp;
        }
    }

    // block_crc
    sig_block->block_crc = esp_rom_crc32_le(0, reinterpret_cast<uint8_t *>(sig_block), CRC_SIGN_BLOCK_LEN);

    // _padding
    memset(sig_block->_padding, 0, sizeof(sig_block->_padding));

    return true;
}

[[gnu::noinline]]
static bool hash_rsa_pubkey(const mbedtls_pk_context *rsa_key, uint8_t *sb_key)
{
    ets_rsa_pubkey_t rsa_pubkey;

    if (!write_rsa_pubkey(static_cast<mbedtls_rsa_context *>(rsa_key->private_pk_ctx), &rsa_pubkey)) {
        return false;
    }

    int err = mbedtls_sha256(reinterpret_cast<const uint8_t *>(&rsa_pubkey), sizeof(rsa_pubkey), sb_key, 0);

    if (err != 0) {
        log_mbedtls_error(err, "Hashing pubkey failed");
        return false;
    }

    return true;
}

[[gnu::noinline]]
static bool generate_cache_path(StringWriter *path, const uint8_t *secure_boot_hash)
{
    path->clear();
    path->puts(cache_dir);
    path->puts("/sbv2-");

    const size_t remaining_len = path->getRemainingLength();

    if (remaining_len < 32 + 4) { // Hash + ".pem"
        logger.printfln("Cache path buffer too short: %zu", remaining_len);
        return false;
    }

    const size_t hash_len = hexdump(secure_boot_hash, 16, path->getRemainingPtr(), remaining_len, HexdumpCase::Lower);

    assert(hash_len == 32);

    path->setLength(path->getLength() + hash_len);
    path->puts(".pem");

    return true;
}

[[gnu::noinline]]
static bool load_secure_boot_key(uint8_t sb_key[ESP_SECURE_BOOT_DIGEST_LEN])
{
    static_assert(EFUSE_BLK_SECURE_BOOT == 2);

    // Read bank 2
    memcpy(sb_key, const_cast<uint32_t *>(&EFUSE.blk2_rdata0.val), ESP_SECURE_BOOT_DIGEST_LEN); // Cast removes volatile, not const

    for (size_t i = 0; i < ESP_SECURE_BOOT_DIGEST_LEN; i++) {
        if (sb_key[i] != 0) {
            return true;
        }
    }

    // All zeros, Secure Boot key unset
    return false;
}

[[gnu::noinline]]
static bool load_key_from_file(std::unique_ptr<uint8_t[]> *key_buffer, size_t *key_length_out, uint8_t *sb_key)
{
    uint8_t sb_key_local[ESP_SECURE_BOOT_DIGEST_LEN];

    if (sb_key == nullptr) {
        sb_key = sb_key_local;
    }

    if (!load_secure_boot_key(sb_key)) {
        logger.printfln("Secure Boot v2 key not set, not loading cache");
        return false;
    }

    char path_buf[64];
    StringWriter path(path_buf, std::size(path_buf));

    generate_cache_path(&path, sb_key);

    if (!file_exists(LittleFS, path.getPtr())) {
        logger.printfln("Secure Boot v2 private key not cached");
        return false;
    }

    File f = LittleFS.open(path.getPtr());
    const size_t key_length = f.size();

    *key_buffer = heap_alloc_array<uint8_t>(key_length + 1); // Leave room for termination

    if (key_buffer == nullptr) {
        logger.printfln("No memory to load cached Secure Boot key");
        return false;
    }

    const size_t readlen = f.read(key_buffer->get(), key_length);

    if (readlen != key_length) {
        logger.printfln("Secure Boot v2 private key read failure");
        return false;
    }

    //logger.printfln("Read %zu bytes from %s", key_length, path.getPtr());

    *key_length_out = key_length;

    return true;
}

[[gnu::noinline]]
static bool parse_key_with_randomness(mbedtls_pk_context *rsa_key, const unsigned char *key_buffer, size_t key_length, mbedtls_f_rng_t *f_rng, void *p_rng)
{
    int err = mbedtls_pk_parse_key(rsa_key, key_buffer, key_length, nullptr, 0, f_rng, p_rng);

    if (err != 0) {
        log_mbedtls_error(err, "Failed to parse Secure Boot v2 private key");
        return false;
    }

    return true;
}

[[gnu::noinline]]
static bool parse_key_without_randomness(mbedtls_pk_context *rsa_key, const unsigned char *key_buffer, size_t key_length)
{
    // Provide randomness for blinding
    mbedtls_entropy_raii entropy; // Large
    mbedtls_ctr_drbg_raii ctr_drbg;
    mbedtls_f_rng_t *f_rng;
    void *p_rng;

    static constexpr const char *drbg_personalization = "sbv2_key_read";

    int err = mbedtls_ctr_drbg_seed(&ctr_drbg.ctx, mbedtls_entropy_func, &entropy, reinterpret_cast<const unsigned char *>(drbg_personalization), strlen(drbg_personalization));

    if (err == 0) {
        f_rng = mbedtls_ctr_drbg_random;
        p_rng = &ctr_drbg;
    } else {
        log_mbedtls_error(err, "mbedtls_ctr_drbg_seed failed");
        f_rng = fallback_hw_rng;
        p_rng = nullptr;
    }

    return parse_key_with_randomness(rsa_key, key_buffer, key_length, f_rng, p_rng);
}

[[gnu::noinline]]
bool ESP32CommonSecureBoot::load_cached_secure_boot_v2_key(mbedtls_pk_context *rsa_key, mbedtls_f_rng_t *f_rng, void *p_rng, uint8_t *sb_key)
{
    std::unique_ptr<uint8_t[]> key_buffer;
    size_t key_length;

    if (!load_key_from_file(&key_buffer, &key_length, sb_key)) {
        return false;
    }

    // Add termination
    key_buffer[key_length] = 0;
    key_length++;

    defer {
        sodium_memzero(key_buffer.get(), key_length);
    };

    // Parse key
    bool success;

    if (f_rng == nullptr) {
        success = parse_key_without_randomness(rsa_key, key_buffer.get(), key_length);
    } else {
        success = parse_key_with_randomness(rsa_key, key_buffer.get(), key_length, f_rng, p_rng);
    }

    if (!success) {
        return false;
    }

    if (sb_key != nullptr) {
        bool key_set = false;

        for (size_t i = 0; i < ESP_SECURE_BOOT_DIGEST_LEN; i++) {
            if (sb_key[i] != 0) {
                key_set = true;
                break;
            }
        }

        if (!key_set) {
            if (!hash_rsa_pubkey(rsa_key, sb_key)) {
                return false;
            }
        }
    }

    return true;
}

[[gnu::noinline]]
bool ESP32CommonSecureBoot::cache_secure_boot_v2_key(const mbedtls_pk_context *rsa_key, uint8_t *sb_key)
{
    constexpr size_t KEY_BUFFER_SIZE = 3072; // RSA-3072 should be <2500 bytes
    std::unique_ptr<uint8_t[]> key_buffer = heap_alloc_array<uint8_t>(KEY_BUFFER_SIZE);

    if (key_buffer == nullptr) {
        logger.printfln("No memory to cache Secure Boot key");
        return false;
    }

    int err = mbedtls_pk_write_key_pem(rsa_key, key_buffer.get(), KEY_BUFFER_SIZE);

    if (err != 0) {
        log_mbedtls_error(err, "mbedtls_pk_write_key_pem failed");
        return false;
    }

    const size_t key_length = strlen(reinterpret_cast<const char *>(key_buffer.get()));

    defer {
        sodium_memzero(key_buffer.get(), key_length);
    };

    // Write key to file
    {
        uint8_t sb_key_local[32];

        if (sb_key == nullptr) {
            sb_key = sb_key_local;
        }

        if (!hash_rsa_pubkey(rsa_key, sb_key)) {
            return false;
        }

        {
            char path_buf[64];
            StringWriter path(path_buf, std::size(path_buf));

            if (!generate_cache_path(&path, sb_key)) {
                return false;
            }

            File f = LittleFS.open(path.getPtr(), "w", true);
            size_t writelen = f.write(key_buffer.get(), key_length);

            if (writelen != key_length) {
                logger.printfln("Writing Secure Boot v2 private key failed");
                f.close();
                LittleFS.remove(path.getPtr());
                return false;
            }
        }

        logger.printfln("Cached Secure Boot v2 private key (%zu B)", key_length);
    }

    return true;
}

[[gnu::noinline]]
bool ESP32CommonSecureBoot::generate_secure_boot_v2_key(mbedtls_pk_context *rsa_key)
{
    // For blake2b
    if (sodium_init() < 0) {
        logger.printfln("Failed to initialize libsodium, cached Secure Boot v2 key not available");
        return false;
    }

    blake_ctr_ctx_t blake_ctr_ctx;
    memset(&blake_ctr_ctx, 0, sizeof(blake_ctr_ctx));

    defer {
        sodium_memzero(&blake_ctr_ctx, sizeof(blake_ctr_ctx));
    };

    // Retrieve device's subkey for signing
    static constexpr const char subkey_context[crypto_kdf_blake2b_CONTEXTBYTES] = {'r','s','a','s','g','k','e','y'};

    if (!esp32_common.get_subkey(blake_ctr_ctx.key, sizeof(blake_ctr_ctx.key), 0, subkey_context)) {
        logger.printfln("Subkey failed, cannot generate RSA signing key");
        return false;
    }

    // Prepare RSA key
    if (rsa_key->private_pk_ctx != nullptr) {
        logger.printfln("RSA context used?");
        mbedtls_pk_free(rsa_key);
    }

    rsa_key->private_pk_ctx = malloc(sizeof(mbedtls_rsa_context));

    if (rsa_key->private_pk_ctx == nullptr) {
        logger.printfln("No memory for RSA context");
        return false;
    }

    mbedtls_rsa_init(static_cast<mbedtls_rsa_context *>(rsa_key->private_pk_ctx));
    rsa_key->private_pk_info = mbedtls_pk_info_from_type(MBEDTLS_PK_RSA);

    constexpr int KEY_SIZE = 3072;  // Required for Secure Boot v2
    constexpr int EXPONENT = 65537; // Efficient and everyone(TM) uses it

    bool ret;
    int err;

    micros_t t_start;
    uint32_t runtime_ms;

    // Deterministically generate device-specific RSA key
    logger.printfln("Generating %i-bit RSA key", KEY_SIZE);
    t_start = now_us();
    if ((err = mbedtls_rsa_gen_key(static_cast<mbedtls_rsa_context *>(rsa_key->private_pk_ctx), blake_ctr_drbg_for_rsa_key_func, &blake_ctr_ctx, KEY_SIZE, EXPONENT)) != 0) {
        log_mbedtls_error(err, "mbedtls_rsa_gen_key failed");
        ret = false;
        goto out_err;
    }
    runtime_ms = (now_us() - t_start).as<uint32_t>() / 1000;
    logger.printfln("RSA key generated after %lums, subkey_id_counter=%lu call_counter=%lu", runtime_ms, blake_ctr_ctx.subkey_id_counter, blake_ctr_ctx.call_counter);

    return true;

out_err:
    mbedtls_pk_free(rsa_key);

    return ret;
}

[[gnu::noinline]]
bool ESP32CommonSecureBoot::generate_and_cache_secure_boot_v2_key(mbedtls_pk_context *rsa_key, uint8_t *sb_key)
{
    if (!generate_secure_boot_v2_key(rsa_key)) {
        return false;
    }

    // Continue even if the key can't be cached, as the key is still usable.
    cache_secure_boot_v2_key(rsa_key, sb_key);

    return true;
}

[[gnu::noinline]]
bool ESP32CommonSecureBoot::load_or_generate_secure_boot_v2_key(mbedtls_pk_context *rsa_key, mbedtls_f_rng_t *f_rng, void *p_rng, uint8_t *sb_key)
{
    if (load_cached_secure_boot_v2_key(rsa_key, f_rng, p_rng, sb_key)) {
        return true;
    }

    return generate_and_cache_secure_boot_v2_key(rsa_key, sb_key);
}

bool ESP32CommonSecureBoot::check_secure_boot_v2_key()
{
    mbedtls_pk_raii rsa_key;

    return load_or_generate_secure_boot_v2_key(&rsa_key.ctx, nullptr, nullptr, nullptr);
}

bool ESP32CommonSecureBoot::get_secure_boot_key(uint8_t key_digest[ESP_SECURE_BOOT_DIGEST_LEN])
{
    if (load_secure_boot_key(key_digest)) {
        return true;
    }

    mbedtls_pk_raii rsa_key;

    return load_or_generate_secure_boot_v2_key(&rsa_key.ctx, nullptr, nullptr, key_digest);
}

bool ESP32CommonSecureBoot::sb_sign_image(ets_secure_boot_sig_block_t *sig_block, const uint8_t image_digest[ESP_SECURE_BOOT_DIGEST_LEN])
{
    mbedtls_entropy_raii entropy; // Large
    mbedtls_ctr_drbg_raii ctr_drbg;

    mbedtls_f_rng_t *f_rng;
    void *p_rng;

    constexpr const char *drbg_personalization = "sbv2_sign";

    int err = mbedtls_ctr_drbg_seed(&ctr_drbg.ctx, mbedtls_entropy_func, &entropy.ctx, reinterpret_cast<const unsigned char *>(drbg_personalization), strlen(drbg_personalization));

    if (err == 0) {
        f_rng = mbedtls_ctr_drbg_random;
        p_rng = &ctr_drbg.ctx;
    } else {
        log_mbedtls_error(err, "mbedtls_ctr_drbg_seed failed");
        f_rng = fallback_hw_rng;
        p_rng = nullptr;
    }

    mbedtls_pk_raii rsa_key;

    if (!load_or_generate_secure_boot_v2_key(&rsa_key.ctx, f_rng, p_rng, nullptr)) {
        return false;
    }

    static_assert(sizeof(sig_block->image_digest) == ESP_SECURE_BOOT_DIGEST_LEN);

    if (image_digest != nullptr) {
        memcpy(sig_block->image_digest, image_digest, ESP_SECURE_BOOT_DIGEST_LEN);
    }

    if (!write_sig_block(sig_block, static_cast<mbedtls_rsa_context *>(rsa_key.ctx.private_pk_ctx), f_rng, p_rng)) {
        return false;
    }

    return true;
}

bool ESP32CommonSecureBoot::sb_sign_image_or_skip(ets_secure_boot_sig_block_t *sig_block, const uint8_t image_digest[ESP_SECURE_BOOT_DIGEST_LEN])
{
    if (!esp_secure_boot_enabled()) {
        logger.printfln("Secure Boot not enabled, not signing image");
        memset(sig_block, 0xFF, sizeof(*sig_block)); // This block is written to Flash, so fill it with 0xFF.

        return true;
    }

    return sb_sign_image(sig_block, image_digest);
}

bool ESP32CommonSecureBoot::set_secure_boot_key()
{
    if (esp_secure_boot_enabled()) {
        return true;
    }

    uint8_t secure_boot_key[ESP_SECURE_BOOT_DIGEST_LEN];

    if (!get_secure_boot_key(secure_boot_key)) {
        return false;
    }

    // Check or write encryption key
    const int key_write_protected_size = esp_efuse_get_field_size(ESP_EFUSE_WR_DIS_SECURE_BOOT_KEY);

    uint8_t secure_boot_key_fuses[ESP_SECURE_BOOT_DIGEST_LEN];

    memcpy(secure_boot_key_fuses, const_cast<uint32_t *>(&EFUSE.blk2_rdata0.val), sizeof(secure_boot_key_fuses)); // Cast removes volatile, not const

    uint32_t key_write_protected = 0;
    int err = esp_efuse_read_field_blob(ESP_EFUSE_WR_DIS_SECURE_BOOT_KEY, &key_write_protected, std::min(static_cast<size_t>(key_write_protected_size), 32u));

    logger.printfln("Secure Boot key write protected=%lu size=%i err=%i", key_write_protected, key_write_protected_size, err);

    if (key_write_protected) {
        if (memcmp(secure_boot_key_fuses, secure_boot_key, sizeof(secure_boot_key_fuses)) == 0) {
            logger.printfln("Secure Boot v2 key already set and matches");
            return true;
        } else {
            logger.printfln("Secure Boot v2 key already set and doesn't match");
            return false;
        }
    }

    for (size_t i = 0; i < std::size(secure_boot_key); i++) {
        uint8_t byte_key   = secure_boot_key[i];
        uint8_t byte_fuses = secure_boot_key_fuses[i];

        if ((~byte_key & byte_fuses) != 0) {
            logger.printfln("Secure Boot v2 key byte %zu has incorrect bits set: expected 0x%hhx, found 0x%hhx", i, byte_key, byte_fuses);
            return false;
        }

        // Mask out already set bits
        secure_boot_key[i] = byte_key & ~byte_fuses;
    }

    //esp32_common_dump_mem(secure_boot_key, sizeof(secure_boot_key));

    err = esp_efuse_write_block(EFUSE_BLK_SECURE_BOOT, secure_boot_key, 0, sizeof(secure_boot_key) * 8);

    if (err != ESP_OK) {
        logger.printfln("Writing Secure Boot v2 key failed: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
        return false;
    }

    key_write_protected = ~0lu;
    err = esp_efuse_write_field_blob(ESP_EFUSE_WR_DIS_SECURE_BOOT_KEY, &key_write_protected, std::min(static_cast<size_t>(key_write_protected_size), 32u));

    if (err != ESP_OK) {
        logger.printfln("Write-protecting Secure Boot v2 key failed: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
        return false;
    }

    return true;
}

bool ESP32CommonSecureBoot::set_secure_boot_fuse()
{
    const esp_err_t err = esp_efuse_write_field_bit(ESP_EFUSE_ABS_DONE_1);

    if (err != ESP_OK) {
        logger.printfln("Setting Secure Boot eFuse failed: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
        return false;
    }

    return true;
}

static constexpr const esp_efuse_desc_t **const efuse_lockdown_data[] = {
    //ESP_EFUSE_RD_DIS_ENCRYPT_FLASH_KEY, // Don't disable reading the Flash encryption key by software. This would break software-assisted decryption speed-ups.
    ESP_EFUSE_CONSOLE_DEBUG_DISABLE,    // Disable ROM BASIC interpreter fallback.
    ESP_EFUSE_JTAG_DISABLE,             // Disable JTAG.
    ESP_EFUSE_DISABLE_DL_ENCRYPT,       // Disable UART bootloader encryption.
    ESP_EFUSE_DISABLE_DL_DECRYPT,       // Disable UART bootloader decryption.
    ESP_EFUSE_DISABLE_DL_CACHE,         // Disable UART bootloader MMU cache (used for decryption).
    ESP_EFUSE_UART_DOWNLOAD_DIS,        // Disable UART download boot mode.

    // Four things can be read-protected:
    // Flash encryption key in BLOCK1 - must not be read-protected.
    // Secure Boot key in BLOCK2 - must not be read-protected.
    // User data in BLOCK3 - must not be read-protected.
    // Some settings in BLOCK0 - must not be read-protected.
    // As there is nothing more that can be read-protected, disallow further read-protection to avoid bricking the device.
    //ESP_EFUSE_WR_DIS_EFUSE_RD_DISABLE,      // wr_disable 0

    // The cache is required for decryption. Disallow disabling it, because that would brick the device.
    ESP_EFUSE_WR_DIS_DIS_CACHE,             // wr_disable 3, also protects MAC, MAC_CRC, DISABLE_APP_CPU, DISABLE_BT, DIS_CACHE, VOL_LEVEL_HP_INV

    ESP_EFUSE_WR_DIS_ABS_DONE_0,            // wr_disable 12, protects abstract_done_0 (Secure Boot v1), must not be set.
};

[[gnu::noinline]]
static bool batch_write_fuses(const esp_efuse_desc_t **const *fuses, size_t fuse_count)
{
    esp_err_t err;

    esp_efuse_batch_write_begin();

    for (size_t i = 0; i < fuse_count; i++) {
        err = esp_efuse_write_field_bit(fuses[i]);

        if (err != ESP_OK) {
            logger.printfln("Setting fuse %zu failed: %s (0x%04x)", i, esp_err_to_name(err), static_cast<unsigned>(err));
            esp_efuse_batch_write_cancel();
            return false;
        }
    }

    err = esp_efuse_batch_write_commit();

    if (err != ESP_OK) {
        logger.printfln("Fuses commit failed: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
        return false;
    }

    return true;
}

bool ESP32CommonSecureBoot::lockdown()
{
    if (!batch_write_fuses(efuse_lockdown_data, std::size(efuse_lockdown_data))) {
        logger.printfln("Setting lockdown fuses failed");
        return false;
    }

    return true;
}

bool ESP32CommonSecureBoot::is_secure_boot_enabled()
{
    return esp_secure_boot_enabled();
}

bool ESP32CommonSecureBoot::is_locked_down()
{
    // If uart_download_dis is set, the device was locked down.
    return EFUSE.blk0_rdata0.rd_uart_download_dis;
}

#endif
