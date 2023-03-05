/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023, Enno Boland <g@s01.de>                                 *
 *                                                                            *
 * This program is free software: you can redistribute it and/or modify       *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation, version 2.                                   *
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
 * @file         lzo-helper.c
 */

#include <lzo/lzo1x.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static int
sqsh_uncompress(void) {
	int rv = 0;
	uint64_t compressed_size = 0;
	uint64_t target_size = 0;
	char wrkmem[LZO1X_1_MEM_COMPRESS] = {0};

	rv = fread(&target_size, sizeof(uint64_t), 1, stdin);
	if (rv != 1) {
		return EXIT_FAILURE;
	}
	rv = fread(&compressed_size, sizeof(uint64_t), 1, stdin);
	if (rv != 1) {
		return EXIT_FAILURE;
	}
	uint8_t compressed[compressed_size];
	uint8_t target[target_size];

	rv = fread(compressed, sizeof(uint8_t), compressed_size, stdin);
	if (rv < 0 || (uint64_t)rv != compressed_size) {
		return EXIT_FAILURE;
	}

	const int64_t compress_rv = lzo1x_decompress_safe(
			compressed, compressed_size, target, &target_size, wrkmem);
	if (compress_rv < 0) {
		target_size = 0;
	}

	rv = fwrite(&compress_rv, sizeof(int64_t), 1, stdout);
	if (rv != 1) {
		return EXIT_FAILURE;
	}
	rv = fwrite(&target_size, sizeof(uint64_t), 1, stdout);
	if (rv != 1) {
		return EXIT_FAILURE;
	}
	rv = fwrite(target, sizeof(uint8_t), target_size, stdout);
	if (rv < 0 || (uint64_t)rv != target_size) {
		return EXIT_FAILURE;
	}
	fflush(stdout);

	return 0;
}

int
main(int argc, char *argv[]) {
	if (argc != 1) {
		fprintf(stderr,
				"Usage: %s\n"
				"This executable must only been spawned by libsqsh\n",
				argv[0]);
		return EXIT_FAILURE;
	}

	for (int rv = 0; rv >= 0 && feof(stdin) == 0;) {
		rv = sqsh_uncompress();
	}

	return EXIT_SUCCESS;
}
