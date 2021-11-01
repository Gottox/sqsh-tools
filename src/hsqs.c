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
 * @file        : hsqs
 * @created     : Friday Apr 30, 2021 11:09:40 CEST
 */

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "compression/compression.h"
#include "context/superblock_context.h"
#include "hsqs.h"

int
hsqs_init(
		struct Hsqs *hsqs, uint8_t *buffer, const size_t size,
		const enum HsqsDtor dtor) {
	int rv = 0;

	rv = hsqs_superblock_init(&hsqs->superblock, buffer, size);
	if (rv < 0) {
		return rv;
	}

	hsqs->size = size;
	hsqs->dtor = dtor;

	return rv;
}

int
hsqs_open(struct Hsqs *hsqs, const char *path) {
	int rv = 0;
	int fd = -1;
	uint8_t *file_map = MAP_FAILED;
	struct stat st = {0};

	fd = open(path, 0);
	if (fd < 0) {
		rv = -errno;
		goto err;
	}

	if (fstat(fd, &st) < 0) {
		rv = -errno;
		goto err;
	}

	file_map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (file_map == MAP_FAILED) {
		rv = -errno;
		goto err;
	}

	// mmap outlives the file descriptor, so we can close it now.
	close(fd);
	fd = -1;

	rv = hsqs_init(hsqs, file_map, st.st_size, HSQS_DTOR_MUNMAP);
	if (rv < 0) {
		goto err;
	}
	hsqs->buffer = file_map;

	return rv;
err:
	if (fd >= 0) {
		close(fd);
	}

	if (file_map != MAP_FAILED) {
		munmap(file_map, st.st_size);
	}
	return rv;
}

int
hsqs_cleanup(struct Hsqs *hsqs) {
	int rv = 0;

	hsqs_superblock_cleanup(&hsqs->superblock);

	switch (hsqs->dtor) {
	case HSQS_DTOR_FREE:
		free(hsqs->buffer);
		break;
	case HSQS_DTOR_MUNMAP:
		rv |= munmap(hsqs->buffer, hsqs->size);
		break;
	case HSQS_DTOR_NONE:
		// noop
		break;
	}
	return rv;
}
