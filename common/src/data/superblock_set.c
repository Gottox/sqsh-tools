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
 * @file         superblock_set.c
 */

#include <cextras/endian.h>
#include <sqsh_data_set.h>

/***************************************
 * data/superblock_data.c
 */

void
sqsh__data_superblock_magic_set(
		struct SqshDataSuperblock *superblock, const uint32_t value) {
	superblock->magic = CX_CPU_2_LE32(value);
}

void
sqsh__data_superblock_inode_count_set(
		struct SqshDataSuperblock *superblock, const uint32_t value) {
	superblock->inode_count = CX_CPU_2_LE32(value);
}

void
sqsh__data_superblock_modification_time_set(
		struct SqshDataSuperblock *superblock, const uint32_t value) {
	superblock->modification_time = CX_CPU_2_LE32(value);
}

void
sqsh__data_superblock_block_size_set(
		struct SqshDataSuperblock *superblock, const uint32_t value) {
	superblock->block_size = CX_CPU_2_LE32(value);
}

void
sqsh__data_superblock_fragment_entry_count_set(
		struct SqshDataSuperblock *superblock, const uint32_t value) {
	superblock->fragment_entry_count = CX_CPU_2_LE32(value);
}

void
sqsh__data_superblock_compression_id_set(
		struct SqshDataSuperblock *superblock, const uint16_t value) {
	superblock->compression_id = CX_CPU_2_LE16(value);
}

void
sqsh__data_superblock_block_log_set(
		struct SqshDataSuperblock *superblock, const uint16_t value) {
	superblock->block_log = CX_CPU_2_LE16(value);
}

void
sqsh__data_superblock_flags_set(
		struct SqshDataSuperblock *superblock, const uint16_t value) {
	superblock->flags = CX_CPU_2_LE16(value);
}

void
sqsh__data_superblock_id_count_set(
		struct SqshDataSuperblock *superblock, const uint16_t value) {
	superblock->id_count = CX_CPU_2_LE16(value);
}

void
sqsh__data_superblock_version_major_set(
		struct SqshDataSuperblock *superblock, const uint16_t value) {
	superblock->version_major = CX_CPU_2_LE16(value);
}

void
sqsh__data_superblock_version_minor_set(
		struct SqshDataSuperblock *superblock, const uint16_t value) {
	superblock->version_minor = CX_CPU_2_LE16(value);
}

void
sqsh__data_superblock_root_inode_ref_set(
		struct SqshDataSuperblock *superblock, const uint64_t value) {
	superblock->root_inode_ref = CX_CPU_2_LE64(value);
}

void
sqsh__data_superblock_bytes_used_set(
		struct SqshDataSuperblock *superblock, const uint64_t value) {
	superblock->bytes_used = CX_CPU_2_LE64(value);
}

void
sqsh__data_superblock_id_table_start_set(
		struct SqshDataSuperblock *superblock, const uint64_t value) {
	superblock->id_table_start = CX_CPU_2_LE64(value);
}

void
sqsh__data_superblock_xattr_id_table_start_set(
		struct SqshDataSuperblock *superblock, const uint64_t value) {
	superblock->xattr_id_table_start = CX_CPU_2_LE64(value);
}

void
sqsh__data_superblock_inode_table_start_set(
		struct SqshDataSuperblock *superblock, const uint64_t value) {
	superblock->inode_table_start = CX_CPU_2_LE64(value);
}

void
sqsh__data_superblock_directory_table_start_set(
		struct SqshDataSuperblock *superblock, const uint64_t value) {
	superblock->directory_table_start = CX_CPU_2_LE64(value);
}

void
sqsh__data_superblock_fragment_table_start_set(
		struct SqshDataSuperblock *superblock, const uint64_t value) {
	superblock->fragment_table_start = CX_CPU_2_LE64(value);
}

void
sqsh__data_superblock_export_table_start_set(
		struct SqshDataSuperblock *superblock, const uint64_t value) {
	superblock->export_table_start = CX_CPU_2_LE64(value);
}
