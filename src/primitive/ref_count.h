/******************************************************************************
 *                                                                            *
 * Copyright (c) 2022, Enno Boland <g@s01.de>                                 *
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
 * @file         ref_count.h
 */

#include "../utils.h"

#ifndef REFCOUNT_H

#define REFCOUNT_H

typedef int (*sqshRefCountDtor)(void *);

/**
 * @brief A container struct for reference counting objects.
 */
struct SqshRefCount {
	size_t references;
	sqshRefCountDtor dtor;
};

/**
 * @brief Initialize a SqshRefCount struct.
 * @memberof SqshRefCount
 * @param ref_count A pointer to the SqshRefCount struct.
 * @param object_size The size of the object to be reference counted.
 * @param dtor A pointer to a function that is called when the reference count
 * reaches 0.
 * @return 0 on success, less than 0 on error.
 */
int sqsh_ref_count_new(
		struct SqshRefCount **ref_count, size_t object_size,
		sqshRefCountDtor dtor);

/**
 * @brief Increment the reference count of a SqshRefCount struct.
 * @memberof SqshRefCount
 * @param ref_count A pointer to the SqshRefCount struct.
 * @return pointer to the reference counted object.
 */
void *sqsh_ref_count_retain(struct SqshRefCount *ref_count);

/**
 * @brief Decrement the reference count of a SqshRefCount struct. If the
 * reference count reaches 0, the destructor function is called.
 * @memberof SqshRefCount
 * @param ref_count A pointer to the SqshRefCount struct.
 * @return amount of references left.
 */
int sqsh_ref_count_release(struct SqshRefCount *ref_count);

#endif /* end of include guard REFCOUNT_H */
