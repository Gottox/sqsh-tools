#define _GNU_SOURCE

#include <assert.h>
#include <cextras/concurrency.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <testlib.h>
#include <unistd.h>

static void
test_simple_future(void) {
	int marker = 42;
	cx_future_t future = cx_future_init(NULL);
	assert(future != NULL);

	cx_future_resolve(future, &marker);

	int *result = cx_future_wait(future);
	assert(result == &marker);

	cx_future_destroy(future);
}

static void
resolver(void *arg) {
	cx_future_t future = arg;

	int *input = cx_future_get_in_value(future);
	assert(input != NULL);

	int *output = malloc(sizeof(*output));
	assert(output != NULL);
	*output = *input;
	cx_future_resolve(future, output);
}

static void
waiter(void *arg) {
	cx_future_t future = arg;

	int *result = cx_future_wait(future);

	assert(*result == 42);
	free(result);
	cx_future_destroy(future);
}

static void
test_first_wait_then_resolve(void) {
	int marker = 42;
	cx_threadpool_t pool = cx_threadpool_init(1);
	assert(pool != NULL);

	cx_future_t future = cx_future_init(&marker);

	cx_threadpool_schedule(pool, 0, waiter, future);

	usleep(10000);

	resolver(future);

	cx_threadpool_destroy(pool);
}

static void
test_first_resolve_then_wait(void) {
	int marker = 42;
	cx_threadpool_t pool = cx_threadpool_init(1);
	assert(pool != NULL);

	cx_future_t future = cx_future_init(&marker);

	cx_threadpool_schedule(pool, 0, resolver, future);

	usleep(10000);

	waiter(future);

	cx_threadpool_destroy(pool);
}

DECLARE_TESTS
TEST(test_simple_future)
TEST(test_first_wait_then_resolve)
TEST(test_first_resolve_then_wait)
END_TESTS
