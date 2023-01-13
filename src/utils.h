// utils.c
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
 * @file         sqsh_utils.h
 */

#ifndef SQSH_UTILS_H
#define SQSH_UTILS_H

#include <assert.h>
#include <sqsh_common.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SQSH_MIN(a, b) (a < b ? a : b)
#define SQSH_MAX(a, b) (a > b ? a : b)

#define SQSH_ADD_OVERFLOW(a, b, res) __builtin_add_overflow(a, b, res)
#define SQSH_SUB_OVERFLOW(a, b, res) __builtin_sub_overflow(a, b, res)
#define SQSH_MULT_OVERFLOW(a, b, res) __builtin_mul_overflow(a, b, res)

// Does not work for x == 0
#define SQSH_DEVIDE_CEIL(x, y) (((x - 1) / y) + 1)
#define SQSH_PADDING(x, p) SQSH_DEVIDE_CEIL(x, p) * p

SQSH_NO_UNUSED static inline void *
sqsh_memdup(const void *source, size_t size) {
	void *target = calloc(size + 1, sizeof(uint8_t));
	if (target == NULL) {
		return NULL;
	}
	return memcpy(target, source, size);
}

#ifdef __cplusplus
}
#endif
#endif /* end of include guard SQSH_UTILS_H */
