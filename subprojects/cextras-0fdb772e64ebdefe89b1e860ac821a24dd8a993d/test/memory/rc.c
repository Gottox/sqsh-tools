
#define _GNU_SOURCE

#include <assert.h>
#include <cextras/memory.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <testlib.h>
#include <unistd.h>

struct MyStruct {
	struct CxRc rc;
};

static void
test_simple(void) {
	bool should_free;
	struct MyStruct s = {0};
	cx_rc_init(&s.rc);
	assert(s.rc.count == 1);
	cx_rc_retain(&s.rc);
	assert(s.rc.count == 2);
	should_free = cx_rc_release(&s.rc);
	assert(should_free == false);
	assert(s.rc.count == 1);
	should_free = cx_rc_release(&s.rc);
	assert(should_free == true);
	assert(s.rc.count == 0);
}

DECLARE_TESTS
TEST(test_simple)
END_TESTS
