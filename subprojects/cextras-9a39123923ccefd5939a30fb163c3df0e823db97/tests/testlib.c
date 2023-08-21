/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : testlib
 * @created     : Tuesday Jul 25, 2023 13:56:58 CEST
 */

#include <assert.h>
#include <testlib.h>

static void
test1(void) {}

static void
test_fail(void) {
	assert(0);
}

DECLARE_TESTS
TEST(test1)
NO_TEST(test_fail)
END_TESTS
