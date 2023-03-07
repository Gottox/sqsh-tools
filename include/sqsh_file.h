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

#ifdef __cplusplus
}
#endif
#endif // SQSH_FILE_H
