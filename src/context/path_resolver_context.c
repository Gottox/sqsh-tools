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
 * @file         path_resolver_context.c
 */

#include "../utils.h"
#include <sqsh.h>
#include <sqsh_context.h>
#include <sqsh_error.h>
#include <sqsh_iterator.h>

#include <string.h>

static const char *
path_find_next_segment(const char *segment) {
	segment = strchr(segment, '/');
	if (segment == NULL) {
		return NULL;
	} else {
		return segment + 1;
	}
}

size_t
path_get_segment_len(const char *segment) {
	const char *next_segment = path_find_next_segment(segment);
	if (next_segment == NULL) {
		return strlen(segment);
	} else {
		return next_segment - segment - 1;
	}
}

static int
path_segments_count(const char *path) {
	int count = 0;
	for (; path; path = path_find_next_segment(path)) {
		count++;
	}

	return count;
}

static int
path_find_inode_ref(
		uint64_t *target, uint64_t dir_ref, struct Sqsh *sqsh, const char *name,
		const size_t name_len) {
	struct SqshInodeContext inode = {0};
	struct SqshDirectoryIterator iter = {0};
	int rv = 0;
	rv = sqsh_inode_init_by_ref(&inode, sqsh, dir_ref);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh_directory_iterator_init(&iter, &inode);
	if (rv < 0) {
		goto out;
	}
	rv = sqsh_directory_iterator_lookup(&iter, name, name_len);
	if (rv < 0) {
		goto out;
	}

	*target = sqsh_directory_iterator_inode_ref(&iter);

out:
	sqsh_directory_iterator_cleanup(&iter);
	sqsh_inode_cleanup(&inode);
	return rv;
}

struct SqshPathResolverContext *
sqsh_path_resolver_new(struct Sqsh *sqsh, int *err) {
	struct SqshPathResolverContext *context =
			calloc(1, sizeof(struct SqshPathResolverContext));
	if (context == NULL) {
		return NULL;
	}
	*err = sqsh_path_resolver_init(context, sqsh);
	if (*err < 0) {
		free(context);
		return NULL;
	}
	return context;
}

int
sqsh_path_resolver_init(
		struct SqshPathResolverContext *context, struct Sqsh *sqsh) {
	context->sqsh = sqsh;
	return 0;
}

int
sqsh_path_resolver_resolve(
		struct SqshPathResolverContext *context, struct SqshInodeContext *inode,
		const char *path) {
	int i;
	int rv = 0;
	int segment_count = path_segments_count(path) + 1;
	struct SqshSuperblockContext *superblock = sqsh_superblock(context->sqsh);
	const char *segment = path;
	uint64_t *inode_refs = calloc(segment_count, sizeof(uint64_t));
	if (inode_refs == NULL) {
		rv = SQSH_ERROR_MALLOC_FAILED;
		goto out;
	}
	inode_refs[0] = sqsh_superblock_inode_root_ref(superblock);

	for (i = 0; segment; segment = path_find_next_segment(segment)) {
		size_t segment_len = path_get_segment_len(segment);

		if (segment_len == 0 || (segment_len == 1 && segment[0] == '.')) {
			continue;
		} else if (segment_len == 2 && strncmp(segment, "..", 2) == 0) {
			i = SQSH_MAX(0, i - 1);
			continue;
		}
		uint64_t parent_inode_ref = inode_refs[i];
		i++;
		rv = path_find_inode_ref(
				&inode_refs[i], parent_inode_ref, context->sqsh, segment,
				segment_len);
		if (rv < 0) {
			goto out;
		}
	}

	rv = sqsh_inode_init_by_ref(inode, context->sqsh, inode_refs[i]);

out:
	free(inode_refs);
	return rv;
}

int
sqsh_path_resolver_cleanup(struct SqshPathResolverContext *context) {
	(void)context;
	return 0;
}

int
sqsh_path_resolver_free(struct SqshPathResolverContext *context) {
	if (context == NULL) {
		return 0;
	}
	int rv = sqsh_path_resolver_cleanup(context);
	free(context);
	return rv;
}
