/* BSD 2-Clause License
*
* Copyright (c) 2024, nguyenvannam142@gmail.com
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
*    list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "cmocka_include.h"
#include "osal.h"

#define TEST_MODULE_IDX (OSAL_LOG_MODULE_NUM_MAX - 1)

/* Exercise the OSALOG_* macros themselves (not just osal_log_print), so the
 * OSALOG_MODULE/OSALOG_SRCFILE plumbing in osal_log.h is covered too. */
#define OSALOG_MODULE TEST_MODULE_IDX

static char g_log_buf[OSAL_LOG_STRING_SIZE];
static int g_log_calls;

static void test_log_output(char *logstr)
{
	g_log_calls++;
	snprintf(g_log_buf, sizeof(g_log_buf), "%s", logstr);
}

static void reset_capture(void)
{
	g_log_calls = 0;
	g_log_buf[0] = 0;
}

static void test_log_init_reinit(void **state)
{
	(void)state;
	osal_error_t res;

	/* Not yet initialized (setup_clean guarantees this): NULL is rejected. */
	res = osal_log_init(NULL);
	assert_int_equal(res, OSAL_E_PARAM);

	res = osal_log_init(test_log_output);
	assert_int_equal(res, OSAL_E_OK);

	/* Re-init while already initialized is a no-op: the argument is
	 * ignored and the previously installed output function is kept. */
	res = osal_log_init(NULL);
	assert_int_equal(res, OSAL_E_OK);

	reset_capture();
	res = osal_log_module_init(TEST_MODULE_IDX, "reinit", OSALOG_LEVEL_INFO, false);
	assert_int_equal(res, OSAL_E_OK);
	res = osal_log_print(TEST_MODULE_IDX, false, OSALOG_LEVEL_INFO, "still alive\n");
	assert_int_equal(res, OSAL_E_OK);
	assert_int_equal(g_log_calls, 1);
}

static void test_log_module_init_dup(void **state)
{
	(void)state;
	osal_error_t res;

	res = osal_log_module_init(TEST_MODULE_IDX, "first", OSALOG_LEVEL_INFO, false);
	assert_int_equal(res, OSAL_E_OK);

	res = osal_log_module_init(TEST_MODULE_IDX, "second", OSALOG_LEVEL_INFO, false);
	assert_int_equal(res, OSAL_E_INUSE);
}

static void test_log_module_init_invalid(void **state)
{
	(void)state;
	osal_error_t res;

	res = osal_log_module_init(OSAL_LOG_MODULE_NUM_MAX, "bad_idx", OSALOG_LEVEL_INFO, false);
	assert_int_equal(res, OSAL_E_PARAM);

	res = osal_log_module_init(TEST_MODULE_IDX, "bad_level", OSALOG_LEVEL_MAX, false);
	assert_int_equal(res, OSAL_E_PARAM);
}

static void test_log_level_filter(void **state)
{
	(void)state;
	osal_error_t res;

	res = osal_log_module_init(TEST_MODULE_IDX, "test", OSALOG_LEVEL_ERROR, false);
	assert_int_equal(res, OSAL_E_OK);

	/* WARN is more verbose than the module's ERROR threshold: filtered out. */
	reset_capture();
	res = osal_log_print(TEST_MODULE_IDX, false, OSALOG_LEVEL_WARN, "should be filtered\n");
	assert_int_equal(res, OSAL_E_OK);
	assert_int_equal(g_log_calls, 0);

	/* ERROR is at the threshold: printed, with the module name embedded. */
	reset_capture();
	res = osal_log_print(TEST_MODULE_IDX, false, OSALOG_LEVEL_ERROR, "should print\n");
	assert_int_equal(res, OSAL_E_OK);
	assert_int_equal(g_log_calls, 1);
	assert_non_null(strstr(g_log_buf, "should print"));
	assert_non_null(strstr(g_log_buf, "test"));

	/* Raise the level at runtime: WARN now passes too. */
	res = osal_log_module_change(TEST_MODULE_IDX, OSALOG_LEVEL_WARN);
	assert_int_equal(res, OSAL_E_OK);

	reset_capture();
	res = osal_log_print(TEST_MODULE_IDX, false, OSALOG_LEVEL_WARN, "now visible\n");
	assert_int_equal(res, OSAL_E_OK);
	assert_int_equal(g_log_calls, 1);

	/* Silence the module entirely. */
	res = osal_log_module_change(TEST_MODULE_IDX, OSALOG_LEVEL_NONE);
	assert_int_equal(res, OSAL_E_OK);

	reset_capture();
	res = osal_log_print(TEST_MODULE_IDX, false, OSALOG_LEVEL_FATAL, "silenced\n");
	assert_int_equal(res, OSAL_E_OK);
	assert_int_equal(g_log_calls, 0);
}

static void test_log_macros(void **state)
{
	(void)state;
	osal_error_t res;

	res = osal_log_module_init(TEST_MODULE_IDX, "macro", OSALOG_LEVEL_INFO, false);
	assert_int_equal(res, OSAL_E_OK);

	/* OSALOG_ERROR/OSALOG_INFO route through OSALOG_HELPER, which prepends
	 * "<file>:<line>|" ahead of the caller's message. */
	reset_capture();
	OSALOG_ERROR("boom %d\n", 42);
	assert_int_equal(g_log_calls, 1);
	assert_non_null(strstr(g_log_buf, "log_test.c:"));
	assert_non_null(strstr(g_log_buf, "boom 42"));

	/* DEBUG is more verbose than the module's INFO threshold: filtered out. */
	reset_capture();
	OSALOG_DEBUG("should be filtered\n");
	assert_int_equal(g_log_calls, 0);

	/* OSALOGTS_* variant forces a timestamp prefix ahead of "<file>:<line>|". */
	reset_capture();
	OSALOGTS_INFO("with timestamp\n");
	assert_int_equal(g_log_calls, 1);
	assert_non_null(strstr(g_log_buf, "with timestamp"));
	assert_true(strchr(g_log_buf, '|') != strrchr(g_log_buf, '|'));
}

static void test_log_print_unregistered_module(void **state)
{
	(void)state;
	osal_error_t res;

	reset_capture();
	res = osal_log_print(TEST_MODULE_IDX, false, OSALOG_LEVEL_ERROR, "nobody home\n");
	assert_int_equal(res, OSAL_E_NOINIT);
	assert_int_equal(g_log_calls, 0);
}

static void test_log_deinit_safety(void **state)
{
	(void)state;
	osal_error_t res;

	res = osal_log_module_init(TEST_MODULE_IDX, "test", OSALOG_LEVEL_INFO, false);
	assert_int_equal(res, OSAL_E_OK);

	osal_log_deinit();

	/* After deinit, printing to a previously-registered module must fail
	 * cleanly (module table cleared) rather than crash. */
	reset_capture();
	res = osal_log_print(TEST_MODULE_IDX, false, OSALOG_LEVEL_ERROR, "gone\n");
	assert_int_equal(res, OSAL_E_NOINIT);
	assert_int_equal(g_log_calls, 0);

	/* Deinit again must be idempotent / safe. */
	osal_log_deinit();

	/* And the subsystem must be fully usable again after a fresh init. */
	res = osal_log_init(test_log_output);
	assert_int_equal(res, OSAL_E_OK);
	res = osal_log_module_init(TEST_MODULE_IDX, "test", OSALOG_LEVEL_INFO, false);
	assert_int_equal(res, OSAL_E_OK);
	reset_capture();
	res = osal_log_print(TEST_MODULE_IDX, false, OSALOG_LEVEL_INFO, "back\n");
	assert_int_equal(res, OSAL_E_OK);
	assert_int_equal(g_log_calls, 1);
}

static void test_log_module_change_invalid(void **state)
{
	(void)state;
	osal_error_t res;

	/* Out-of-range index or level must be rejected. */
	res = osal_log_module_change(OSAL_LOG_MODULE_NUM_MAX, OSALOG_LEVEL_INFO);
	assert_int_equal(res, OSAL_E_PARAM);

	res = osal_log_module_change(TEST_MODULE_IDX, OSALOG_LEVEL_MAX);
	assert_int_equal(res, OSAL_E_PARAM);

	/* Changing the level of a module that was never registered (inuse ==
	 * false) must report NOINIT rather than silently "succeeding". */
	res = osal_log_module_change(TEST_MODULE_IDX, OSALOG_LEVEL_INFO);
	assert_int_equal(res, OSAL_E_NOINIT);
}

static void test_log_print_invalid_params(void **state)
{
	(void)state;
	osal_error_t res;

	res = osal_log_module_init(TEST_MODULE_IDX, "test", OSALOG_LEVEL_INFO, false);
	assert_int_equal(res, OSAL_E_OK);

	/* Out-of-range module index is rejected. Level is only compared
	 * against the module threshold (no range validation), so an
	 * excessively verbose level is simply filtered out (no print, but
	 * still OSAL_E_OK). */
	reset_capture();
	res = osal_log_print(OSAL_LOG_MODULE_NUM_MAX, false, OSALOG_LEVEL_INFO, "x\n");
	assert_int_equal(res, OSAL_E_PARAM);
	assert_int_equal(g_log_calls, 0);

	reset_capture();
	res = osal_log_print(TEST_MODULE_IDX, false, OSALOG_LEVEL_MAX, "x\n");
	assert_int_equal(res, OSAL_E_OK);
	assert_int_equal(g_log_calls, 0);
}

static int setup_clean(void **state)
{
	(void)state;
	reset_capture();
	osal_log_deinit();
	return 0;
}

static int setup_ready(void **state)
{
	(void)state;
	reset_capture();
	osal_log_deinit();
	osal_log_init(test_log_output);
	return 0;
}

static int teardown(void **state)
{
	(void)state;
	osal_log_deinit();
	return 0;
}

int main(void)
{
	setenv("CMOCKA_TEST_ABORT", "1", 1);

	const struct CMUnitTest tests[] = {
		cmocka_unit_test_setup_teardown(test_log_init_reinit, setup_clean, teardown),
		cmocka_unit_test_setup_teardown(test_log_module_init_dup, setup_ready, teardown),
		cmocka_unit_test_setup_teardown(test_log_module_init_invalid, setup_ready, teardown),
		cmocka_unit_test_setup_teardown(test_log_level_filter, setup_ready, teardown),
		cmocka_unit_test_setup_teardown(test_log_macros, setup_ready, teardown),
		cmocka_unit_test_setup_teardown(test_log_print_unregistered_module, setup_ready, teardown),
		cmocka_unit_test_setup_teardown(test_log_deinit_safety, setup_ready, teardown),
		cmocka_unit_test_setup_teardown(test_log_module_change_invalid, setup_ready, teardown),
		cmocka_unit_test_setup_teardown(test_log_print_invalid_params, setup_ready, teardown),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
