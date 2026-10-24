/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

#include "cmocka_include.h"
#include "osal.h"

#define MAX_RES 100

typedef struct {
	osal_resrc_t *resrc;
} rmdata_t;

static void test_rm(bool use_mutex)
{
	osal_rm_t rm;
	rmdata_t rmdatas[MAX_RES];
	osal_resrc_t resrces[MAX_RES];
	osal_rm_cfg_t rmcfg;
	osal_resrc_t *resrc;
	int e;
	int i;
	uint32_t use;
	uint32_t avail;

	memset(&rmcfg, 0, sizeof(rmcfg));
	if (use_mutex) {
		osal_mutex_init();
		rmcfg.mutex = osal_mutex_create();
		assert_non_null(rmcfg.mutex);
	}
	rmcfg.n_resrces = MAX_RES;
	rmcfg.resrces = resrces;
	for (i = 0; i < MAX_RES; i++) {
		resrces[i].data = &rmdatas[i];
	}
	e = osal_rm_init(&rm, &rmcfg);
	assert_int_equal(e, OSAL_E_OK);
	avail = osal_rm_avail(&rm);
	assert_int_equal(avail, MAX_RES);
	use = osal_rm_use(&rm);
	assert_int_equal(use, 0);

	/* alloc then free */
	for (i = 0; i < MAX_RES * 10; i++) {
		avail = osal_rm_avail(&rm);
		assert_int_equal(avail, MAX_RES);

		use = osal_rm_use(&rm);
		assert_int_equal(use, 0);

		resrc = osal_rm_alloc(&rm);
		assert_non_null(resrc);
		rmdata_t *rmd = resrc->data;
		assert_non_null(rmd);

		osal_rm_free(&rm, resrc);
	}

	/* alloc only */
	for (i = 0; i < MAX_RES; i++) {
		avail = osal_rm_avail(&rm);
		assert_int_equal(avail, MAX_RES - i);

		use = osal_rm_use(&rm);
		assert_int_equal(use, MAX_RES - avail);

		resrc = osal_rm_alloc(&rm);
		assert_non_null(resrc);
		rmdata_t *rmd = resrc->data;
		assert_non_null(rmd);
	}
	resrc = osal_rm_alloc(&rm);
	assert_null(resrc);
	avail = osal_rm_avail(&rm);
	assert_int_equal(avail, 0);
	use = osal_rm_use(&rm);
	assert_int_equal(use, MAX_RES);

	/* Tear down the rm before deleting the mutex it references,
	 * osal_rm_deinit locks that mutex to serialize the pool reset. */
	osal_rm_deinit(&rm);
	if (use_mutex) {
		osal_mutex_delete(rmcfg.mutex);
		osal_mutex_deinit();
	}
}

static void test_rm_null_params(void **state)
{
	(void)state;
	osal_rm_t rm;
	osal_rm_cfg_t rmcfg;
	osal_resrc_t resrc;
	osal_error_t err;

	memset(&rmcfg, 0, sizeof(rmcfg));

	err = osal_rm_init(NULL, &rmcfg);
	assert_int_equal(err, OSAL_E_PARAM);

	err = osal_rm_init(&rm, NULL);
	assert_int_equal(err, OSAL_E_PARAM);

	/* NULL-pointer parameter paths for the remaining APIs must be
	 * rejected/no-op safely rather than crashing. */
	assert_null(osal_rm_alloc(NULL));
	assert_int_equal(osal_rm_avail(NULL), 0);

	osal_rm_free(NULL, &resrc);
	osal_rm_free(&rm, NULL);

	osal_rm_deinit(NULL);
}

static void test_rm_run(void **state)
{
	(void)state;
	int i;

	for (i = 0; i < 10; i++) {
		test_rm(true);
		test_rm(false);
	}
}

int main(void)
{
	setenv("CMOCKA_TEST_ABORT", "1", 1);

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_rm_run),
		cmocka_unit_test(test_rm_null_params),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
