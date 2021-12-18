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
 * @file        : resolve_path
 * @created     : Friday Sep 10, 2021 10:06:27 CEST
 */

#include "resolve_path.h"
#include "context/directory_context.h"
#include "context/inode_context.h"
#include "context/superblock_context.h"
#include "data/directory.h"
#include "data/inode.h"
#include "error.h"
#include "hsqs.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static const char *
find_next_segment(const char *segment) {
	segment = strchr(segment, '/');
	if (segment == NULL) {
		return NULL;
	} else {
		return segment + 1;
	}
}

size_t
get_segment_len(const char *segment) {
	const char *next_segment = find_next_segment(segment);
	if (next_segment == NULL) {
		return strlen(segment);
	} else {
		return next_segment - segment - 1;
	}
}

static int
count_path_segments(const char *path) {
	int count = 0;
	for (; path; path = find_next_segment(path)) {
		count++;
	}

	return count;
}

static int
find_inode_ref(
		uint64_t *target, uint64_t dir_ref, struct Hsqs *hsqs, const char *name,
		const size_t name_len) {
	struct HsqsInodeContext inode = {0};
	struct HsqsDirectoryContext dir = {0};
	struct HsqsDirectoryIterator iter = {0};
	int rv = 0;
	rv = hsqs_inode_load(&inode, hsqs, dir_ref);
	if (rv < 0) {
		goto out;
	}
	rv = hsqs_directory_init(&dir, &inode);
	if (rv < 0) {
		goto out;
	}
	rv = hsqs_directory_iterator_init(&iter, &dir);
	if (rv < 0) {
		goto out;
	}
	rv = hsqs_directory_iterator_lookup(&iter, name, name_len);
	if (rv < 0) {
		goto out;
	}

	*target = hsqs_directory_iterator_inode_ref(&iter);

out:
	hsqs_directory_iterator_cleanup(&iter);
	hsqs_directory_cleanup(&dir);
	hsqs_inode_cleanup(&inode);
	return rv;
}

int
hsqs_resolve_path(
		struct HsqsInodeContext *inode, struct Hsqs *hsqs, const char *path) {
	int i;
	int rv = 0;
	int segment_count = count_path_segments(path) + 1;
	struct HsqsSuperblockContext *superblock = hsqs_superblock(hsqs);
	const char *segment = path;
	uint64_t *inode_refs = calloc(segment_count, sizeof(uint64_t));
	if (inode_refs == NULL) {
		rv = HSQS_ERROR_MALLOC_FAILED;
		goto out;
	}
	inode_refs[0] = hsqs_superblock_inode_root_ref(superblock);

	for (i = 0; segment; segment = find_next_segment(segment)) {
		size_t segment_len = get_segment_len(segment);

		if (segment_len == 0 || (segment_len == 1 && segment[0] == '.')) {
			continue;
		} else if (segment_len == 2 && strncmp(segment, "..", 2) == 0) {
			i = MAX(0, i - 1);
			continue;
		} else {
			uint64_t parent_inode_ref = inode_refs[i];
			i++;
			rv = find_inode_ref(
					&inode_refs[i], parent_inode_ref, hsqs, segment,
					segment_len);
			if (rv < 0) {
				goto out;
			}
		}
	}

	rv = hsqs_inode_load(inode, hsqs, inode_refs[i]);

out:
	free(inode_refs);
	return rv;
}
