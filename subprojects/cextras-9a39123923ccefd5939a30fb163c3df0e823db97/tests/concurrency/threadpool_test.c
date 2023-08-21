
#define _GNU_SOURCE

#include <assert.h>
#include <cextras/concurrency.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <testlib.h>
#include <unistd.h>

#define LENGTH(x) (sizeof(x) / sizeof(x[0]))

static unsigned int
ackermann(unsigned int m, unsigned int n) {
	if (m == 0) {
		return n + 1;
	} else if (m > 0 && n == 0) {
		return ackermann(m - 1, 1);
	} else {
		return ackermann(m - 1, ackermann(m, n - 1));
	}
}

static void
thread_func_ackermann(void *arg) {
	unsigned int *store = arg;

	*store = ackermann(2, 6);
}

static void
test_init_cleanup(void) {
	cx_threadpool_t pool;
	int rv = 0;

	pool = cx_threadpool_init(2);
	assert(rv == 0);

	rv = cx_threadpool_destroy(pool);
	assert(rv == 0);
}

static void
thread_func_inc(void *arg) {
	atomic_uint *counter = arg;
	int rv = 0;

	usleep(1000);
	atomic_fetch_add(counter, 1);
	assert(rv == PTHREAD_BARRIER_SERIAL_THREAD || rv == 0);
}

static void
test_add_task(void) {
	cx_threadpool_t pool;
	int rv = 0;
	atomic_uint counter = 0;

	pool = cx_threadpool_init(1);
	assert(pool != NULL);

	rv = cx_threadpool_schedule(pool, 0, thread_func_inc, &counter);
	assert(rv == 0);

	while (atomic_load(&counter) != 1) {
		usleep(1000);
	}

	rv = cx_threadpool_destroy(pool);
	assert(rv == 0);
}

static void
test_add_multiple_tasks_ackermann(void) {
	cx_threadpool_t pool;
	int rv = 0;
	unsigned int ackermann_results[100] = {0};

	pool = cx_threadpool_init(0);
	assert(pool != NULL);

	for (size_t i = 0; i < LENGTH(ackermann_results); i++) {
		ackermann_results[i] = i;
		rv = cx_threadpool_schedule(
				pool, 0, thread_func_ackermann, &ackermann_results[i]);
		assert(rv == 0);
	}

	rv = cx_threadpool_destroy(pool);
	assert(rv == 0);

	unsigned int expected = ackermann(2, 6);
	for (size_t i = 0; i < LENGTH(ackermann_results); i++) {
		assert(ackermann_results[i] == expected);
	}
}

static void
test_add_multiple_tasks(void) {
	cx_threadpool_t pool;
	int rv = 0;
	atomic_int counter[10000] = {0};

	pool = cx_threadpool_init(0);
	assert(pool != NULL);

	for (size_t i = 0; i < LENGTH(counter); i++) {
		rv = cx_threadpool_schedule(pool, 0, thread_func_inc, &counter[i]);
		assert(rv == 0);
	}

	rv = cx_threadpool_destroy(pool);
	assert(rv == 0);

	for (size_t i = 0; i < LENGTH(counter); i++) {
		assert(atomic_load(&counter[i]) == 1);
	}
}

DECLARE_TESTS
TEST(test_init_cleanup)
TEST(test_add_task)
TEST(test_add_multiple_tasks)
TEST(test_add_multiple_tasks_ackermann)
END_TESTS
