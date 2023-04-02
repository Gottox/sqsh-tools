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
 * @file         sqsh_metablock_private.h
 */

#ifndef SQSH_METABLOCK_PRIVATE_H
#define SQSH_METABLOCK_PRIVATE_H

#include "sqsh_compression_private.h"
#include "sqsh_mapper_private.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SqshArchive;

////////////////////////////////////////
// metablock/metablock_iterator.c

struct SqshMetablockIterator {
	/**
	 * @privatesection
	 */
	struct SqshMapReader reader;
	struct SqshCompressionManager *compression_manager;
	const struct SqshCompression *old_compression;
	struct SqshBuffer old_buffer;
	struct SqshExtractView extract_view;
	bool is_compressed;
	uint16_t outer_size;
	uint16_t inner_size;
	const uint8_t *data;
};

/**
 * @internal
 * @memberof SqshMetablockIterator
 * @brief Initializes an iterator for reading metablocks.
 *
 * @param[out] iterator The iterator to initialize.
 * @param[in] sqsh The Squash instance the metablocks belong to.
 * @param[in] compression_manager The compression manager to use.
 * @param[in] start_address The address of the first metablock to read.
 * @param[in] upper_limit The maximum address the iterator is allowed to read.
 * @return 0 on success, or a negative value on error.
 */
SQSH_NO_UNUSED int sqsh__metablock_iterator_init(
		struct SqshMetablockIterator *iterator, struct SqshArchive *sqsh,
		struct SqshCompressionManager *compression_manager,
		uint64_t start_address, uint64_t upper_limit);

/**
 * @internal
 * @memberof SqshMetablockIterator
 * @brief skip several metablocks. sqsh__metablock_iterator_skip(i, 1) is
 * equivalent to sqsh__metablock_iterator_next(i).
 *
 * @param[in,out] iterator The iterator to advance.
 * @param[in] amount The number of metablocks to skip.
 * @return 0 on success, or a negative value on error.
 */
SQSH_NO_UNUSED int sqsh__metablock_iterator_skip(
		struct SqshMetablockIterator *iterator, size_t amount);

/**
 * @internal
 * @memberof SqshMetablockIterator
 * @brief Moves the iterator to the next metablock.
 *
 * @param[in,out] iterator The iterator to advance.
 * @return 0 on success, or a negative value on error.
 */
SQSH_NO_UNUSED int
sqsh__metablock_iterator_next(struct SqshMetablockIterator *iterator);

/**
 * @internal
 * @memberof SqshMetablockIterator
 * @brief Returns a pointer to the data of the current metablock.
 *
 * @param[in] iterator The iterator.
 * @return A pointer to the data of the current metablock.
 */
const uint8_t *
sqsh__metablock_iterator_data(const struct SqshMetablockIterator *iterator);

/**
 * @internal
 * @memberof SqshMetablockIterator
 * @brief Returns a pointer to the data of the current metablock.
 *
 * @param[in] iterator The iterator.
 * @return Returns the address of the data of the current metablock.
 */
uint64_t sqsh__metablock_iterator_data_address(
		const struct SqshMetablockIterator *iterator);

/**
 * @internal
 * @memberof SqshMetablockIterator
 * @brief Returns whether the current metablock is compressed.
 *
 * @param[in] iterator The iterator.
 * @return Whether the current metablock is compressed.
 */
bool sqsh__metablock_iterator_is_compressed(
		const struct SqshMetablockIterator *iterator);

/**
 * @internal
 * @memberof SqshMetablockIterator
 * @brief Returns the size of the current metablock.
 *
 * @param[in] iterator The iterator.
 * @return The size of the current metablock.
 */
size_t
sqsh__metablock_iterator_size(const struct SqshMetablockIterator *iterator);

/**
 * @internal
 * @memberof SqshMetablockIterator
 * @brief Cleans up an iterator for reading metablocks.
 *
 * @param[in,out] iterator The iterator to clean up.
 * @return 0 on success, or a negative value on error.
 */
int sqsh__metablock_iterator_cleanup(struct SqshMetablockIterator *iterator);

////////////////////////////////////////
// metablock/metablock_reader.c

struct SqshMetablockReader {
	/**
	 * @privatesection
	 */
	struct SqshMetablockIterator iterator;
	struct SqshBuffer buffer;
	// TODO: remove the compression field and use the compression_manager
	// instead
	const struct SqshCompression *compression;
	struct SqshCompressionManager *compression_manager;
	sqsh_index_t offset;
	size_t size;
};

/**
 * @internal
 * @memberof SqshMetablockReader
 * @brief Initializes a metablock cursor.
 *
 * @param[out] cursor Pointer to the metablock cursor to be initialized.
 * @param[in] sqsh Pointer to the Sqsh struct.
 * @param[in] compression_manager compression manager to use.
 * @param[in] start_address Start address of the metablock.
 * @param[in] upper_limit Upper limit of the metablock.
 *
 * @return 0 on success, less than zero on error.
 */
SQSH_NO_UNUSED int sqsh__metablock_reader_init(
		struct SqshMetablockReader *cursor, struct SqshArchive *sqsh,
		struct SqshCompressionManager *compression_manager,
		const uint64_t start_address, const uint64_t upper_limit);

/**
 * @internal
 * @memberof SqshMetablockReader
 * @brief Advances the metablock cursor by the given offset and size.
 *
 * @param[in,out] cursor Pointer to the metablock cursor to be advanced.
 * @param[in] offset Offset to advance the cursor by.
 * @param[in] size Size of the block to advance the cursor by.
 *
 * @return 0 on success, less than zero on error.
 */
SQSH_NO_UNUSED int sqsh__metablock_reader_advance(
		struct SqshMetablockReader *cursor, sqsh_index_t offset, size_t size);

/**
 * @internal
 * @memberof SqshMetablockReader
 * @brief Returns a pointer to the data at the current position of the metablock
 * cursor.
 *
 * @param[in] cursor Pointer to the metablock cursor.
 *
 * @return Pointer to the data at the current position of the metablock cursor.
 */
const uint8_t *
sqsh__metablock_reader_data(const struct SqshMetablockReader *cursor);

/**
 * @internal
 * @memberof SqshMetablockReader
 * @brief Returns the size of the data at the current position of the metablock
 * cursor.
 *
 * @param[in] cursor Pointer to the metablock cursor.
 *
 * @return Size of the data at the current position of the metablock cursor.
 */
size_t sqsh__metablock_reader_size(const struct SqshMetablockReader *cursor);

/**
 * @internal
 * @memberof SqshMetablockReader
 * @brief Cleans up and frees the resources used by the metablock cursor.
 *
 * @param[in,out] cursor Pointer to the metablock cursor to be cleaned up.
 *
 * @return 0 on success, less than zero on error.
 */
int sqsh__metablock_reader_cleanup(struct SqshMetablockReader *cursor);

#ifdef __cplusplus
}
#endif
#endif // SQSH_METABLOCK_PRIVATE_H
