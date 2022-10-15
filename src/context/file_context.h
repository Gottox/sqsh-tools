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
 * @file         file_context.h
 */

#include "../primitive/buffer.h"
#include <stdint.h>

#ifndef FILE_CONTEXT_H

#define FILE_CONTEXT_H

struct SqshInodeContext;
struct Sqsh;

/**
 * @brief The SqshFileContext struct
 *
 * This struct is used to assemble file contents.
 */
struct SqshFileContext {
	struct SqshMapper *mapper;
	struct SqshFragmentTable *fragment_table;
	struct SqshInodeContext *inode;
	struct SqshBuffer buffer;
	struct SqshCompression *compression;
	uint64_t seek_pos;
	uint32_t block_size;
};

/**
 * @brief Initializes a SqshContentContext struct.
 * @memberof SqshContentContext
 * @param context The file context to initialize.
 * @param inode The inode context to retrieve the file contents from.
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int
sqsh_file_init(struct SqshFileContext *context, struct SqshInodeContext *inode);

/**
 * @brief Seek to a position in the file content.
 * @memberof SqshContentContext
 * @param context The file context to seek in. If the context buffer
 * already contains data, it will be cleared.
 * @param offset The offset to seek to.
 * @return 0 on success, less than 0 on error.
 */
SQSH_NO_UNUSED int
sqsh_file_seek(struct SqshFileContext *context, uint64_t seek_pos);

/**
 * @brief Reads data from the current seek position
 * and writes it to the content buffer.
 * @memberof SqshContentContext
 * @param context The file context to read from.
 * @param size The size of the buffer.
 * @return The number of bytes read on success, less than 0 on error.
 */
int sqsh_file_read(struct SqshFileContext *context, uint64_t size);

/**
 * @brief Gets a pointer to read file content.
 * @memberof SqshContentContext
 * @param context The file context to get the data from.
 * @return A pointer to the data in the file content buffer.
 */
const uint8_t *sqsh_file_data(struct SqshFileContext *context);

/**
 * @brief Gets the size of the file content buffer.
 * @memberof SqshContentContext
 * @param context The file context to get the size from.
 * @return The size of the file content buffer.
 */
uint64_t sqsh_file_size(struct SqshFileContext *context);

/**
 * @brief Frees the resources used by the file context.
 * @memberof SqshContentContext
 * @param context The file context to clean up.
 */
int sqsh_file_cleanup(struct SqshFileContext *context);

#endif /* end of include guard FILE_CONTEXT_H */
