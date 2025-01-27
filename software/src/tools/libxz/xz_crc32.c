// SPDX-License-Identifier: 0BSD

#include "esp_rom_crc.h"

#include "xz_private.h"

uint32_t xz_crc32(const uint8_t *buf, size_t size, uint32_t crc)
{
	return esp_rom_crc32_le(crc, buf, size);
}
