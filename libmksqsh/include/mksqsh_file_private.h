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
 * @file         mksqsh_file_private.h
 */

#ifndef MKSQSH_FILE_PRIVATE_H
#define MKSQSH_FILE_PRIVATE_H

#include <cextras/memory.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "mksqsh_file.h"

#ifdef __cplusplus
extern "C" {
#endif

enum MksqshFileContentType {
	MKSQSH_FILE_CONTENT_NONE,
	MKSQSH_FILE_CONTENT_MEMORY,
	MKSQSH_FILE_CONTENT_FD,
	MKSQSH_FILE_CONTENT_PATH,
};

struct MksqshFile {
	struct CxRc rc;
	enum MksqshFileType type;
	struct MksqshArchive *archive;
	char *name;
	enum MksqshFileContentType content_type;
	uint8_t *data;
	size_t size;
	FILE *fd;
	char *path;
	struct MksqshFile **children;
	size_t child_count;
};

#ifdef __cplusplus
}
#endif

#endif /* MKSQSH_FILE_PRIVATE_H */
