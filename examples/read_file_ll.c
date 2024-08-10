/**
 * @author Enno Boland (mail@eboland.de)
 * @file read_file_ll.c
 *
 * This is an example program that prints the content of a file in a squashfs
 * archive. It uses low level variants of the API.
 */

#include <sqsh.h>
#include <stdio.h>

int
main(int argc, char *argv[]) {
	int error_code = 0;
	if (argc != 3) {
		printf("Usage: %s <sqsh-file> <path>\n", argv[0]);
		return 1;
	}
	struct SqshConfig config = {
			// Read the header file to find documentation on these fields.
			// It's safe to set them all to 0.
			.source_mapper = sqsh_mapper_impl_mmap,
			.source_size = 0,
			.mapper_block_size = 0,
			.mapper_lru_size = 0,
			.metablock_lru_size = 0,
			.data_lru_size = 0,
			.archive_offset = 0,
			.max_symlink_depth = 0,
	};
	struct SqshArchive *archive =
			sqsh_archive_open(argv[1], &config, &error_code);
	if (error_code != 0) {
		sqsh_perror(error_code, "sqsh_archive_new");
		return 1;
	}
	struct SqshFile *file = sqsh_open(archive, argv[2], &error_code);
	if (error_code != 0) {
		sqsh_perror(error_code, "sqsh_open");
		return 1;
	}

	struct SqshFileIterator *iterator =
			sqsh_file_iterator_new(file, &error_code);
	if (error_code != 0) {
		sqsh_perror(error_code, "sqsh_file_iterator_new");
		return 1;
	}

	while (sqsh_file_iterator_next(iterator, SIZE_MAX, &error_code)) {
		const uint8_t *data = sqsh_file_iterator_data(iterator);
		size_t size = sqsh_file_iterator_size(iterator);
		fwrite(data, size, 1, stdout);
	}
	if (error_code < 0) {
		sqsh_perror(error_code, "sqsh_file_iterator_next");
		return 1;
	}

	sqsh_file_iterator_free(iterator);
	sqsh_close(file);
	sqsh_archive_close(archive);
	return 0;
}
