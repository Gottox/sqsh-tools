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
 * @file         directory_internal.h
 */

#include "../utils.h"
#include "directory.h"

#ifndef DIRECTORY_INTERNAL_H

#define DIRECTORY_INTERNAL_H

struct SQSH_UNALIGNED SqshDirectoryEntry {
	uint16_t offset;
	int16_t inode_offset;
	uint16_t type;
	uint16_t name_size;
	// uint8_t name[0]; // [name_size + 1]
};

STATIC_ASSERT(sizeof(struct SqshDirectoryEntry) == SQSH_SIZEOF_DIRECTORY_ENTRY);

struct SQSH_UNALIGNED SqshDirectoryFragment {
	uint32_t count;
	uint32_t start;
	uint32_t inode_number;
	// struct SqshDirectoryEntry entries[0]; // [count + 1]
};

STATIC_ASSERT(
		sizeof(struct SqshDirectoryFragment) == SQSH_SIZEOF_DIRECTORY_FRAGMENT);

#endif /* end of include guard DIRECTORY_INTERNAL_H */
