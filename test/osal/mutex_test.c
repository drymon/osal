/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

#include "cmocka_include.h"
#include "osal.h"

static void test_mutex_loop(void)
{
	int res;
	int i;
	uint32_t use;
	uint32_t avail;

	osal_mutex_t *mutex = osal_mutex_create();
	assert_null(mutex);

	res = osal_mutex_init();
	assert_int_equal(res, OSAL_E_OK);

	use = osal_mutex_use();
	assert_int_equal(use, 0);

	avail = osal_mutex_avail();
	assert_int_equal(avail, OSAL_MUTEX_NUM_MAX);

	/* only create, not delete */
	for (i = 0; i < OSAL_MUTEX_NUM_MAX; i++) {
		use = osal_mutex_use();
		assert_int_equal(use, i);

		avail = osal_mutex_avail();
		assert_int_equal(avail, OSAL_MUTEX_NUM_MAX - i);

		mutex = osal_mutex_create();
		assert_non_null(mutex);

		res = osal_mutex_lock(mutex);
		assert_int_equal(res, OSAL_E_OK);

		osal_mutex_unlock(mutex);
	}
	/* no more mutex */
	mutex = osal_mutex_create();
	assert_null(mutex);

	osal_mutex_deinit();
	res = osal_mutex_init();
	assert_int_equal(res, OSAL_E_OK);

	/* create and then delete */
	for (i = 0; i < OSAL_MUTEX_NUM_MAX; i++) {
		use = osal_mutex_use();
		assert_int_equal(use, 0);

		avail = osal_mutex_avail();
		assert_int_equal(avail, OSAL_MUTEX_NUM_MAX);

		mutex = osal_mutex_create();
		assert_non_null(mutex);

		res = osal_mutex_lock(mutex);
		assert_int_equal(res, OSAL_E_OK);

		osal_mutex_unlock(mutex);

		osal_mutex_delete(mutex);
	}
	mutex = osal_mutex_create();
	assert_non_null(mutex);

	osal_mutex_deinit();
}

static void test_mutex_null_params(void **state)
{
	(void)state;
	int res;

	(void)state;

	/* NULL-pointer parameter paths for the lock/unlock/delete APIs must
	 * be rejected safely rather than crashing. */
	res = osal_mutex_lock(NULL);
	assert_int_equal(res, OSAL_E_PARAM);

	res = osal_mutex_unlock(NULL);
	assert_int_equal(res, OSAL_E_PARAM);

	/* delete(NULL) is documented as a safe no-op. */
	osal_mutex_delete(NULL);
}

static void test_mutex(void **state)
{
	int i;
	(void)state;
	for (i = 0; i < 10; i++) {
		test_mutex_loop();
	}
}

int main(void)
{
	setenv("CMOCKA_TEST_ABORT", "1", 1);

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_mutex),
		cmocka_unit_test(test_mutex_null_params),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
