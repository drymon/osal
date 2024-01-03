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
#include "osal_mutex.h"

static void test_mutex_loop(void)
{
	int res;
	int i;
	uint32_t use;
	uint32_t avail;

	osal_mutex_t *mutex = osal_mutex_create();
	assert_null(mutex);

	res = osal_mutex_init();
	assert_int_equal(res, OSAL_E_OK);

	use = osal_mutex_use();
	assert_int_equal(use, 0);

	avail = osal_mutex_avail();
	assert_int_equal(avail, OSAL_MUTEX_NUM_MAX);

	/* only create, not delete */
	for (i = 0; i < OSAL_MUTEX_NUM_MAX; i++) {
		use = osal_mutex_use();
		assert_int_equal(use, i);

		avail = osal_mutex_avail();
		assert_int_equal(avail, OSAL_MUTEX_NUM_MAX-i);

		mutex = osal_mutex_create();
		assert_non_null(mutex);

		res = osal_mutex_lock(mutex);
		assert_int_equal(res, OSAL_E_OK);

		osal_mutex_unlock(mutex);
	}
	/* no more mutex */
	mutex = osal_mutex_create();
	assert_null(mutex);

	osal_mutex_deinit();
	res = osal_mutex_init();
	assert_int_equal(res, OSAL_E_OK);

	/* create and then delete */
	for (i = 0; i < OSAL_MUTEX_NUM_MAX; i++) {
		use = osal_mutex_use();
		assert_int_equal(use, 0);

		avail = osal_mutex_avail();
		assert_int_equal(avail, OSAL_MUTEX_NUM_MAX);

		mutex = osal_mutex_create();
		assert_non_null(mutex);

		res = osal_mutex_lock(mutex);
		assert_int_equal(res, OSAL_E_OK);

		osal_mutex_unlock(mutex);

		osal_mutex_delete(mutex);
	}
	mutex = osal_mutex_create();
	assert_non_null(mutex);

	osal_mutex_deinit();
}

static void test_mutex(void **state)
{
	int i;
	for (i = 0; i < 10; i++) {
		test_mutex_loop();
	}
}

int main(int argc, char **argv)
{
	setenv("CMOCKA_TEST_ABORT", "1", 1);

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_mutex),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
