/**
 * @author Enno Boland (mail@eboland.de)
 * @file list_xattrs.c
 *
 * This is an example program that prints all extended attributes of a file in
 * a squashfs archive.
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
	struct SqshArchive *archive = sqsh_archive_open(argv[1], NULL, NULL);
	assert(archive != NULL);

	char **keys = sqsh_easy_xattr_keys(archive, argv[2], NULL);
	assert(keys != NULL);
	for (int i = 0; keys[i] != NULL; i++) {
		char *value = sqsh_easy_xattr_get(archive, argv[2], keys[i], NULL);
		printf("%s=%s\n", keys[i], value);
		free(value);
	}
	free(keys);

	sqsh_archive_close(archive);
	return 0;
}
