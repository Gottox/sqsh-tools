/**
 * @author      : Enno Boland (mail@eboland.de)
 */

#include <lz4.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int
main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	char compress[100] = {0};
	unsigned long compress_len = sizeof(compress);

	int rv = LZ4_compress_default(
			argv[1], compress, strlen(argv[1]), compress_len);
	compress_len = rv;

	uint64_t hdr[2] = {512, compress_len};
	fwrite(hdr, sizeof(hdr), 1, stdout);
	fwrite(compress, rv, sizeof(char), stdout);
}
