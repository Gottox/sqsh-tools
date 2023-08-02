/**
 * @author Enno Boland (mail@eboland.de)
 * @file readme_example.c
 */

#include <assert.h>
#include <sqsh.h>

int
main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	int rv;
	struct SqshArchive *archive =
			sqsh_archive_new("/path/to/archive.squashfs", NULL, &rv);
	assert(rv == 0);
	struct SqshInode *file = sqsh_open(archive, "/path/to/file", &rv);
	assert(rv == 0);
	struct SqshFileIterator *iterator = sqsh_file_iterator_new(file, &rv);
	assert(rv == 0);
	while (sqsh_file_iterator_next(iterator, 1) > 0) {
		const uint8_t *data = sqsh_file_iterator_data(iterator);
		size_t size = sqsh_file_iterator_size(iterator);
		printf("Chunk Size: %lu\n", size);
		puts("Data:");
		fwrite(data, 1, size, stdout);
	}
	sqsh_file_iterator_free(iterator);
	sqsh_close(file);
	sqsh_archive_free(archive);
}
