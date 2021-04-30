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

#include "metablock.h"
#include "squash.h"
#include "superblock.h"

struct Squash *
squash_init(uint8_t *buffer, const size_t size, const enum SquashDtor dtor) {
	struct SquashSuperblockWrap *superblock =
		squash_superblock_wrap(buffer, size);
	if (superblock == NULL) {
		errno = EINVAL;
		return NULL;
	}

	struct Squash *squash = calloc(1, sizeof(struct Squash));

	squash->superblock = superblock;
	squash->size = size;
	squash->dtor = dtor;

	if (squash->superblock->flags & SQUASH_SUPERBLOCK_COMPRESSOR_OPTIONS) {
		squash->compression_options =
			squash_metablock_new(squash, SQUASH_SUPERBLOCK_SIZE);
	}

	return squash;
}

struct Squash *
squash_open(const char *path) {
	int fd = -1;
	uint8_t *file_map = MAP_FAILED;
	struct stat st = {0};
	struct Squash *squash = NULL;

	fd = open(path, 0);
	if (fd < 0) {
		goto err;
	}

	if (fstat(fd, &st) < 0) {
		close(fd);
		goto err;
	}

	file_map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (file_map == MAP_FAILED) {
		goto err;
	}

	// mmap outlives the file descriptor, so we can close it now.
	close(fd);
	fd = -1;

	squash = squash_init(file_map, st.st_size, SQUASH_DTOR_MUNMAP);
	if (squash) {
		return squash;
	}

err:
	if (fd >= 0) {
		assert(0 == close(fd));
	}
	if (file_map != MAP_FAILED) {
		assert(0 == munmap(file_map, st.st_size));
	}
	return NULL;
}

int
squash_cleanup(struct Squash *squash) {
	int rv = 0;
	if (squash == NULL) {
		return 0;
	}

	squash_metablock_cleanup(squash->compression_options);

	switch (squash->dtor) {
	case SQUASH_DTOR_FREE:
		free(squash->superblock);
		break;
	case SQUASH_DTOR_MUNMAP:
		rv |= munmap(squash->superblock, squash->size);
		break;
	case SQUASH_DTOR_NONE:
		// noop
		break;
	}

	free(squash);
	return rv;
}
