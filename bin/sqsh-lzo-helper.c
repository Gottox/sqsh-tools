/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023, Enno Boland <g@s01.de>                                 *
 *                                                                            *
 * This program is free software: you can redistribute it and/or modify       *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation, version 3.                                   *
 *                                                                            *
 * This program is distributed in the hope that it will be useful, but        *
 * WITHOUT ANY WARRANTY; without even the implied warranty of                 *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU           *
 * General Public License for more details.                                   *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program. If not, see <http://www.gnu.org/licenses/>.       *
 *                                                                            *
 ******************************************************************************/

/**
 * @author       Enno Boland (mail@eboland.de)
 * @file         sqsh-lzo2-helper.c
 */

#include <alloca.h>
#include <lzo/lzo1x.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static int
sqsh_uncompress(void) {
	int rv = 0;
	uint64_t compressed_size = 0;
	uint64_t target_size = 0;
	uint8_t *compressed = NULL;
	uint8_t *target = NULL;
	char wrkmem[LZO1X_1_MEM_COMPRESS] = {0};

	rv = fread(&target_size, sizeof(uint64_t), 1, stdin);
	if (rv != 1) {
		rv = EXIT_FAILURE;
		goto out;
	}
	rv = fread(&compressed_size, sizeof(uint64_t), 1, stdin);
	if (rv != 1) {
		rv = EXIT_FAILURE;
		goto out;
	}
	compressed = calloc(compressed_size, sizeof(uint8_t));
	target = calloc(target_size, sizeof(uint8_t));
	rv = fread(compressed, sizeof(uint8_t), compressed_size, stdin);
	if (rv < 0 || (uint64_t)rv != compressed_size) {
		rv = EXIT_FAILURE;
		goto out;
	}

	const int64_t compress_rv = lzo1x_decompress_safe(
			compressed, compressed_size, target, &target_size, wrkmem);
	if (compress_rv < 0) {
		target_size = 0;
	}

	rv = fwrite(&compress_rv, sizeof(int64_t), 1, stdout);
	if (rv != 1) {
		rv = EXIT_FAILURE;
		goto out;
	}
	rv = fwrite(&target_size, sizeof(uint64_t), 1, stdout);
	if (rv != 1) {
		rv = EXIT_FAILURE;
		goto out;
	}
	rv = fwrite(target, sizeof(uint8_t), target_size, stdout);
	if (rv < 0 || (uint64_t)rv != target_size) {
		rv = EXIT_FAILURE;
		goto out;
	}
	fflush(stdout);

out:
	free(compressed);
	free(target);
	return rv;
}

int
main(int argc, char *argv[]) {
	int rv = 0;
	(void)argv;

	if (argc != 1) {
		fprintf(stderr,
				"Usage: %s\n"
				"This executable must only been spawned by libsqsh\n",
				argv[0]);
		return EXIT_FAILURE;
	}

	while (rv == 0 && feof(stdin) == 0) {
		rv = sqsh_uncompress();
	}

	return EXIT_SUCCESS;
}
