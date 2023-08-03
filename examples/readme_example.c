/**
 * @author Enno Boland (mail@eboland.de)
 * @file readme_example.c
 */

#include <assert.h>
#include <sqsh.h>
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	struct SqshArchive *archive =
			sqsh_archive_new("/path/to/archive.squashfs", NULL, NULL);

	uint8_t *contents = sqsh_file_content(archive, "/path/to/file");
	assert(contents != NULL);
	const size_t size = sqsh_file_size(archive, "/path/to/file");
	fwrite(contents, 1, size, stdout);
	free(contents);

	char **files = sqsh_directory_list(archive, "/path/to/directory", NULL);
	assert(files != NULL);
	for (int i = 0; files[i] != NULL; i++) {
		printf("%s\n", files[i]);
	}
	free(files);

	sqsh_archive_free(archive);
	return 0;
}
