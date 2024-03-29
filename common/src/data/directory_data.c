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
 * @file         directory_data.c
 */

#include <cextras/endian.h>
#include <sqsh_data_private.h>

uint16_t
sqsh__data_directory_entry_offset(const struct SqshDataDirectoryEntry *entry) {
	return CX_LE_2_CPU16(entry->offset);
}

int16_t
sqsh__data_directory_entry_inode_offset(
		const struct SqshDataDirectoryEntry *entry) {
	return (int16_t)CX_LE_2_CPU16((uint16_t)entry->inode_offset);
}

uint16_t
sqsh__data_directory_entry_type(const struct SqshDataDirectoryEntry *entry) {
	return CX_LE_2_CPU16(entry->type);
}

uint16_t
sqsh__data_directory_entry_name_size(
		const struct SqshDataDirectoryEntry *entry) {
	return CX_LE_2_CPU16(entry->name_size);
}

const uint8_t *
sqsh__data_directory_entry_name(const struct SqshDataDirectoryEntry *entry) {
	return (const uint8_t *)&entry[1];
}

uint32_t
sqsh__data_directory_fragment_count(
		const struct SqshDataDirectoryFragment *fragment) {
	return CX_LE_2_CPU32(fragment->count);
}
uint32_t
sqsh__data_directory_fragment_start(
		const struct SqshDataDirectoryFragment *fragment) {
	return CX_LE_2_CPU32(fragment->start);
}
uint32_t
sqsh__data_directory_fragment_inode_number(
		const struct SqshDataDirectoryFragment *fragment) {
	return CX_LE_2_CPU32(fragment->inode_number);
}
