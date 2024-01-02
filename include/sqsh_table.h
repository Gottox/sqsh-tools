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
 * @file         sqsh_table.h
 */

#ifndef SQSH_TABLE_H
#define SQSH_TABLE_H

#include "sqsh_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************
 * table/table.c
 */

struct SqshTable;

/**
 * @memberof SqshTable
 * @brief Retrieves an element from the table.
 *
 * @param[in]  table The table to retrieve the element from.
 * @param[in]  index The index of the element to retrieve.
 * @param[out] target The buffer to store the element in.
 *
 * @return 0 on success, a negative value on error.
 */
int
sqsh_table_get(const struct SqshTable *table, sqsh_index_t index, void *target);

/***************************************
 * table/id_table.c
 */

struct SqshIdTable;

/**
 * @memberof SqshTable
 * @brief Retrieves an element from the table.
 *
 * @param[in]  table The table to retrieve the element from.
 * @param[in]  index The index of the element to retrieve.
 * @param[out] id The buffer to store the element in.
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh_id_table_get(
		const struct SqshIdTable *table, sqsh_index_t index, uint32_t *id);

/***************************************
 * table/export_table.c
 */

struct SqshExportTable;

/**
 * @memberof SqshTable
 * @brief Retrieves an element from the table.
 *
 * @param[in]  table The table to retrieve the element from.
 * @param[in]  inode The index of the element to retrieve.
 * @param[out] inode_ref A pointer to a uint64_t to store the inode reference
 *
 * @return 0 on success, a negative value on error.
 */
int sqsh_export_table_resolve_inode(
		const struct SqshExportTable *table, uint64_t inode,
		uint64_t *inode_ref);

#ifdef __cplusplus
}
#endif
#endif /* SQSH_TABLE_H */
