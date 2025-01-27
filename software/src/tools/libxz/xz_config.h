/* SPDX-License-Identifier: 0BSD */

/*
 * Private includes and definitions for userspace use of XZ Embedded
 *
 * Author: Lasse Collin <lasse.collin@tukaani.org>
 */

#ifndef XZ_CONFIG_H
#define XZ_CONFIG_H

/* Uncomment to enable building of xz_dec_catrun(). */
/* #define XZ_DEC_CONCATENATED */

/* Uncomment to enable CRC64 support. */
/* #define XZ_USE_CRC64 */

/* Uncomment as needed to enable BCJ filter decoders. */
/* #define XZ_DEC_X86 */
/* #define XZ_DEC_ARM */
/* #define XZ_DEC_ARMTHUMB */
/* #define XZ_DEC_ARM64 */
/* #define XZ_DEC_RISCV */
/* #define XZ_DEC_POWERPC */
/* #define XZ_DEC_IA64 */
/* #define XZ_DEC_SPARC */

/*
 * Visual Studio 2013 update 2 supports only __inline, not inline.
 * MSVC v19.0 / VS 2015 and newer support both.
 */
#if defined(_MSC_VER) && _MSC_VER < 1900 && !defined(inline)
#	define inline __inline
#endif

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "xz.h"

#define kmalloc(size, flags) malloc(size)
#define kfree(ptr) free(ptr)
#define vmalloc(size) malloc(size)
#define vfree(ptr) free(ptr)

#define memeq(a, b, size) (memcmp(a, b, size) == 0)
#define memzero(buf, size) memset(buf, 0, size)

#ifndef min
#	define min(x, y) ((x) < (y) ? (x) : (y))
#endif
#define min_t(type, x, y) min(x, y)

#ifndef fallthrough
#	if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202000
#		define fallthrough [[fallthrough]]
#	elif defined(__GNUC__) && __GNUC__ >= 7
#		define fallthrough __attribute__((__fallthrough__))
#	else
#		define fallthrough do {} while (0)
#	endif
#endif

/*
 * Some functions have been marked with __always_inline to keep the
 * performance reasonable even when the compiler is optimizing for
 * small code size. You may be able to save a few bytes by #defining
 * __always_inline to plain inline, but don't complain if the code
 * becomes slow.
 *
 * NOTE: System headers on GNU/Linux may #define this macro already,
 * so if you want to change it, you need to #undef it first.
 */
#ifdef __always_inline
#undef __always_inline
#endif
#define __always_inline inline

#ifndef get_le32
#define get_le32(ptr) (*(const uint32_t *)(ptr))
#endif

#endif
