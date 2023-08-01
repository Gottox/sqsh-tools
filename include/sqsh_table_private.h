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
 * @file         sqsh_table_private.h
 */

#ifndef SQSH_TABLE_PRIVATE_H
#define SQSH_TABLE_PRIVATE_H

#include "sqsh_extract_private.h"
#include "sqsh_mapper_private.h"
#include "sqsh_table.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SqshInode;
struct SqshDataFragment;
struct SqshDataXattrLookupTable;

/***************************************
 * table/table.c
 */

/**
 * @brief A generic table as used in an archive.
 */
struct SqshTable {
	/**
	 * @privatesection
	 */
	struct SqshArchive *sqsh;
	struct SqshMapper *mapper;
	struct SqshMapReader lookup_table;
	uint64_t start_block;
	size_t element_size;
	size_t element_count;
};

/**
 * @internal
 * @memberof SqshTable
 * @brief Initializes a table with a SQSH context.
 *
 * @param[out] table The table to initialize.
 * @param[in]  sqsh The SQSH context to use for the table.
 * @param[in]  start_block The starting offset of the table in blocks.
 * @param[in]  element_size The size of each element in the table.
 * @param[in]  element_count The number of elements in the table.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_EXPORT SQSH_NO_UNUSED int sqsh__table_init(
		struct SqshTable *table, struct SqshArchive *sqsh, uint64_t start_block,
		size_t element_size, size_t element_count);

/**
 * @internal
 * @memberof SqshTable
 * @brief Cleans up a table.
 *
 * @param[in] table The table to clean up.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_EXPORT int sqsh__table_cleanup(struct SqshTable *table);

/***************************************
 * table/id_table.c
 */

/**
 * @brief The id table of an archive.
 */
struct SqshIdTable {
	/**
	 * @privatesection
	 */
	struct SqshTable table;
};

/**
 * @internal
 * @memberof SqshIdTable
 * @brief Initializes a table with a SQSH context.
 *
 * @param[out] table The table to initialize.
 * @param[in]  sqsh The SQSH context to use for the table.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_EXPORT int
sqsh__id_table_init(struct SqshIdTable *table, struct SqshArchive *sqsh);

/**
 * @internal
 * @memberof SqshIdTable
 * @brief Cleans up a table.
 *
 * @param[in] table The table to clean up.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_EXPORT int sqsh__id_table_cleanup(struct SqshIdTable *table);

/***************************************
 * table/export_table.c
 */

/**
 * @brief The export table.
 */
struct SqshExportTable {
	/**
	 * @privatesection
	 */
	struct SqshTable table;
};

/**
 * @internal
 * @memberof SqshExportTable
 * @brief Initializes a table with a SQSH context.
 *
 * @param[out] table The table to initialize.
 * @param[in]  sqsh The SQSH context to use for the table.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_EXPORT int sqsh__export_table_init(
		struct SqshExportTable *table, struct SqshArchive *sqsh);

/**
 * @internal
 * @memberof SqshExportTable
 * @brief Cleans up a table.
 *
 * @param[in] table The table to clean up.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_EXPORT int sqsh__export_table_cleanup(struct SqshExportTable *table);

/***************************************
 * file/fragment_table.c
 */

/**
 * @brief The fragment table of an archive.
 */
struct SqshFragmentTable {
	/**
	 * @privatesection
	 */
	struct SqshTable table;
	struct SqshExtractManager extract_manager;
};

/**
 * @internal
 * @memberof SqshFragmentTable
 * @brief Initializes a fragment table with a SQSH context.
 *
 * @param[out] table The fragment table to initialize.
 * @param[in]  sqsh The SQSH context to use for the fragment table.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_EXPORT SQSH_NO_UNUSED int sqsh__fragment_table_init(
		struct SqshFragmentTable *table, struct SqshArchive *sqsh);

/**
 * @internal
 * @memberof SqshFragmentTable
 * @brief Retrieves the fragment meta informations from the fragment table for
 * the given inode.
 *
 * @param[in]  table The fragment table to use.
 * @param[in]  inode The inode to retrieve the fragment meta informations for.
 * @param[out] fragment The fragment meta informations.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_EXPORT int sqsh__fragment_table_get(
		const struct SqshFragmentTable *table, const struct SqshInode *inode,
		struct SqshDataFragment *fragment);

/**
 * @internal
 * @memberof SqshFragmentTable
 * @brief Cleans up a fragment table.
 *
 * @param[in] table The fragment table to clean up.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_EXPORT int
sqsh__fragment_table_cleanup(struct SqshFragmentTable *table);

/***************************************
 * table/xattr_table.c
 */

/**
 * @brief The extended attribute lookup table.
 */
struct SqshXattrTable {
	/**
	 * @privatesection
	 */
	struct SqshMapReader header;
	struct SqshTable table;
};

/**
 * @memberof SqshXattrTable
 * @brief Retrieves the starting offset of the table.
 *
 * @param[in] table The extended attribute table to retrieve the offset from.
 *
 * @return The starting offset of the table.
 */
uint64_t sqsh_xattr_table_start(struct SqshXattrTable *table);

/**
 * @memberof SqshXattrTable
 * @brief Retrieves an extended attribute from the table.
 *
 * @param[in]  table The extended attribute table to retrieve the attribute
 * from.
 * @param[in]  index The index of the attribute to retrieve.
 * @param[out] target The extended attribute lookup table to store the attribute
 * in.
 *
 * @return 0 on success, a negative value on error.
 */
SQSH_NO_UNUSED int sqsh_xattr_table_get(
		const struct SqshXattrTable *table, sqsh_index_t index,
		struct SqshDataXattrLookupTable *target);

#ifdef __cplusplus
}
#endif
#endif /* SQSH_TABLE_PRIVATE_H */
