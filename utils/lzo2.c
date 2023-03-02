/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : lzo2
 * @created     : Wednesday Mar 01, 2023 22:50:06 CET
 */

#include <lzo/lzo1x.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int
main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	unsigned char compress[100];
	unsigned long compress_len = sizeof(compress);
	char wrkmem[LZO1X_1_MEM_COMPRESS];
	lzo1x_1_compress(
			(unsigned char *)argv[1], strlen(argv[1]), compress, &compress_len,
			&wrkmem);

	uint64_t hdr[2] = {512, compress_len};
	fwrite(hdr, sizeof(hdr), 1, stdout);
	fwrite(compress, compress_len, 1, stdout);
}
