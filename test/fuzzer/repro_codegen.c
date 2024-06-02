#include <assert.h>
#include <libgen.h>
#include <stdio.h>

#define HEADER \
	"#include <sqsh.h>\n" \
	"#include <utest.h>\n" \
	"int LLVMFuzzerTestOneInput(char *data, size_t size);"

#define TEST_PRE_FORMAT \
	"UTEST(fuzzer_repro, %s) {\n" \
	"	(void)utest_result;\n" \
	"	unsigned char input[] = {\n"

#define TEST_POST \
	"\n" \
	"		};\n" \
	"	LLVMFuzzerTestOneInput((char *)input, sizeof(input));\n" \
	"}\n"

#define FOOTER "UTEST_MAIN()"

int
main(int argc, char *argv[]) {
	puts("#include <utest.h>\n"
		 "");
	puts(HEADER);
	for (int i = 1; i < argc; i++) {
		printf(TEST_PRE_FORMAT, basename(argv[i]));
		FILE *f = fopen(argv[i], "rb");
		assert(f);
		int c;
		while ((c = fgetc(f)) != EOF) {
			printf("0x%02x, ", c);
		}
		fclose(f);
		puts(TEST_POST);
	}
	puts(FOOTER);
	return 0;
}
