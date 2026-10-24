/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

#include "cmocka_include.h"
#include "osal.h"

OSAL_STATIC_ASSERT(100 > 10);
OSAL_STATIC_ASSERT(100 >= 100);
OSAL_STATIC_ASSERT(10 + 1 > 10);
OSAL_STATIC_ASSERT(10 - 1 < 10);

static void test_error(void **state)
{
	const char *estr;
	(void)state;

	estr = osal_errstr(-1);
	assert_string_equal(estr, "OSAL_UNKNOWN");
	estr = osal_errstr(OSAL_E_MAX);
	assert_string_equal(estr, "OSAL_UNKNOWN");
	estr = osal_errstr(10000);
	assert_string_equal(estr, "OSAL_UNKNOWN");
	estr = osal_errstr(OSAL_E_OK);
	assert_string_equal(estr, "OSAL_E_OK");
	estr = osal_errstr(OSAL_E_PARAM);
	assert_string_equal(estr, "OSAL_E_PARAM");
	estr = osal_errstr(OSAL_E_RESRC);
	assert_string_equal(estr, "OSAL_E_RESRC");
	estr = osal_errstr(OSAL_E_FAILURE);
	assert_string_equal(estr, "OSAL_E_FAILURE");
	estr = osal_errstr(OSAL_E_OSCALL);
	assert_string_equal(estr, "OSAL_E_OSCALL");
	estr = osal_errstr(OSAL_E_TIMEOUT);
	assert_string_equal(estr, "OSAL_E_TIMEOUT");
	estr = osal_errstr(OSAL_E_QFULL);
	assert_string_equal(estr, "OSAL_E_QFULL");
	estr = osal_errstr(OSAL_E_QEMPTY);
	assert_string_equal(estr, "OSAL_E_QEMPTY");
	estr = osal_errstr(OSAL_E_INUSE);
	assert_string_equal(estr, "OSAL_E_INUSE");
	estr = osal_errstr(OSAL_E_NOINIT);
	assert_string_equal(estr, "OSAL_E_NOINIT");
}

int main(void)
{
	setenv("CMOCKA_TEST_ABORT", "1", 1);

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_error),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
