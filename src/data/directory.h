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
 * @file         directory.h
 */

#include "../utils.h"
#include <stdint.h>
#include <stdlib.h>

#ifndef SQSH__DIRECTORY_H

#define SQSH__DIRECTORY_H

#define SQSH_SIZEOF_DIRECTORY_FRAGMENT 12
#define SQSH_SIZEOF_DIRECTORY_ENTRY 8

struct SQSH_UNALIGNED SqshDirectoryEntry;

struct SQSH_UNALIGNED SqshDirectoryFragment;

uint16_t
sqsh_data_directory_entry_offset(const struct SqshDirectoryEntry *entry);
int16_t
sqsh_data_directory_entry_inode_offset(const struct SqshDirectoryEntry *entry);
uint16_t sqsh_data_directory_entry_type(const struct SqshDirectoryEntry *entry);
uint16_t
sqsh_data_directory_entry_name_size(const struct SqshDirectoryEntry *entry);
const uint8_t *
sqsh_data_directory_entry_name(const struct SqshDirectoryEntry *entry);

uint32_t sqsh_data_directory_fragment_count(
		const struct SqshDirectoryFragment *fragment);
uint32_t sqsh_data_directory_fragment_start(
		const struct SqshDirectoryFragment *fragment);
uint32_t sqsh_data_directory_fragment_inode_number(
		const struct SqshDirectoryFragment *fragment);
const struct SqshDirectoryEntry *sqsh_data_directory_fragment_entries(
		const struct SqshDirectoryFragment *fragment);

#endif /* end of include guard SQSH__DIRECTORY_H */
