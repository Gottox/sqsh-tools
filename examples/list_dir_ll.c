/**
 * @author Enno Boland (mail@eboland.de)
 * @file list_dir_ll.c
 *
 * This is an example program that lists the top level files in a squashfs
 * archive. It uses low level variants of the API.
 */

#include <sqsh.h>
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char *argv[]) {
	int error_code = 0;
	if (argc != 2) {
		printf("Usage: %s <sqsh-file>\n", argv[0]);
		return 1;
	}
	struct SqshConfig config = {
			// Read the header file to find documentation on these fields.
			// It's safe to set them all to 0.
			.source_mapper = sqsh_mapper_impl_mmap,
			.source_size = 0,
			.mapper_block_size = 0,
			.mapper_lru_size = 0,
			.compression_lru_size = 0,
			.archive_offset = 0,
			.max_symlink_depth = 0,
	};
	struct SqshArchive *archive =
			sqsh_archive_new(argv[1], &config, &error_code);
	if (error_code != 0) {
		sqsh_perror(error_code, "sqsh_archive_new");
		return 1;
	}
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(archive);
	uint64_t inode_root_ref = sqsh_superblock_inode_root_ref(superblock);
	struct SqshInode *inode =
			sqsh_inode_new(archive, inode_root_ref, &error_code);
	if (error_code != 0) {
		sqsh_perror(error_code, "sqsh_inode_new");
		return 1;
	}

	struct SqshDirectoryIterator *iterator =
			sqsh_directory_iterator_new(inode, &error_code);
	if (error_code != 0) {
		sqsh_perror(error_code, "sqsh_directory_iterator_new");
		return 1;
	}

	while ((error_code = sqsh_directory_iterator_next(iterator)) > 0) {
		const char *name = sqsh_directory_iterator_name(iterator);
		size_t size = sqsh_directory_iterator_name_size(iterator);
		fwrite(name, size, 1, stdout);
		fputc('\n', stdout);
	}
	if (error_code < 0) {
		sqsh_perror(error_code, "sqsh_directory_iterator_next");
		return 1;
	}

	sqsh_directory_iterator_free(iterator);
	sqsh_inode_free(inode);
	sqsh_archive_close(archive);
}
