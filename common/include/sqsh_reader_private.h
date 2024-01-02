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
 * @file         sqsh_reader_private.h
 */

#ifndef SQSH_READER_PRIVATE_H
#define SQSH_READER_PRIVATE_H

#include "sqsh_common.h"
#include <cextras/collection.h>
#include <sys/wait.h>

#ifdef __cplusplus
extern "C" {
#endif

/***************************************
 * reader/reader.c
 */

struct SqshReaderIteratorImpl {
	bool (*next)(void *iterator, size_t desired_size, int *err);
	int (*skip)(void *iterator, sqsh_index_t *offset, size_t desired_size);
	const uint8_t *(*data)(const void *iterator);
	size_t (*size)(const void *iterator);
};

/**
 * @brief An iterator over extended attributes.
 */
struct SqshReader {
	/**
	 * @privatesection
	 */

	/**
	 * @brief The for this reader.
	 */
	void *iterator;

	/**
	 * @brief interface to the iterator.
	 */
	const struct SqshReaderIteratorImpl *iterator_impl;

	/**
	 * @brief The offset of the iterator.
	 *
	 * A value of "0" indicates, that the reader currently directly maps data
	 * from the iterator. That means, that `data` points into memory managed by
	 * the iterator.
	 *
	 * A non-zero value indicates, that the reader has copied data from the
	 * iterator into the buffer. That means, that `data` points into memory
	 * managed by the buffer. The actual value of iterator_offset is the offset
	 * between the buffer and the iterator.
	 *
	 * example:
	 *
	 * ```
	 *            0123456789
	 * buffer:    ##########
	 * iterator:        ####
	 * ```
	 *
	 * in this case, the iterator_offset is 6.
	 */
	sqsh_index_t iterator_offset;

	/**
	 * @brief The offset of mapped data.
	 *
	 * This value is set to zero if the reader uses buffered data.
	 *
	 * Otherwise is indicates the offset of the data in the iterator.
	 */
	sqsh_index_t offset;

	/**
	 * @brief The buffer to store data in if they cannot be directly mapped.
	 */
	struct CxBuffer buffer;

	/**
	 * @brief The data that is presented to the user.
	 *
	 * This pointer has the offset already applied if in mapped mode.
	 *
	 * Otherwise it points to the beginning of the buffer.
	 */
	const uint8_t *data;
	/**
	 * @brief The size of the data that is presented to the user.
	 */
	size_t size;

	size_t iterator_size;
};

/**
 * @internal
 * @memberof SqshReader
 * @brief Initializes a reader.
 *
 * @param[out] reader         Pointer to the metablock reader to be initialized.
 * @param[in]  iterator_impl  Implementation of the iterator.
 * @param[in]  iterator       Iterator to use for the reader.
 *
 * @return 0 on success, less than zero on error.
 */
SQSH_NO_EXPORT SQSH_NO_UNUSED int sqsh__reader_init(
		struct SqshReader *reader,
		const struct SqshReaderIteratorImpl *iterator_impl, void *iterator);

/**
 * @internal
 * @memberof SqshReader
 * @brief Advances the reader by the given offset and size.
 *
 * @param[in,out] reader  Pointer to the metablock reader to be advanced.
 * @param[in] offset      Offset to advance the reader by.
 * @param[in] size        Size of the block to advance the reader by.
 *
 * @return 0 on success, less than zero on error.
 */
SQSH_NO_EXPORT SQSH_NO_UNUSED int sqsh__reader_advance(
		struct SqshReader *reader, sqsh_index_t offset, size_t size);

/**
 * @internal
 * @memberof SqshReader
 * @brief Returns a pointer to the data at the current position of the metablock
 * reader.
 *
 * @param[in] reader  Pointer to the metablock reader.
 *
 * @return Pointer to the data at the current position of the metablock reader.
 */
SQSH_NO_EXPORT const uint8_t *
sqsh__reader_data(const struct SqshReader *reader);

/**
 * @internal
 * @memberof SqshReader
 * @brief Returns the size of the data at the current position of the metablock
 * reader.
 *
 * @param[in] reader Pointer to the metablock reader.
 *
 * @return Size of the data at the current position of the metablock reader.
 */
SQSH_NO_EXPORT size_t sqsh__reader_size(const struct SqshReader *reader);

/**
 * @internal
 * @memberof SqshReader
 * @brief Cleans up and frees the resources used by the metablock reader.
 *
 * @param[in,out] reader Pointer to the metablock reader to be cleaned up.
 *
 * @return 0 on success, less than zero on error.
 */
SQSH_NO_EXPORT int sqsh__reader_cleanup(struct SqshReader *reader);

#ifdef __cplusplus
}
#endif
#endif /* SQSH_READER_PRIVATE_H */
