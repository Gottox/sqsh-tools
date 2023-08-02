/**
 * @author Enno Boland (mail@eboland.de)
 * @file list_dir.c
 *
 * This is an example program that lists the top level files in a squashfs
 * archive.
 */

#include <assert.h>
#include <sqsh.h>
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: %s <sqsh-file>\n", argv[0]);
		return 1;
	}
	struct SqshArchive *archive = sqsh_archive_new(argv[1], NULL, NULL);
	assert(archive != NULL);

	char **dir_list = sqsh_directory_list(archive, "/", NULL);
	assert(dir_list != NULL);

	for (int i = 0; dir_list[i] != NULL; i++) {
		puts(dir_list[i]);
	}

	free(dir_list);
	sqsh_archive_free(archive);
}
