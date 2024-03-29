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
 * @file         superblock_data.c
 */

#include <cextras/endian.h>
#include <sqsh_data_private.h>

uint32_t
sqsh__data_superblock_magic(const struct SqshDataSuperblock *superblock) {
	return CX_LE_2_CPU32(superblock->magic);
}

uint32_t
sqsh__data_superblock_inode_count(const struct SqshDataSuperblock *superblock) {
	return CX_LE_2_CPU32(superblock->inode_count);
}

uint32_t
sqsh__data_superblock_modification_time(
		const struct SqshDataSuperblock *superblock) {
	return CX_LE_2_CPU32(superblock->modification_time);
}

uint32_t
sqsh__data_superblock_block_size(const struct SqshDataSuperblock *superblock) {
	return CX_LE_2_CPU32(superblock->block_size);
}

uint32_t
sqsh__data_superblock_fragment_entry_count(
		const struct SqshDataSuperblock *superblock) {
	return CX_LE_2_CPU32(superblock->fragment_entry_count);
}

uint16_t
sqsh__data_superblock_compression_id(
		const struct SqshDataSuperblock *superblock) {
	return CX_LE_2_CPU16(superblock->compression_id);
}

uint16_t
sqsh__data_superblock_block_log(const struct SqshDataSuperblock *superblock) {
	return CX_LE_2_CPU16(superblock->block_log);
}

uint16_t
sqsh__data_superblock_flags(const struct SqshDataSuperblock *superblock) {
	return CX_LE_2_CPU16(superblock->flags);
}

uint16_t
sqsh__data_superblock_id_count(const struct SqshDataSuperblock *superblock) {
	return CX_LE_2_CPU16(superblock->id_count);
}

uint16_t
sqsh__data_superblock_version_major(
		const struct SqshDataSuperblock *superblock) {
	return CX_LE_2_CPU16(superblock->version_major);
}

uint16_t
sqsh__data_superblock_version_minor(
		const struct SqshDataSuperblock *superblock) {
	return CX_LE_2_CPU16(superblock->version_minor);
}

uint64_t
sqsh__data_superblock_root_inode_ref(
		const struct SqshDataSuperblock *superblock) {
	return CX_LE_2_CPU64(superblock->root_inode_ref);
}

uint64_t
sqsh__data_superblock_bytes_used(const struct SqshDataSuperblock *superblock) {
	return CX_LE_2_CPU64(superblock->bytes_used);
}

uint64_t
sqsh__data_superblock_id_table_start(
		const struct SqshDataSuperblock *superblock) {
	return CX_LE_2_CPU64(superblock->id_table_start);
}

uint64_t
sqsh__data_superblock_xattr_id_table_start(
		const struct SqshDataSuperblock *superblock) {
	return CX_LE_2_CPU64(superblock->xattr_id_table_start);
}

uint64_t
sqsh__data_superblock_inode_table_start(
		const struct SqshDataSuperblock *superblock) {
	return CX_LE_2_CPU64(superblock->inode_table_start);
}

uint64_t
sqsh__data_superblock_directory_table_start(
		const struct SqshDataSuperblock *superblock) {
	return CX_LE_2_CPU64(superblock->directory_table_start);
}

uint64_t
sqsh__data_superblock_fragment_table_start(
		const struct SqshDataSuperblock *superblock) {
	return CX_LE_2_CPU64(superblock->fragment_table_start);
}

uint64_t
sqsh__data_superblock_export_table_start(
		const struct SqshDataSuperblock *superblock) {
	return CX_LE_2_CPU64(superblock->export_table_start);
}
