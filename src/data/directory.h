/******************************************************************************
 *                                                                            *
 * Copyright (c) 2021, Enno Boland <g@s01.de>                                 *
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
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : directory
 * @created     : Monday Sep 06, 2021 14:46:06 CEST
 */

#include "../utils.h"
#include <stdint.h>
#include <stdlib.h>

#ifndef SQUASH__DIRECTORY_H

#define SQUASH__DIRECTORY_H

#define SQUASH_SIZEOF_DIRECTORY_FRAGMENT 12
#define SQUASH_SIZEOF_DIRECTORY_ENTRY 8

struct SquashDirectoryEntry;

struct SquashDirectoryFragment;

uint16_t squash_data_directory_entry_offset(
		const struct SquashDirectoryEntry *entry);
int16_t squash_data_directory_entry_inode_offset(
		const struct SquashDirectoryEntry *entry);
uint16_t squash_data_directory_entry_type(
		const struct SquashDirectoryEntry *entry);
uint16_t squash_data_directory_entry_name_size(
		const struct SquashDirectoryEntry *entry);
const uint8_t *squash_data_directory_entry_name(
		const struct SquashDirectoryEntry *entry);

uint32_t squash_data_directory_fragment_count(
		const struct SquashDirectoryFragment *fragment);
uint32_t squash_data_directory_fragment_start(
		const struct SquashDirectoryFragment *fragment);
uint32_t squash_data_directory_fragment_inode_number(
		const struct SquashDirectoryFragment *fragment);
const struct SquashDirectoryEntry *squash_data_directory_fragment_entries(
		const struct SquashDirectoryFragment *fragment);

#endif /* end of include guard SQUASH__DIRECTORY_H */
