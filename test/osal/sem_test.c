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
#include "osal.h"
#include "osal_sem.h"
#include "osal_time.h"

static void test_sem_loop(void)
{
	int res;
	int i;
	uint32_t use;
	uint32_t avail;
	osal_sem_t *sem;
	uint64_t ts1, ts2, diff, wait_nsec;

	/* check if we can create sem if it is deinitialized */
	osal_sem_deinit();
	sem = osal_sem_create();
	assert_null(sem);

	res = osal_sem_init(NULL);
	assert_int_equal(res, OSAL_E_OK);

	use = osal_sem_use();
	assert_int_equal(use, 0);

	avail = osal_sem_avail();
	assert_int_equal(avail, OSAL_SEM_NUM_MAX);

	/* only create, not delete */
	for (i = 0; i < OSAL_SEM_NUM_MAX; i++) {
		use = osal_sem_use();
		assert_int_equal(use, i);

		avail = osal_sem_avail();
		assert_int_equal(avail, OSAL_SEM_NUM_MAX-i);

		sem = osal_sem_create();
		assert_non_null(sem);

		res = osal_sem_post(sem);
		assert_int_equal(res, OSAL_E_OK);

		osal_sem_wait(sem);
	}
	/* no more sem */
	sem = osal_sem_create();
	assert_null(sem);

	osal_sem_deinit();
	res = osal_sem_init(NULL);
	assert_int_equal(res, OSAL_E_OK);

	/* create and then delete */
	for (i = 0; i < OSAL_SEM_NUM_MAX; i++) {
		use = osal_sem_use();
		assert_int_equal(use, 0);

		avail = osal_sem_avail();
		assert_int_equal(avail, OSAL_SEM_NUM_MAX);

		sem = osal_sem_create();
		assert_non_null(sem);

		res = osal_sem_post(sem);
		assert_int_equal(res, OSAL_E_OK);

		osal_sem_wait(sem);

		osal_sem_delete(sem);
	}
	sem = osal_sem_create();
	assert_non_null(sem);

	wait_nsec = OSAL_MSEC_NSEC;
	for (i = 0; i < 10; i++) {
		res = osal_clock_time(&ts1);
		assert_int_equal(res, OSAL_E_OK);
		wait_nsec += i*100000;
		res = osal_sem_waittime(sem, wait_nsec/OSAL_USEC_NSEC);
		assert_int_equal(res, OSAL_E_TIMEOUT);

		res = osal_clock_time(&ts2);
		assert_int_equal(res, OSAL_E_OK);

		diff = ts2-ts1;
		assert_in_range(diff, wait_nsec, wait_nsec+OSAL_USEC_NSEC*300);
	}

	osal_sem_deinit();
}

static void test_sem(void **state)
{
	(void)state;
	int i;
	for (i = 0; i < 10; i++) {
		test_sem_loop();
	}
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
		cmocka_unit_test_setup_teardown(test_sem, setup, teardown),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
