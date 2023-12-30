#include "cmocka_include.h"
#include "osal_error.h"

static void test_error(void **state)
{
	const char *estr;

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
}

int main(int argc, char **argv)
{
	setenv("CMOCKA_TEST_ABORT", "1", 1);

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_error),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
