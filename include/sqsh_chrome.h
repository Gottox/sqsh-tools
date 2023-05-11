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
 * @file         sqsh_chrome.h
 */

#ifndef SQSH_CHROME_H
#define SQSH_CHROME_H

#include "sqsh_common.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SqshArchive;

////////////////////////////////////////
// chrome/inode.c

/**
 * @brief Initialize the inode context from a path.
 *
 * @param[in] archive The sqsh archive context.
 * @param[in] path The path the file or directory.
 * @param[out] err Pointer to an int where the error code will be stored.
 *
 * @return 0 on success, less than 0 on error.
 */
struct SqshInode *
sqsh_open(struct SqshArchive *archive, const char *path, int *err);

/**
 * @brief cleans up a the inode context.
 *
 * @param[in] inode The inode context.
 *
 * @return 0 on success, less than 0 on error.
 */
int sqsh_close(struct SqshInode *inode);

////////////////////////////////////////
// chrome/file.c

/**
 * @brief writes data to a file descriptor.
 *
 * @param[in] inode The inode context.
 * @param[in] file The file descriptor.
 *
 * @return The number of bytes read on success, less than 0 on error.
 */
int sqsh_file_to_stream(const struct SqshInode *inode, FILE *file);

/**
 * @brief retrieves the content of a file.
 *
 * @param[in] archive  The sqsh archive context.
 * @param[in] path     The path the file or directory.
 *
 * @return The content of the file on success, NULL on error.
 */
char *sqsh_file_content(struct SqshArchive *archive, const char *path);

#ifdef __cplusplus
}
#endif
#endif // SQSH_CHROME_H
