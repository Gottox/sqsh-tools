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
 * @file         sqsh_file.h
 */

#ifndef SQSH_FILE_H
#define SQSH_FILE_H

#include "sqsh_table_private.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SqshInodeContext;

////////////////////////////////////////
// file/fragment_table.c

struct SqshFragmentTable;

/**
 * @memberof SqshFragmentTable
 * @brief Writes the fragments of an inode to a buffer.
 *
 * @param[in]  context The fragment table to use.
 * @param[in]  inode The inode to retrieve the fragments from.
 * @param[out] buffer The buffer to write the fragments to.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int sqsh_fragment_table_to_buffer(
		struct SqshFragmentTable *context, const struct SqshInodeContext *inode,
		struct SqshBuffer *buffer);

////////////////////////////////////////
// context/file_reader.c

struct SqshFileReader;

struct SqshFileReader *
sqsh_file_reader_new(const struct SqshInodeContext *inode, int *err);

int sqsh_file_reader_advance(
		struct SqshFileReader *reader, sqsh_index_t offset, size_t size);

const uint8_t *sqsh_file_reader_data(struct SqshFileReader *reader);

size_t sqsh_file_reader_size(struct SqshFileReader *reader);

int sqsh_file_reader_free(struct SqshFileReader *context);

////////////////////////////////////////
// context/file_context.c

struct SqshFileContext;

/**
 * @memberof SqshFileContext
 * @brief Initializes a SqshFileContext struct.
 *
 * @param[in] inode The inode context to retrieve the file contents from.
 * @param[out] err Pointer to an int where the error code will be stored.
 *
 * @return The Initialized file context
 */
SQSH_NO_UNUSED
struct SqshFileContext *
sqsh_file_new(const struct SqshInodeContext *inode, int *err);

/**
 * @memberof SqshFileContext
 * @brief Seek to a position in the file content.
 *
 * @param[in] context The file context to seek in. If the context buffer
 * already contains data, it will be cleared.
 * @param seek_pos The offset to seek to.
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int
sqsh_file_seek(struct SqshFileContext *context, uint64_t seek_pos);

/**
 * @memberof SqshFileContext
 * @brief Reads data from the current seek position
 * and writes it to the content buffer.
 *
 * @param[in] context The file context to read from.
 * @param size The size of the buffer.
 *
 * @return The number of bytes read on success, less than 0 on error.
 */
SQSH_NO_UNUSED int
sqsh_file_read(struct SqshFileContext *context, uint64_t size);

/**
 * @memberof SqshFileContext
 * @brief Gets a pointer to read file content.
 *
 * @param[in] context The file context to get the data from.
 *
 * @return A pointer to the data in the file content buffer.
 */
const uint8_t *sqsh_file_data(struct SqshFileContext *context);

/**
 * @memberof SqshFileContext
 * @brief Gets the size of the file content buffer.
 *
 * @param[in] context The file context to get the size from.
 *
 * @return The size of the file content buffer.
 */
uint64_t sqsh_file_size(const struct SqshFileContext *context);

/**
 * @memberof SqshFileContext
 * @brief Frees the resources used by the file context.
 *
 * @param[in] context The file context to free.
 */
int sqsh_file_free(struct SqshFileContext *context);

////////////////////////////////////////
// file/file_iterator.c

struct SqshFileIterator;

/**
 * @brief Initializes a SqshFileIterator struct.
 *
 * @param[in,out] iterator The file iterator struct to initialize.
 * @param[in] inode The inode context to retrieve the file contents from.
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int sqsh__file_iterator_init(
		struct SqshFileIterator *iterator,
		const struct SqshInodeContext *inode);

/**
 * @brief Cleans up resources used by a SqshFileIterator struct.
 *
 * @param[in,out] iterator The file iterator struct to clean up.
 *
 * @return 0 on success, less than 0 on error.
 */
int sqsh__file_iterator_cleanup(struct SqshFileIterator *iterator);

/**
 * @brief Creates a new SqshFileIterator struct and initializes it.
 *
 * @memberof SqshFileIterator
 *
 * @param[in] inode The inode context to retrieve the file contents from.
 * @param[out] err Pointer to an int where the error code will be stored.
 *
 * @return A pointer to the newly created and initialized SqshFileIterator
 * struct.
 */
SQSH_NO_UNUSED struct SqshFileIterator *
sqsh_file_iterator_new(const struct SqshInodeContext *inode, int *err);

/**
 * @brief Skips a certain amount of data in the file iterator.
 *
 * @memberof SqshFileIterator
 *
 * @param[in,out] iterator The file iterator to skip data in.
 * @param[in] amount The amount of data to skip.
 *
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int
sqsh_file_iterator_skip(struct SqshFileIterator *iterator, sqsh_index_t amount);

/**
 * @brief Reads a certain amount of data from the file iterator.
 *
 * @memberof SqshFileIterator
 *
 * @param[in,out] iterator The file iterator to read data from.
 * @param[in] desired_size The amount of data to read.
 *
 * @return The number of bytes read on success, less than 0 on error.
 */
SQSH_NO_UNUSED int
sqsh_file_iterator_next(struct SqshFileIterator *iterator, size_t desired_size);

/**
 * @brief Gets a pointer to the current data in the file iterator.
 *
 * @memberof SqshFileIterator
 *
 * @param[in] iterator The file iterator to get data from.
 *
 * @return A pointer to the current data in the file iterator.
 */
SQSH_NO_UNUSED const uint8_t *
sqsh_file_iterator_data(struct SqshFileIterator *iterator);

/**
 * @brief Gets the size of the data currently in the file iterator.
 *
 * @memberof SqshFileIterator
 *
 * @param[in] iterator The file iterator to get the size from.
 *
 * @return The size of the data currently in the file iterator.
 */
SQSH_NO_UNUSED size_t
sqsh_file_iterator_size(struct SqshFileIterator *iterator);

/**
 * @brief Frees the resources used by a SqshFileIterator struct.
 *
 * @memberof SqshFileIterator
 *
 * @param[in,out] iterator The file iterator to free.
 *
 * @return 0 on success, less than 0 on error.
 */
int sqsh_file_iterator_free(struct SqshFileIterator *iterator);

#ifdef __cplusplus
}
#endif
#endif // SQSH_FILE_H
