/**
 * @author       Enno Boland (mail@eboland.de)
 * @file         custom_mapper.c
 *
 * This example shows how to use a custom archive mapper that read from a file.
 * Keep in mind, that libsqsh does guard the mapper against concurrent access.
 * So these functions can be considered thread-safe.
 */

#include <assert.h>
#include <errno.h>
#include <sqsh.h>
#include <stdio.h>
#include <stdlib.h>

// This function is called while initialization and gets its input parameter
// from the `input` parameter of sqsh_archive_open(). The `size` parameter is
// initialized from SqshConfig::source_size and expected to be set to actual
// size of the input file.
static int
mapper_init(struct SqshMapper *mapper, const void *input, uint64_t *size) {
	(void)size;
	int rv = 0;
	FILE *file;
	long pos;

	file = fopen(input, "r");
	if (file == NULL) {
		rv = -errno;
		goto out;
	}

	pos = fseek(file, 0, SEEK_END);
	if (pos < 0) {
		rv = -errno;
		goto out;
	}
	*size = (size_t)pos;

	sqsh_mapper_set_user_data(mapper, file);

out:
	return rv;
}

// This function is called when libsqsh needs a chunk of the archive in memory.
// The `offset` parameter is the offset in the archive starting from the
// beginning of the archive. That means that the mapper does not need to handle
// SqshConfig::archive_offset. The `size` parameter is the size of the chunk
// that should be read. The `data` parameter is a double pointer. It is expected
// to be set to a buffer of `size` bytes.
static int
mapper_map(
		const struct SqshMapper *mapper, uint64_t offset, size_t size,
		uint8_t **data) {
	int rv = 0;
	FILE *file = sqsh_mapper_user_data(mapper);

	*data = calloc(size, sizeof(uint8_t));
	if (*data == NULL) {
		rv = -SQSH_ERROR_MALLOC_FAILED;
		goto out;
	}
	rv = fseek(file, offset, SEEK_SET);
	if (rv < 0) {
		rv = -errno;
		goto out;
	}

	if (fread(*data, sizeof(uint8_t), size, file) != size) {
		rv = -1;
		goto out;
	}

out:
	if (rv < 0) {
		free(*data);
	}
	return rv;
}

// This function is called when libsqsh cleans up a chunk of the archive in
// memory. The `data` parameter is the buffer that was allocated in mapper_map.
// The `size` parameter is the size of the buffer.
static int
mapper_unmap(const struct SqshMapper *mapper, uint8_t *data, size_t size) {
	(void)mapper;
	(void)size;
	free(data);
	return 0;
}

// This function is called when the archive is closed. It is expected to clean
// up any resources that were allocated in mapper_init.
static int
mapper_cleanup(struct SqshMapper *mapper) {
	FILE *file = sqsh_mapper_user_data(mapper);
	fclose(file);
	return 0;
}

static const struct SqshMemoryMapperImpl custom_mapper = {
		// The block size is a hint to libsqsh how much data it should request
		// from the mapper at once. This field must be set as it is used as
		// default value. Setting it to 0 is undefined behavior.
		.block_size_hint = 1024 * 1024,
		.init2 = mapper_init,
		.map2 = mapper_map,
		.unmap = mapper_unmap,
		.cleanup = mapper_cleanup,
};

int
main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <file>\n", argv[0]);
		return 1;
	}

	struct SqshConfig config = {
			.source_mapper = &custom_mapper,
	};
	struct SqshArchive *archive = sqsh_archive_open(argv[1], &config, NULL);
	assert(archive != NULL);

	sqsh_archive_close(archive);
	return 0;
}
