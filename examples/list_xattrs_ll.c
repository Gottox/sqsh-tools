/**
 * @author Enno Boland (mail@eboland.de)
 * @file list_xattrs_ll.c
 *
 * This is an example program that prints all extended attributes of a file in
 * a squashfs archive. It uses low level variants of the API.
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

	struct SqshXattrIterator *iterator =
			sqsh_xattr_iterator_new(file, &error_code);
	if (error_code != 0) {
		sqsh_perror(error_code, "sqsh_xattr_iterator_new");
		return 1;
	}

	while (sqsh_xattr_iterator_next(iterator, &error_code)) {
		const char *prefix = sqsh_xattr_iterator_prefix(iterator);
		const size_t prefix_size = sqsh_xattr_iterator_prefix_size(iterator);
		const char *name = sqsh_xattr_iterator_name(iterator);
		const size_t name_size = sqsh_xattr_iterator_name_size(iterator);
		const char *value = sqsh_xattr_iterator_value(iterator);
		const size_t value_size = sqsh_xattr_iterator_value_size2(iterator);

		printf("%.*s%.*s=%.*s\n", (int)prefix_size, prefix, (int)name_size,
			   name, (int)value_size, value);
	}
	if (error_code < 0) {
		sqsh_perror(error_code, "sqsh_xattr_iterator_next");
		return 1;
	}

	sqsh_xattr_iterator_free(iterator);
	sqsh_close(file);
	sqsh_archive_close(archive);
	return 0;
}
