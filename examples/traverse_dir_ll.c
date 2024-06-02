/**
 * @author Enno Boland (mail@eboland.de)
 * @file traverse_dir_ll.c
 *
 * This is an example program that lists the top level files in a squashfs
 * archive. It uses low level variants of the API.
 */

#include <sqsh.h>
#include <stdio.h>

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
			sqsh_archive_open(argv[1], &config, &error_code);
	if (error_code != 0) {
		sqsh_perror(error_code, "sqsh_archive_new");
		return 1;
	}
	const struct SqshSuperblock *superblock = sqsh_archive_superblock(archive);
	uint64_t inode_root_ref = sqsh_superblock_inode_root_ref(superblock);
	struct SqshFile *file =
			sqsh_open_by_ref(archive, inode_root_ref, &error_code);
	if (error_code != 0) {
		sqsh_perror(error_code, "sqsh_file_new");
		return 1;
	}

	struct SqshTreeTraversal *traversal =
			sqsh_tree_traversal_new(file, &error_code);
	if (error_code != 0) {
		sqsh_perror(error_code, "sqsh_directory_iterator_new");
		return 1;
	}

	while (sqsh_tree_traversal_next(traversal, &error_code)) {
		enum SqshTreeTraversalState state =
				sqsh_tree_traversal_state(traversal);
		if (state == SQSH_TREE_TRAVERSAL_STATE_DIRECTORY_END) {
			continue;
		}
		size_t depth = sqsh_tree_traversal_depth(traversal);
		size_t segment_size;
		const char *segment;
		for (sqsh_index_t i = 0; i < depth; i++) {
			segment = sqsh_tree_traversal_path_segment(
					traversal, &segment_size, i);
			fputc('/', stdout);
			fwrite(segment, 1, segment_size, stdout);
		}
		fputc('\n', stdout);
	}
	if (error_code < 0) {
		sqsh_perror(error_code, "sqsh_directory_iterator_next");
		return 1;
	}

	sqsh_tree_traversal_free(traversal);
	sqsh_close(file);
	sqsh_archive_close(archive);
	return 0;
}
