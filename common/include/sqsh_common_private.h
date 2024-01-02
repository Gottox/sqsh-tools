/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023, Enno Boland <g@s01.de>                                 *
 *                                                                            *
 * Redistribution and use in source and binary forms, with or without         *
 * modification, are permitted provided that the following conditions are     *
 * met:                                                                       *
 *                                                                            *
 * * Redistributions of source code must retain the above copyright notice,   *
 *   this list of conditions and the following disclaimer.                    *
 * * Redistributions in binary form must reproduce the above copyright        *
 *   notice, this list of conditions and the following disclaimer in the      *
 *   documentation and/or other materials provided with the distribution.     *
 *                                                                            *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS    *
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,  *
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR     *
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR          *
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,      *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,        *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR         *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF     *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING       *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               *
 *                                                                            *
 ******************************************************************************/

/**
 * @author       Enno Boland (mail@eboland.de)
 * @file         sqsh_common_private.h
 */

#ifndef SQSH_UTILS_H
#define SQSH_UTILS_H

#include <sqsh_common.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SQSH_MIN(a, b) ((a) < (b) ? (a) : (b))
#define SQSH_MAX(a, b) ((a) > (b) ? (a) : (b))

#define SQSH_UNLIKELY(x) __builtin_expect(!!(x), 0)

#define SQSH__ADD_OVERFLOW(a, b, res) __builtin_add_overflow(a, b, res)
#define SQSH__SUB_OVERFLOW(a, b, res) __builtin_sub_overflow(a, b, res)
#define SQSH__MULT_OVERFLOW(a, b, res) __builtin_mul_overflow(a, b, res)

#define SQSH_ADD_OVERFLOW(a, b, res) \
	SQSH_UNLIKELY(SQSH__ADD_OVERFLOW(a, b, res))
#define SQSH_SUB_OVERFLOW(a, b, res) \
	SQSH_UNLIKELY(SQSH__SUB_OVERFLOW(a, b, res))
#define SQSH_MULT_OVERFLOW(a, b, res) \
	SQSH_UNLIKELY(SQSH__MULT_OVERFLOW(a, b, res))

#define SQSH_DIVIDE_CEIL(x, y) ((x) / (y) + !!((x) % (y)))
#define SQSH_PADDING(x, p) (SQSH_DIVIDE_CEIL(x, p) * p)

#define SQSH_CONFIG_DEFAULT(x, d) (size_t)(x == 0 ? (d) : SQSH_MAX(x, 0))

#define SQSH_NEW_IMPL(init, type, ...) \
	int rv = 0; \
	type *obj = calloc(1, sizeof(type)); \
	if (obj == NULL) { \
		rv = -SQSH_ERROR_MALLOC_FAILED; \
		goto out; \
	} \
	rv = init(obj, __VA_ARGS__); \
	if (rv < 0) { \
		free(obj); \
		obj = NULL; \
	} \
out: \
	if (err != NULL) { \
		*err = rv; \
	} \
	return obj

#define SQSH_FREE_IMPL(cleanup, obj) \
	if (obj == NULL) { \
		return 0; \
	} \
	int rv = cleanup(obj); \
	free(obj); \
	return rv

SQSH_NO_UNUSED static inline uint64_t
sqsh_address_ref_outer_offset(uint64_t ref) {
	return ref >> 16;
}

SQSH_NO_UNUSED static inline uint16_t
sqsh_address_ref_inner_offset(uint64_t ref) {
	return ref & 0xFFFF;
}

SQSH_NO_UNUSED static inline uint64_t
sqsh_address_ref_create(uint32_t outer_offset, uint16_t inner_offset) {
	return ((uint64_t)outer_offset << 16) | inner_offset;
}

SQSH_NO_UNUSED static inline uint32_t
sqsh_datablock_size(uint32_t size_info) {
	return size_info & (uint32_t) ~(1 << 24);
}

SQSH_NO_UNUSED static inline bool
sqsh_datablock_is_compressed(uint32_t size_info) {
	return !(size_info & (1 << 24));
}

#ifdef __cplusplus
}
#endif
#endif /* end of include guard SQSH_UTILS_H */
