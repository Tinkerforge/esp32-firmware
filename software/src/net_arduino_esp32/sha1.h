/*
 * Based on the SHA-1 C implementation by Steve Reid <steve@edmweb.com>
 * 100% Public Domain
 */

#ifndef TF_SHA1_H
#define TF_SHA1_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TF_SHA1_BLOCK_LENGTH 64
#define TF_SHA1_DIGEST_LENGTH 20

typedef struct TF_SHA1 {
	uint32_t state[5];
	uint64_t count;
	uint8_t buffer[TF_SHA1_BLOCK_LENGTH];
} TF_SHA1;

void tf_sha1_init(TF_SHA1 *sha1);
void tf_sha1_update(TF_SHA1 *sha1, const uint8_t *data, size_t length);
void tf_sha1_final(TF_SHA1 *sha1, uint8_t digest[TF_SHA1_DIGEST_LENGTH]);

#ifdef __cplusplus
}
#endif

#endif
