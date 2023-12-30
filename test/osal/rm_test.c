#include "cmocka_include.h"
#include "osal_rm.h"

#define MAX_RES 10

typedef struct {
	osal_resrc_t *resrc;
} rmdata_t;

static void test_rm(bool safe)
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

	if (safe) {
		osal_mutex_init();
	}
	rmcfg.safe = safe;
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
	for (i = 0; i < MAX_RES*10; i++) {
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
		assert_int_equal(avail, MAX_RES-i);

		use = osal_rm_use(&rm);
		assert_int_equal(use, MAX_RES-avail);

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

	osal_rm_deinit(&rm);

	if (safe) {
		osal_mutex_deinit();
	}
}

static void test_rm_run(void **state)
{
	int i;

	for (i = 0; i < 10; i++) {
		test_rm(true);
		test_rm(false);
	}
}

int main(int argc, char **argv)
{
	setenv("CMOCKA_TEST_ABORT", "1", 1);

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_rm_run),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
