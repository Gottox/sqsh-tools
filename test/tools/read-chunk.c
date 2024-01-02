/**
 * @author   Enno Boland (mail@eboland.de)
 * @file     read-chunk.c
 * @created  Wednesday Mar 29, 2023 18:37:10 CEST
 */

#include <sqsh_archive.h>
#include <sqsh_easy.h>
#include <sqsh_error.h>
#include <sqsh_file.h>

#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char *argv[]) {
	int rv = 0;
	if (argc != 5) {
		fprintf(stderr, "Usage: %s archive file offset size\n", argv[0]);
		return EXIT_FAILURE;
	}

	struct SqshArchive *archive = sqsh_archive_open(argv[1], NULL, &rv);
	if (archive == NULL) {
		sqsh_perror(rv, "sqsh_archive_open");
		return EXIT_FAILURE;
	}

	struct SqshFile *file = sqsh_open(archive, argv[2], &rv);
	if (file == NULL) {
		sqsh_perror(rv, "sqsh_open");
		return EXIT_FAILURE;
	}

	struct SqshFileReader *reader = sqsh_file_reader_new(file, &rv);
	if (reader == NULL) {
		sqsh_perror(rv, "sqsh_file_reader_new");
		return EXIT_FAILURE;
	}

	rv = sqsh_file_reader_advance(reader, atoi(argv[3]), atoi(argv[4]));
	if (rv < 0) {
		sqsh_perror(rv, "sqsh_file_reader_advance");
		return EXIT_FAILURE;
	}

	size_t size = sqsh_file_reader_size(reader);
	const uint8_t *data = sqsh_file_reader_data(reader);
	rv = fwrite(data, sizeof(uint8_t), size, stdout);
	if (rv != (int)size) {
		sqsh_perror(rv, "fwrite");
		return EXIT_FAILURE;
	}

	sqsh_file_reader_free(reader);
	sqsh_close(file);
	sqsh_archive_close(archive);
	return EXIT_SUCCESS;
}
