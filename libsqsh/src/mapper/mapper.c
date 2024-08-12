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
 * @file         mapper.c
 */

#define SQSH__NO_DEPRECATED_FIELD
#include <sqsh_mapper_private.h>

#include <sqsh_archive.h>
#include <sqsh_common_private.h>
#include <sqsh_error.h>

int
sqsh__mapper_init(
		struct SqshMapper *mapper, const void *source,
		const struct SqshConfig *config) {
	int rv = 0;
	uint64_t size = config->source_size;
	if (config->source_mapper) {
		mapper->impl = config->source_mapper;
	} else {
		mapper->impl = sqsh_mapper_impl_mmap;
	}

	if (config->mapper_block_size) {
		mapper->block_size = (size_t)config->mapper_block_size;
	} else {
		mapper->block_size = mapper->impl->block_size_hint;
	}

	if (mapper->impl->init == NULL) {
		rv = mapper->impl->init2(mapper, source, &size);
	} else {
		if (size > SIZE_MAX) {
			rv = -SQSH_ERROR_INTEGER_OVERFLOW;
			goto out;
		}
		size_t small_size = (size_t)size;
		rv = mapper->impl->init(mapper, source, &small_size);
		size = small_size;
	}
	if (rv < 0) {
		goto out;
	}

	mapper->archive_size = size;

out:
	return rv;
}

size_t
sqsh_mapper_block_size(const struct SqshMapper *mapper) {
	return mapper->block_size;
}

size_t
sqsh_mapper_size(const struct SqshMapper *mapper) {
	return (size_t)mapper->archive_size;
}

uint64_t
sqsh_mapper_size2(const struct SqshMapper *mapper) {
	return mapper->archive_size;
}

void
sqsh_mapper_set_user_data(struct SqshMapper *mapper, void *user_data) {
	mapper->user_data = user_data;
}

void *
sqsh_mapper_user_data(const struct SqshMapper *mapper) {
	return mapper->user_data;
}

int
sqsh__mapper_cleanup(struct SqshMapper *mapper) {
	int rv = 0;
	if (mapper->impl != NULL) {
		rv = mapper->impl->cleanup(mapper);
		mapper->impl = NULL;
	}
	return rv;
}
