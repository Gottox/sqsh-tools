/**
 * @author Enno Boland (mail@eboland.de)
 * @file list_files.c
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
	struct SqshArchive *archive = sqsh_archive_open(argv[1], NULL);
	assert(archive != NULL);
	struct SqshInode *inode = sqsh_open(archive, "/", NULL);
	assert(inode != NULL);
	struct SqshDirectoryIterator *iterator =
			sqsh_directory_iterator_new(inode, NULL);
	assert(iterator != NULL);

	while (sqsh_directory_iterator_next(iterator) > 0) {
		/* Use the _dup() variant here because the _name() variant is not
		 * null-terminated.
		 */
		char *name = sqsh_directory_iterator_name_dup(iterator);
		puts(name);
		free(name);
	}

	sqsh_directory_iterator_free(iterator);
	sqsh_close(inode);
	sqsh_archive_close(archive);
}
