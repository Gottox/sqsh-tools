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
 * @file         map_manager.c
 */

#include "../../include/sqsh_mapper_private.h"

#include "../../include/sqsh.h"
#include "../../include/sqsh_error.h"
#include "../utils.h"

#include <stdint.h>
#include <string.h>

#include "sqsh_primitive_private.h"

static void
map_cleanup_cb(void *data) {
	(void)data;
}

int
sqsh__map_manager_init(
		struct SqshMapManager *manager, const void *input,
		const struct SqshConfig *config) {
	int rv;
	size_t map_size;

	rv = sqsh__mapper_init(&manager->mapper, input, config);
	if (rv < 0) {
		goto out;
	}

	map_size = SQSH_DIVIDE_CEIL(
			sqsh__mapper_size(&manager->mapper),
			sqsh__mapper_block_size(&manager->mapper));

	rv = sqsh__ref_count_array_init(
			&manager->maps, map_size, sizeof(struct SqshMapping),
			map_cleanup_cb);

out:
	if (rv < 0) {
		sqsh__map_manager_cleanup(manager);
	}
	return rv;
}

uint64_t
sqsh__map_manager_size(const struct SqshMapManager *manager) {
	return sqsh__mapper_size(&manager->mapper);
}

size_t
sqsh__map_manager_chunk_size(const struct SqshMapManager *manager) {
	(void)manager;
	return sqsh__mapper_block_size(&manager->mapper);
}

size_t
sqsh__map_manager_chunk_count(const struct SqshMapManager *manager) {
	(void)manager;
	return sqsh__ref_count_array_size(&manager->maps);
}

int
sqsh__map_manager_get(struct SqshMapManager *manager, sqsh_index_t index) {
	(void)manager;
	(void)index;
	return 0;
}

int
sqsh__map_manager_cleanup(struct SqshMapManager *manager) {
	(void)manager;
	return 0;
}
