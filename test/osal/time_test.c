/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

#include "cmocka_include.h"
#include "osal.h"

static void test_clock_time_nondecreasing(void **state)
{
	(void)state;
	osal_error_t res;
	uint64_t prev;
	uint64_t now;
	int i;

	res = osal_clock_time(&prev);
	assert_int_equal(res, OSAL_E_OK);

	for (i = 0; i < 1000; i++) {
		res = osal_clock_time(&now);
		assert_int_equal(res, OSAL_E_OK);
		assert_true(now >= prev);
		prev = now;
	}
}

static void test_usleep_duration(void **state)
{
	(void)state;
	osal_error_t res;
	uint64_t ts1;
	uint64_t ts2;
	uint64_t diff_ns;
	uint32_t sleep_usec = 20000; /* 20ms */

	res = osal_clock_time(&ts1);
	assert_int_equal(res, OSAL_E_OK);

	res = osal_usleep(sleep_usec);
	assert_int_equal(res, OSAL_E_OK);

	res = osal_clock_time(&ts2);
	assert_int_equal(res, OSAL_E_OK);

	diff_ns = ts2 - ts1;
	assert_true(diff_ns >= (uint64_t)sleep_usec * OSAL_USEC_NSEC);
	/* Generous upper bound to tolerate CI scheduling jitter, while still
	 * catching a completely broken (e.g. no-op) sleep implementation. */
	assert_true(diff_ns < (uint64_t)sleep_usec * OSAL_USEC_NSEC + OSAL_SEC_NSEC);
}

static void test_sleep_zero_returns(void **state)
{
	(void)state;
	osal_error_t res;
	uint64_t ts1;
	uint64_t ts2;

	res = osal_clock_time(&ts1);
	assert_int_equal(res, OSAL_E_OK);

	/* sleep(0) should return promptly rather than hang. */
	res = osal_sleep(0);
	assert_int_equal(res, OSAL_E_OK);

	res = osal_clock_time(&ts2);
	assert_int_equal(res, OSAL_E_OK);
	assert_true((ts2 - ts1) < OSAL_SEC_NSEC);
}

static void test_clock_time_null_param(void **state)
{
	(void)state;
	osal_error_t res;

	res = osal_clock_time(NULL);
	assert_int_equal(res, OSAL_E_PARAM);
}

int main(void)
{
	setenv("CMOCKA_TEST_ABORT", "1", 1);

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_clock_time_nondecreasing),
		cmocka_unit_test(test_usleep_duration),
		cmocka_unit_test(test_sleep_zero_returns),
		cmocka_unit_test(test_clock_time_null_param),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
