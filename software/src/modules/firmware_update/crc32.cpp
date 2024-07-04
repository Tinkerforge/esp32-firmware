/* bricklib2
 * Copyright (C) 2019 Olaf Lüke <olaf@tinkerforge.com>
 *
 * crc32.c: Implementation of CRC32 checksum calculation
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

#include "crc32.h"

typedef unsigned int word_t;

// Polynomial for 32-bit CRC in IEEE 802.3.
#define CRC32_IEEE_802_3_POLYNOMIAL     0xEDB88320UL

// Convenience macro for inverting the CRC.
#define COMPLEMENT_CRC(c)    ((c) ^ 0xffffffffUL)

// Convenience macro for size of a word.
#define WORD_SIZE            (sizeof(word_t))

// Bitmask for word-aligning an address.
#define WORD_ALIGNMENT_MASK  ~((uintptr_t)WORD_SIZE - 1)


// Recalculate 32-bit CRC for bytes within a word
static inline uint32_t _crc32_ieee_802_3_recalculate_bytes_helper(word_t data, uint32_t crc, uint_fast8_t bytes)
{
	uint_fast8_t bit;

	crc ^= data;

	for (bit = 8 * bytes; bit > 0; bit--) {
		if (crc & 1) {
			crc = (crc >> 1) ^ CRC32_IEEE_802_3_POLYNOMIAL;
		} else {
			crc >>= 1;
		}
	}

	return crc;
}

//  Recalculate 32-bit CRC for another block
//
// This function recalculates the CRC according to the polynomial
// CRC32_POLYNOMIAL for the specified data block and initial CRC value.
//
// To reduce the number of databus accesses and thus speed up the calculation,
// the algorithm is tuned to work with words as much as possible.
void crc32_ieee_802_3_recalculate(const void *data, size_t length, uint32_t *crc)
{
    const word_t *word_ptr = (word_t *)((uintptr_t)data & WORD_ALIGNMENT_MASK);
	size_t temp_length;
	uint32_t temp_crc = COMPLEMENT_CRC(*crc);
	word_t word;

	// Calculate for initial bytes to get word-aligned
	if(length < WORD_SIZE) {
		temp_length = length;
	} else {
		temp_length = ~WORD_ALIGNMENT_MASK & (WORD_SIZE - (uintptr_t)data);
	}

	if(temp_length) {
		length -= temp_length;

		word = *(word_ptr++);
		word >>= 8 * (WORD_SIZE - temp_length);
		temp_crc = _crc32_ieee_802_3_recalculate_bytes_helper(word, temp_crc, temp_length);
	}

	// Calculate for whole words, if any
	temp_length = length & WORD_ALIGNMENT_MASK;

	if(temp_length) {
		length -= temp_length;
		temp_length /= WORD_SIZE;

		while(temp_length--) {
			word = *(word_ptr++);
			temp_crc = _crc32_ieee_802_3_recalculate_bytes_helper(word, temp_crc, WORD_SIZE);
		}
	}

	// Calculate for tailing bytes
	if(length) {
		word = *word_ptr;
		word &= 0xffffffffUL >> (8 * (WORD_SIZE - length));
		temp_crc = _crc32_ieee_802_3_recalculate_bytes_helper(word, temp_crc, length);
	}

	*crc = COMPLEMENT_CRC(temp_crc);
}

uint32_t crc32_ieee_802_3(const void *data, size_t length)
{
	uint32_t crc = 0;
	crc32_ieee_802_3_recalculate(data, length, &crc);
	return crc;
}
