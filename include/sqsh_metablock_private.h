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

#include "sqsh_extract_private.h"
#include "sqsh_mapper_private.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SqshArchive;

/***************************************
 * metablock/metablock_iterator.c
 */

/**
 * @brief Iterator over metablocks.
 */
struct SqshMetablockIterator {
	/**
	 * @privatesection
	 */
	struct SqshMapReader reader;
	struct SqshExtractManager *compression_manager;
	struct SqshExtractView extract_view;
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
 * @param[in] start_address The address of the first metablock to read.
 * @param[in] upper_limit The maximum address the iterator is allowed to read.
 * @return 0 on success, or a negative value on error.
 */
SQSH_NO_EXPORT SQSH_NO_UNUSED int sqsh__metablock_iterator_init(
		struct SqshMetablockIterator *iterator, struct SqshArchive *sqsh,
		uint64_t start_address, uint64_t upper_limit);

/**
 * @internal
 * @memberof SqshMetablockIterator
 * @brief Moves the iterator to the next metablock.
 *
 * @param[in,out] iterator  The iterator to advance.
 * @param[out]    err       Pointer to an int where the error code will be
 * stored.
 *
 * @retval true  When the iterator was moved to the next metablock
 * @retval false When an error occured. See err for details.
 */
SQSH_NO_EXPORT bool
sqsh__metablock_iterator_next(struct SqshMetablockIterator *iterator, int *err);

/**
 * @internal
 * @memberof SqshMetablockIterator
 * @brief Returns a pointer to the data of the current metablock.
 *
 * @param[in] iterator The iterator.
 * @return A pointer to the data of the current metablock.
 */
SQSH_NO_EXPORT const uint8_t *
sqsh__metablock_iterator_data(const struct SqshMetablockIterator *iterator);

/**
 * @internal
 * @memberof SqshMetablockIterator
 * @brief Returns the size of the current metablock.
 *
 * @param[in] iterator The iterator.
 * @return The size of the current metablock.
 */
SQSH_NO_EXPORT size_t
sqsh__metablock_iterator_size(const struct SqshMetablockIterator *iterator);

/**
 * @internal
 * @memberof SqshMetablockIterator
 * @brief Cleans up an iterator for reading metablocks.
 *
 * @param[in,out] iterator The iterator to clean up.
 * @return 0 on success, or a negative value on error.
 */
SQSH_NO_EXPORT int
sqsh__metablock_iterator_cleanup(struct SqshMetablockIterator *iterator);

/***************************************
 * metablock/metablock_reader.c
 */

/**
 * @brief Reader over metablocks.
 */
struct SqshMetablockReader {
	/**
	 * @privatesection
	 */
	struct SqshReader reader;
	struct SqshMetablockIterator iterator;
};

/**
 * @internal
 * @memberof SqshMetablockReader
 * @brief Initializes a metablock reader.
 *
 * @param[out] reader Pointer to the metablock reader to be initialized.
 * @param[in] sqsh Pointer to the Sqsh struct.
 * @param[in] start_address Start address of the metablock.
 * @param[in] upper_limit Upper limit of the metablock.
 *
 * @return 0 on success, less than zero on error.
 */
SQSH_NO_EXPORT SQSH_NO_UNUSED int sqsh__metablock_reader_init(
		struct SqshMetablockReader *reader, struct SqshArchive *sqsh,
		const uint64_t start_address, const uint64_t upper_limit);

/**
 * @internal
 * @memberof SqshMetablockReader
 * @brief Advances the metablock reader by the given offset and size.
 *
 * @param[in,out] reader Pointer to the metablock reader to be advanced.
 * @param[in] offset Offset to advance the reader by.
 * @param[in] size Size of the block to advance the reader by.
 *
 * @return 0 on success, less than zero on error.
 */
SQSH_NO_EXPORT SQSH_NO_UNUSED int sqsh__metablock_reader_advance(
		struct SqshMetablockReader *reader, sqsh_index_t offset, size_t size);

/**
 * @internal
 * @memberof SqshMetablockReader
 * @brief Returns a pointer to the data at the current position of the metablock
 * reader.
 *
 * @param[in] reader Pointer to the metablock reader.
 *
 * @return Pointer to the data at the current position of the metablock reader.
 */
SQSH_NO_EXPORT const uint8_t *
sqsh__metablock_reader_data(const struct SqshMetablockReader *reader);

/**
 * @internal
 * @memberof SqshMetablockReader
 * @brief Returns the size of the data at the current position of the metablock
 * reader.
 *
 * @param[in] reader Pointer to the metablock reader.
 *
 * @return Size of the data at the current position of the metablock reader.
 */
SQSH_NO_EXPORT size_t
sqsh__metablock_reader_size(const struct SqshMetablockReader *reader);

/**
 * @internal
 * @memberof SqshMetablockReader
 * @brief Cleans up and frees the resources used by the metablock reader.
 *
 * @param[in,out] reader Pointer to the metablock reader to be cleaned up.
 *
 * @return 0 on success, less than zero on error.
 */
SQSH_NO_EXPORT int
sqsh__metablock_reader_cleanup(struct SqshMetablockReader *reader);

#ifdef __cplusplus
}
#endif
#endif /* SQSH_METABLOCK_PRIVATE_H */
