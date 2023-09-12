/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : test
 * @created     : Tuesday Jul 25, 2023 13:23:23 CEST
 */

#ifndef TESTLIB_H
#define TESTLIB_H

#ifdef __cplusplus
extern "C" {
#endif

struct TestlibTest {
	void (*func)(void);
	const char *name;
	int enabled;
};

#ifdef __cplusplus
#	define DECLARE_TESTS \
		extern "C" const struct TestlibTest testlib_tests[] = {
#else
#	define DECLARE_TESTS const struct TestlibTest testlib_tests[] = {
#endif

#define TEST(func) {func, #func, 1},
#define NO_TEST(func) {func, #func, 0},

#define END_TESTS \
	{ 0, 0, 0 } \
	} \
	;

#ifdef __cplusplus
}
#endif
#endif /* TESTLIB_H */
