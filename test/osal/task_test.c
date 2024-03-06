/* BSD 2-Clause License
*
* Copyright (c) 2023, nguyenvannam142@gmail.com
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
#include "osal_task.h"
#include "osal.h"

static void test_task_init(void **state)
{
	(void)state;
	int res;
	uint32_t use;
	uint32_t avail;

	res = osal_init(NULL);
	assert_int_equal(res, OSAL_E_OK);
	/* reinit should be okay */
	res = osal_task_init(NULL);
	assert_int_equal(res, OSAL_E_OK);
	res = osal_task_init(NULL);
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
	(void)state;
	int res;
	int i;
	osal_task_t *task;
	osal_task_cfg_t cfg = {0};
	uint32_t use;
	uint32_t avail;
	test_task_count = 0;

	res = osal_init(NULL);
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
	(void)state;
	int i;
	uint32_t use;
	uint32_t avail;
	osal_task_t *task;
	osal_task_cfg_t cfg = {
		.task_handler = test_task_handler,
		.task_arg = (void *)&test_task_handler
	};
	test_task_count = 0;

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
	(void)state;
	osal_init(NULL);
	return 0;
}

static int teardown(void **state)
{
	(void)state;
	osal_deinit();
	return 0;
}

int main(void)
{
	setenv("CMOCKA_TEST_ABORT", "1", 1);

	const struct CMUnitTest tests[] = {
		cmocka_unit_test_setup_teardown(test_task_init, setup, teardown),
		cmocka_unit_test_setup_teardown(test_task_create, setup, teardown),
		cmocka_unit_test_setup_teardown(test_task_delete, setup, teardown),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
