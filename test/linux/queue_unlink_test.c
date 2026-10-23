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

/* Linux-only: the Linux queue backend is backed by a POSIX message queue, visible in
 * /dev/mqueue. Other backends have no such kernel object. */
#include <errno.h>
#include "cmocka_include.h"
#include "osal.h"

static void test_queue_delete_unlinks(void **state)
{
	(void)state;
	osal_queue_cfg_t cfg;
	osal_queue_t *queue;
	char path[256];

	memset(&cfg, 0, sizeof(cfg));
	snprintf((char *)cfg.name, sizeof(cfg.name), "dmosal_qu_%d", (int)getpid());
	cfg.msglen = 8;
	cfg.qsize = 2;

	queue = osal_queue_create(&cfg);
	assert_non_null(queue);

	snprintf(path, sizeof(path), "/dev/mqueue/%s", (char *)cfg.name);
	assert_int_equal(access(path, F_OK), 0);

	osal_queue_delete(queue);

	assert_int_not_equal(access(path, F_OK), 0);
	assert_int_equal(errno, ENOENT);
}

static int setup(void **state)
{
	osal_config_t cfg;

	(void)state;
	memset(&cfg, 0, sizeof(cfg));
	osal_init(&cfg);
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
		cmocka_unit_test_setup_teardown(test_queue_delete_unlinks, setup, teardown),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
