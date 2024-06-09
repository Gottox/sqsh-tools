/**
 * @author Enno Boland (mail@eboland.de)
 * @file list_dir.c
 *
 * This is an example program that traverses a directory in a SquashFS archive.
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
	struct SqshArchive *archive = sqsh_archive_open(argv[1], NULL, NULL);
	assert(archive != NULL);

	char **dir_list = sqsh_easy_tree_traversal(archive, "/xfce-extra", NULL);
	assert(dir_list != NULL);

	for (int i = 0; dir_list[i] != NULL; i++) {
		puts(dir_list[i]);
	}

	free(dir_list);
	sqsh_archive_close(archive);
	return 0;
}
