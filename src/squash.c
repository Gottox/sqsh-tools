/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : squash
 * @created     : Friday Apr 30, 2021 11:09:40 CEST
 */

#include <assert.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "format/metablock.h"
#include "squash.h"
#include "superblock.h"
#include "extractor/extractor.h"

int
squash_init(struct Squash *squash, uint8_t *buffer, const size_t size,
		const enum SquashDtor dtor) {
	int rv = 0;

	rv = squash_superblock_init(&squash->superblock, buffer, size);
	if (rv < 0) {
		return rv;
	}

	squash->size = size;
	squash->dtor = dtor;

	rv = squash_extractor_init(squash, &squash->extractor);
	if (rv < 0) {
		return rv;
	}

	return rv;
}

int
squash_open(struct Squash *squash, const char *path) {
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

	rv = squash_init(squash, file_map, st.st_size, SQUASH_DTOR_MUNMAP);
	if (rv < 0) {
		goto err;
	}

	return rv;
err:
	if (fd >= 0) {
		assert(0 == close(fd));
	}

	if (file_map != MAP_FAILED) {
		assert(0 == munmap(file_map, st.st_size));
	}
	return rv;
}

int
squash_cleanup(struct Squash *squash) {
	int rv = 0;

	if (squash->superblock.wrap->flags & SQUASH_SUPERBLOCK_COMPRESSOR_OPTIONS) {
		rv = squash_extractor_cleanup(&squash->extractor);
		if (rv < 0)
			return rv;
	}

	// TODO this should go into superblock.c
	switch (squash->dtor) {
	case SQUASH_DTOR_FREE:
		free(squash->superblock.wrap);
		break;
	case SQUASH_DTOR_MUNMAP:
		rv |= munmap(squash->superblock.wrap, squash->size);
		break;
	case SQUASH_DTOR_NONE:
		// noop
		break;
	}
	return rv;
}
