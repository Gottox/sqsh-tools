/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023-2024, Enno Boland <g@s01.de>                            *
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
 * @file         sqsh_common.h
 */

#ifndef SQSH_COMMON_H
#define SQSH_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
#	define SQSH_STATIC_ASSERT(cond)
#else
/**
 * @brief Static assert
 *
 * @param cond Condition to assert
 */
#	define SQSH_STATIC_ASSERT(cond) _Static_assert(cond, #cond)
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

/**
 * @brief Warn if return value is unused
 */
#define SQSH_NO_UNUSED __attribute__((warn_unused_result))

/**
 * @brief Do not export symbol
 */
#define SQSH_NO_EXPORT __attribute__((visibility("hidden")))

/**
 * @deprecated Since 1.6.0. Use size_t instead.
 * @brief typedef used for indexing
 */
__attribute__((deprecated(
		"Since 1.6.0. Use size_t instead."))) typedef size_t sqsh_index_t;

#ifdef __cplusplus
}
#endif
#endif /* SQSH_COMMON_H */
