/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

#include "cmocka_include.h"
#include "osal.h"

static char g_log_buf[4096];
static int g_log_calls;

static void test_log_output(char *logstr)
{
	size_t used = strlen(g_log_buf);
	g_log_calls++;
	snprintf(g_log_buf + used, sizeof(g_log_buf) - used, "%s", logstr);
}

static void reset_capture(void)
{
	g_log_calls = 0;
	g_log_buf[0] = 0;
}

static void test_osal_version(void **state)
{
	(void)state;
	char *ver;

	ver = osal_version();
	assert_non_null(ver);
	assert_true(strlen(ver) > 0);
}

static void test_osal_init_default_config(void **state)
{
	(void)state;
	osal_error_t res;

	/* NULL config falls back to built-in defaults. */
	res = osal_init(NULL);
	assert_int_equal(res, OSAL_E_OK);

	osal_deinit();
}

static void test_osal_init_custom_config(void **state)
{
	(void)state;
	osal_error_t res;
	osal_config_t cfg;

	memset(&cfg, 0, sizeof(cfg));
	cfg.log_output = test_log_output;
	cfg.osal_level = OSALOG_LEVEL_INFO;

	res = osal_init(&cfg);
	assert_int_equal(res, OSAL_E_OK);

	/* Re-init while already initialized is a no-op: the config argument
	 * is ignored and the subsystem stays usable. */
	res = osal_init(NULL);
	assert_int_equal(res, OSAL_E_OK);

	osal_deinit();

	/* Deinit again must be idempotent / safe. */
	osal_deinit();
}

static void test_osal_init_config_null_log_output(void **state)
{
	(void)state;
	osal_error_t res;
	osal_config_t cfg;

	/* A non-NULL config with a NULL log_output field must still fall
	 * back to the default writer rather than leaving logging broken. */
	memset(&cfg, 0, sizeof(cfg));
	cfg.log_output = NULL;
	cfg.osal_level = OSALOG_LEVEL_INFO;

	res = osal_init(&cfg);
	assert_int_equal(res, OSAL_E_OK);

	osal_deinit();
}

static void test_osal_init_unwinds_on_failure(void **state)
{
	osal_mutex_t *m;

	(void)state;
	osal_deinit();

	/* With the mutex pool exhausted osal_init() cannot create its shared mutex. */
	assert_int_equal(osal_mutex_init(), OSAL_E_OK);
	do {
		m = osal_mutex_create();
	} while (m != NULL);

	assert_int_equal(osal_init(NULL), OSAL_E_RESRC);

	/* The failed init left nothing behind, a new one works. */
	assert_int_equal(osal_init(NULL), OSAL_E_OK);
	osal_deinit();
}

static void test_osal_print_resource(void **state)
{
	(void)state;
	osal_error_t res;
	osal_config_t cfg;

	memset(&cfg, 0, sizeof(cfg));
	cfg.log_output = test_log_output;
	cfg.osal_level = OSALOG_LEVEL_INFO;

	res = osal_init(&cfg);
	assert_int_equal(res, OSAL_E_OK);

	reset_capture();
	osal_print_resource();
	/* One header line + one line per subsystem + one footer line. */
	assert_true(g_log_calls >= 7);
	assert_non_null(strstr(g_log_buf, "tmcheck"));

	osal_deinit();
}

int main(void)
{
	setenv("CMOCKA_TEST_ABORT", "1", 1);

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_osal_version),
		cmocka_unit_test(test_osal_init_default_config),
		cmocka_unit_test(test_osal_init_custom_config),
		cmocka_unit_test(test_osal_init_config_null_log_output),
		cmocka_unit_test(test_osal_init_unwinds_on_failure),
		cmocka_unit_test(test_osal_print_resource),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
