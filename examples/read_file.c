/**
 * @author Enno Boland (mail@eboland.de)
 * @file read_file.c
 *
 * This is an example program that prints the content of a file in a squashfs
 * archive.
 */

#include <assert.h>
#include <sqsh.h>
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Usage: %s <sqsh-file> <path>\n", argv[0]);
		return 1;
	}
	struct SqshArchive *archive = sqsh_archive_new(argv[1], NULL, NULL);
	assert(archive != NULL);

	char *content = sqsh_file_content(archive, argv[2]);
	assert(content != NULL);
	size_t size = sqsh_file_size(archive, argv[2]);

	fwrite(content, size, 1, stdout);

	free(content);
	sqsh_archive_free(archive);
}
