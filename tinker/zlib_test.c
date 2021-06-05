/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : zlib_test
 * @created     : Tuesday May 18, 2021 21:57:27 CEST
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

#define BLOCK_SIZE 1024

static int
read_file(const char *filename, uint8_t **buffer, size_t *size) {
	FILE *file;
	int rv = 0;
	uint8_t *p;
	if (filename) {
		file = fopen(filename, "r");
	} else {
		file = stdin;
	}

	if (file == NULL) {
		return -1;
	}

	*buffer = NULL;
	*size = 0;
	do {
		*size += rv;
		*buffer = realloc(*buffer, *size + BUFSIZ);
		p = &(*buffer)[*size];
		rv = fread(p, sizeof(uint8_t), BUFSIZ, file);
	} while (rv > 0);
	fclose(file);

	return 0;
}

int
init_decompress(z_stream *stream, uint8_t *compressed, size_t size) {
	int rv = 0;
	stream->zalloc = Z_NULL;
	stream->zfree = Z_NULL;
	stream->opaque = Z_NULL;
	stream->avail_in = size;
	stream->next_in = compressed;

	rv = inflateInit2(stream, -8);
	if (rv != Z_OK) {
		return -1;
	}

	return rv;
}

int
do_decompress(z_stream *stream, uint8_t **out, size_t *size) {
	int rv = 0;
	uint8_t *tmp = realloc(*out, *size + BLOCK_SIZE);
	if (tmp == NULL) {
		return -1;
	}
	*out = tmp;
	stream->avail_out = BLOCK_SIZE;
	stream->next_out = &(*out)[*size];

	rv = inflate(stream, Z_SYNC_FLUSH);
	switch (rv) {
	case Z_OK:
		*size += BLOCK_SIZE;
		return 0;
	case Z_STREAM_END:
		*size = stream->total_out;
		return *size;
	default:
		return -1;
	}
}

int
cleanup_decompress(z_stream *stream) {
	return 0;
}

int
decompress(uint8_t *compressed, size_t in_size) {
	int rv = 0;
	z_stream stream;
	uint8_t *buffer = NULL;
	size_t buffer_size = 0;

	init_decompress(&stream, compressed, in_size);

	fflush(stdout);
	do {
		rv = do_decompress(&stream, &buffer, &buffer_size);
	} while (rv == 0);

	cleanup_decompress(&stream);

	if (rv < 0) {
		return rv;
	}

	fwrite(buffer, sizeof(uint8_t), buffer_size, stdout);

	return rv;
}

int
main(int argc, char *argv[]) {
	uint8_t *buffer = NULL;
	size_t size = 0;
	const char *filename = NULL;

	if (argc >= 2) {
		filename = argv[1];
	}
	assert(read_file(filename, &buffer, &size) >= 0);
	free(buffer);
	assert(decompress(buffer + 10, size - 10) >= 0);
	// fwrite(buffer+10, sizeof(uint8_t), size-10, stdout);
	return 0;
}
