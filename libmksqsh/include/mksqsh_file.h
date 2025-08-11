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
 * @file         mksqsh_file.h
 */

#ifndef MKSQSH_FILE_H
#define MKSQSH_FILE_H

#include "mksqsh_archive.h"
#include <cextras/memory.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The type of a file entry in the archive builder.
 */
enum MksqshFileType {
	/** Directory file type */
	MKSQSH_FILE_TYPE_DIR,
	/** Regular file type */
	MKSQSH_FILE_TYPE_REG,
};

struct MksqshFile *mksqsh_file_new(
		struct MksqshArchive *archive, enum MksqshFileType type, int *err);

int mksqsh_file_add(
		struct MksqshFile *parent, const char *file_name,
		struct MksqshFile *child);

void
mksqsh_file_content_from_fd(struct MksqshFile *file, FILE *source, int *err);

void mksqsh_file_content_from_path(
		struct MksqshFile *file, const char *path, int *err);

void
mksqsh_file_content(struct MksqshFile *file, const char *content, size_t size);

struct MksqshFile *mksqsh_file_retain(struct MksqshFile *file);

void mksqsh_file_release(struct MksqshFile *file);

#ifdef __cplusplus
}
#endif

#endif /* MKSQSH_FILE_H */
