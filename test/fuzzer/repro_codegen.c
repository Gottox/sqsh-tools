#include <assert.h>
#include <ctype.h>
#include <libgen.h>
#include <stdio.h>

#define HEADER \
	"#include <sqsh.h>\n" \
	"#include <testlib.h>\n" \
	"int LLVMFuzzerTestOneInput(char *data, size_t size);"

#define TEST_PRE_FORMAT \
	"static void %s(void) {\n" \
	"	unsigned char input[] = {\n"

#define TEST_POST \
	"\n" \
	"		};\n" \
	"	LLVMFuzzerTestOneInput((char *)input, sizeof(input));\n" \
	"}\n"

static char *
test_name(const char *path) {
	char *name = basename((char *)path);
	for (char *p = name; *p; p++) {
		if (!isalnum(*p)) {
			*p = '_';
		}
	}
	return name;
}

int
main(int argc, char *argv[]) {
	puts("#include <testlib.h>\n"
		 "");
	puts(HEADER);
	for (int i = 1; i < argc; i++) {
		FILE *f = fopen(argv[i], "rb");
		assert(f);

		char *name = test_name(argv[i]);
		printf(TEST_PRE_FORMAT, name);
		int c;
		while ((c = fgetc(f)) != EOF) {
			printf("0x%02x, ", c);
		}
		fclose(f);
		puts(TEST_POST);
	}

	puts("DECLARE_TESTS");
	for (int i = 1; i < argc; i++) {
		char *name = test_name(argv[i]);
		printf("TEST(%s)\n", name);
	}
	puts("END_TESTS");
	return 0;
}
