/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

#include "cmocka_include.h"
#include "osal.h"

static int g_timer_count;
static osal_mutex_t *g_timer_count_mtx;
static osal_sem_t *g_timer_sem;

static void timer_count_cb(void *arg)
{
	(void)arg;
	osal_mutex_lock(g_timer_count_mtx);
	g_timer_count++;
	osal_mutex_unlock(g_timer_count_mtx);
	osal_sem_post(g_timer_sem);
}

static int get_timer_count(void)
{
	int n;

	osal_mutex_lock(g_timer_count_mtx);
	n = g_timer_count;
	osal_mutex_unlock(g_timer_count_mtx);
	return n;
}

static void test_timer_init(void **state)
{
	(void)state;
	osal_error_t res;
	uint32_t use;
	uint32_t avail;

	/* reinit while already initialized (via setup's osal_init) is a no-op */
	res = osal_timer_init(NULL);
	assert_int_equal(res, OSAL_E_OK);

	use = osal_timer_use();
	assert_int_equal(use, 0);
	avail = osal_timer_avail();
	assert_int_equal(avail, OSAL_TIMER_NUM_MAX);
}

static void test_timer_create_invalid(void **state)
{
	(void)state;
	osal_timer_t *timer;

	timer = osal_timer_create(NULL, NULL);
	assert_null(timer);
}

static void test_timer_start_invalid(void **state)
{
	(void)state;
	osal_error_t res;
	osal_timer_t *timer;

	timer = osal_timer_create(timer_count_cb, NULL);
	assert_non_null(timer);

	res = osal_timer_start(NULL, 1000, false);
	assert_int_equal(res, OSAL_E_PARAM);

	res = osal_timer_start(timer, 0, false);
	assert_int_equal(res, OSAL_E_PARAM);

	osal_timer_delete(timer);
}

static void test_timer_oneshot(void **state)
{
	(void)state;
	osal_error_t res;
	osal_timer_t *timer;

	g_timer_count = 0;
	timer = osal_timer_create(timer_count_cb, NULL);
	assert_non_null(timer);

	res = osal_timer_start(timer, 10000 /* 10ms */, false);
	assert_int_equal(res, OSAL_E_OK);

	res = osal_sem_waittime(g_timer_sem, 500000 /* 500ms */);
	assert_int_equal(res, OSAL_E_OK);
	assert_int_equal(get_timer_count(), 1);

	/* One-shot: confirm it really doesn't repeat. */
	osal_usleep(50000);
	assert_int_equal(get_timer_count(), 1);

	osal_timer_delete(timer);
}

static void test_timer_periodic(void **state)
{
	(void)state;
	osal_error_t res;
	osal_timer_t *timer;
	int i;
	int stopped_count;

	g_timer_count = 0;
	timer = osal_timer_create(timer_count_cb, NULL);
	assert_non_null(timer);

	res = osal_timer_start(timer, 10000 /* 10ms */, true);
	assert_int_equal(res, OSAL_E_OK);

	for (i = 0; i < 3; i++) {
		res = osal_sem_waittime(g_timer_sem, 500000 /* 500ms */);
		assert_int_equal(res, OSAL_E_OK);
	}
	assert_true(get_timer_count() >= 3);

	osal_timer_stop(timer);
	/* Let any expiration already queued right before stop drain out. */
	osal_usleep(50000);
	stopped_count = get_timer_count();
	osal_usleep(50000);
	assert_int_equal(get_timer_count(), stopped_count);

	osal_timer_delete(timer);
}

static void test_timer_stop_before_fire(void **state)
{
	(void)state;
	osal_error_t res;
	osal_timer_t *timer;

	g_timer_count = 0;
	timer = osal_timer_create(timer_count_cb, NULL);
	assert_non_null(timer);

	/* Long enough that it cannot fire before we stop it below. */
	res = osal_timer_start(timer, 5000000 /* 5s */, false);
	assert_int_equal(res, OSAL_E_OK);

	osal_timer_stop(timer);
	osal_usleep(50000);
	assert_int_equal(get_timer_count(), 0);

	osal_timer_delete(timer);
}

static osal_timer_t *g_dic_timer;
static int g_dic_delete_calls;

static void dic_callback(void *arg)
{
	(void)arg;
	/* Regression check for the Phase 2 timer race fix: deleting a timer
	 * from within its own currently-running callback must be rejected
	 * (logged and returned) rather than freeing the timer out from under
	 * the callback. */
	osal_timer_delete(g_dic_timer);
	g_dic_delete_calls++;
	osal_sem_post(g_timer_sem);
}

static void test_timer_delete_in_callback(void **state)
{
	(void)state;
	osal_error_t res;

	g_dic_delete_calls = 0;
	g_dic_timer = osal_timer_create(dic_callback, NULL);
	assert_non_null(g_dic_timer);

	res = osal_timer_start(g_dic_timer, 10000 /* 10ms */, false);
	assert_int_equal(res, OSAL_E_OK);

	/* Wait for the callback to run; it attempts (and must fail) to delete
	 * its own timer, then posts the semaphore so we know it returned
	 * normally instead of aborting the process. */
	res = osal_sem_waittime(g_timer_sem, 500000 /* 500ms */);
	assert_int_equal(res, OSAL_E_OK);
	assert_int_equal(g_dic_delete_calls, 1);

	/* The timer must still be valid/usable since the in-callback delete
	 * was rejected; delete it now that the callback has finished. */
	osal_timer_delete(g_dic_timer);
}

static int setup(void **state)
{
	(void)state;
	osal_init(NULL);
	g_timer_count_mtx = osal_mutex_create();
	g_timer_sem = osal_sem_create();
	return 0;
}

static int teardown(void **state)
{
	(void)state;
	if (g_timer_count_mtx != NULL) {
		osal_mutex_delete(g_timer_count_mtx);
		g_timer_count_mtx = NULL;
	}
	if (g_timer_sem != NULL) {
		osal_sem_delete(g_timer_sem);
		g_timer_sem = NULL;
	}
	osal_deinit();
	return 0;
}

int main(void)
{
	setenv("CMOCKA_TEST_ABORT", "1", 1);

	const struct CMUnitTest tests[] = {
		cmocka_unit_test_setup_teardown(test_timer_init, setup, teardown),
		cmocka_unit_test_setup_teardown(test_timer_create_invalid, setup, teardown),
		cmocka_unit_test_setup_teardown(test_timer_start_invalid, setup, teardown),
		cmocka_unit_test_setup_teardown(test_timer_oneshot, setup, teardown),
		cmocka_unit_test_setup_teardown(test_timer_periodic, setup, teardown),
		cmocka_unit_test_setup_teardown(test_timer_stop_before_fire, setup, teardown),
		cmocka_unit_test_setup_teardown(test_timer_delete_in_callback, setup, teardown),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
