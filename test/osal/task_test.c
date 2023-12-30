#include "cmocka_include.h"
#include "osal_task.h"
#include "osal.h"

static void test_task_init(void **state)
{
	int res;
	uint32_t use;
	uint32_t avail;

	res = osal_init();
	assert_int_equal(res, OSAL_E_OK);
	res = osal_task_init();
	assert_int_equal(res, OSAL_E_OK);
	res = osal_task_init();
	assert_int_equal(res, OSAL_E_OK);
	use = osal_task_use();
	assert_int_equal(use, 0);
	avail = osal_task_avail();
	assert_int_equal(avail, OSAL_TASK_NUM_MAX);
}

static int test_task_count;

static void test_task_handler(void *arg)
{
	assert_ptr_equal(arg, (void*)&test_task_handler);
	test_task_count++;
}

static void test_task_create(void **state)
{
	int res;
	int i;
	osal_task_t *task;
	osal_task_cfg_t cfg = {0};
	uint32_t use;
	uint32_t avail;

	res = osal_init();
	assert_int_equal(res, OSAL_E_OK);

	task = osal_task_create(NULL);
	assert_null(task);

	task = osal_task_create(&cfg);
	assert_null(task);

	cfg.task_handler = test_task_handler;
	cfg.task_arg = (void *)&test_task_handler;

	for (i = 0; i < OSAL_TASK_NUM_MAX; i++) {
		use = osal_task_use();
		assert_int_equal(use, i);
		avail = osal_task_avail();
		assert_int_equal(avail, OSAL_TASK_NUM_MAX-i);

		task = osal_task_create(&cfg);
		assert_non_null(task);
		usleep(1000);
	}
	task = osal_task_create(&cfg);
	assert_null(task);
	assert_int_equal(test_task_count, OSAL_TASK_NUM_MAX);
}

static void test_task_delete(void **state)
{
	int res;
	int i;
	uint32_t use;
	uint32_t avail;
	osal_task_t *task;
	osal_task_cfg_t cfg = {
		.task_handler = test_task_handler,
		.task_arg = (void *)&test_task_handler
	};

	for (i = 0; i < OSAL_TASK_NUM_MAX*10; i++) {
		use = osal_task_use();
		assert_int_equal(use, 0);
		avail = osal_task_avail();
		assert_int_equal(avail, OSAL_TASK_NUM_MAX);

		task = osal_task_create(&cfg);
		assert_non_null(task);
		usleep(1000);
		osal_task_delete(task);
	}
	task = osal_task_create(&cfg);
	assert_non_null(task);
}

static int setup(void **state)
{
	osal_init();
	return 0;
}

static int teardown(void **state)
{
	osal_deinit();
	return 0;
}

int main(int argc, char **argv)
{
	setenv("CMOCKA_TEST_ABORT", "1", 1);

	const struct CMUnitTest tests[] = {
		cmocka_unit_test_setup_teardown(test_task_init, setup, teardown),
		cmocka_unit_test_setup_teardown(test_task_create, setup, teardown),
		cmocka_unit_test_setup_teardown(test_task_delete, setup, teardown),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
