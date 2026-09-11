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

#include <errno.h>
#include "cmocka_include.h"
#include "osal.h"

static void unique_name(char *out, size_t outsize, const char *tag, int idx)
{
	snprintf(out, outsize, "dmosal_qt_%d_%s_%d", (int)getpid(), tag, idx);
}

static void test_queue_init(void **state)
{
	(void)state;
	osal_error_t res;
	uint32_t use;
	uint32_t avail;

	/* reinit while already initialized (via setup's osal_init) is a no-op */
	res = osal_queue_init(NULL);
	assert_int_equal(res, OSAL_E_OK);

	use = osal_queue_use();
	assert_int_equal(use, 0);
	avail = osal_queue_avail();
	assert_int_equal(avail, OSAL_QUEUE_NUM_MAX);
}

static void test_queue_create_invalid(void **state)
{
	(void)state;
	osal_queue_t *queue;
	osal_queue_cfg_t cfg;

	queue = osal_queue_create(NULL);
	assert_null(queue);

	memset(&cfg, 0, sizeof(cfg));
	queue = osal_queue_create(&cfg); /* empty name */
	assert_null(queue);

	unique_name((char *)cfg.name, sizeof(cfg.name), "inv", 0);
	cfg.msglen = 0;
	cfg.qsize = 4;
	queue = osal_queue_create(&cfg);
	assert_null(queue);

	cfg.msglen = 8;
	cfg.qsize = 0;
	queue = osal_queue_create(&cfg);
	assert_null(queue);
}

static void test_queue_send_recv(void **state)
{
	(void)state;
	osal_queue_cfg_t cfg;
	osal_queue_t *queue;
	osal_error_t res;
	uint8_t msg[16] = "hello dmosal";
	uint8_t buf[16];

	memset(&cfg, 0, sizeof(cfg));
	unique_name((char *)cfg.name, sizeof(cfg.name), "sr", 0);
	cfg.msglen = sizeof(buf);
	cfg.qsize = 4;

	queue = osal_queue_create(&cfg);
	assert_non_null(queue);

	res = osal_queue_send(queue, msg, (uint32_t)strlen((char *)msg) + 1);
	assert_int_equal(res, OSAL_E_OK);

	memset(buf, 0, sizeof(buf));
	res = osal_queue_recv(queue, buf, sizeof(buf), 200000);
	assert_int_equal(res, OSAL_E_OK);
	assert_string_equal((char *)buf, "hello dmosal");

	osal_queue_delete(queue);
}

static void test_queue_full(void **state)
{
	(void)state;
	osal_queue_cfg_t cfg;
	osal_queue_t *queue;
	osal_error_t res;
	uint8_t msg[4] = "abc";

	memset(&cfg, 0, sizeof(cfg));
	unique_name((char *)cfg.name, sizeof(cfg.name), "full", 0);
	cfg.msglen = sizeof(msg);
	cfg.qsize = 1;

	queue = osal_queue_create(&cfg);
	assert_non_null(queue);

	res = osal_queue_send(queue, msg, sizeof(msg));
	assert_int_equal(res, OSAL_E_OK);

	res = osal_queue_send(queue, msg, sizeof(msg));
	assert_int_equal(res, OSAL_E_QFULL);

	osal_queue_delete(queue);
}

static void test_queue_recv_timeout(void **state)
{
	(void)state;
	osal_queue_cfg_t cfg;
	osal_queue_t *queue;
	osal_error_t res;
	uint8_t buf[4];
	uint64_t ts1, ts2, diff_ns;
	uint32_t timeout_usec = 50000; /* 50ms */

	memset(&cfg, 0, sizeof(cfg));
	unique_name((char *)cfg.name, sizeof(cfg.name), "tout", 0);
	cfg.msglen = sizeof(buf);
	cfg.qsize = 1;

	queue = osal_queue_create(&cfg);
	assert_non_null(queue);

	res = osal_clock_time(&ts1);
	assert_int_equal(res, OSAL_E_OK);

	res = osal_queue_recv(queue, buf, sizeof(buf), timeout_usec);
	assert_int_equal(res, OSAL_E_TIMEOUT);

	res = osal_clock_time(&ts2);
	assert_int_equal(res, OSAL_E_OK);

	diff_ns = ts2 - ts1;
	assert_true(diff_ns >= (uint64_t)timeout_usec * OSAL_USEC_NSEC);

	osal_queue_delete(queue);
}

static void test_queue_reopen_match(void **state)
{
	(void)state;
	osal_queue_cfg_t cfg;
	osal_queue_t *creator;
	osal_queue_t *attacher;
	osal_error_t res;
	uint8_t msg[4] = "ok";
	uint8_t buf[4];

	memset(&cfg, 0, sizeof(cfg));
	unique_name((char *)cfg.name, sizeof(cfg.name), "reopen_ok", 0);
	cfg.msglen = sizeof(buf);
	cfg.qsize = 2;

	creator = osal_queue_create(&cfg);
	assert_non_null(creator);

	/* Same name + matching config attaches to the existing kernel queue. */
	attacher = osal_queue_create(&cfg);
	assert_non_null(attacher);

	res = osal_queue_send(creator, msg, sizeof(msg));
	assert_int_equal(res, OSAL_E_OK);

	memset(buf, 0, sizeof(buf));
	res = osal_queue_recv(attacher, buf, sizeof(buf), 200000);
	assert_int_equal(res, OSAL_E_OK);
	assert_string_equal((char *)buf, "ok");

	osal_queue_delete(attacher);
	osal_queue_delete(creator);
}

static void test_queue_reopen_mismatch(void **state)
{
	(void)state;
	osal_queue_cfg_t cfg;
	osal_queue_t *creator;
	osal_queue_t *bad;

	memset(&cfg, 0, sizeof(cfg));
	unique_name((char *)cfg.name, sizeof(cfg.name), "reopen_bad", 0);
	cfg.msglen = 8;
	cfg.qsize = 2;

	creator = osal_queue_create(&cfg);
	assert_non_null(creator);

	/* Same name but a different msglen must be refused. */
	cfg.msglen = 16;
	bad = osal_queue_create(&cfg);
	assert_null(bad);

	/* Same name but a different qsize must also be refused. */
	cfg.msglen = 8;
	cfg.qsize = 4;
	bad = osal_queue_create(&cfg);
	assert_null(bad);

	osal_queue_delete(creator);
}

static void test_queue_delete_unlinks(void **state)
{
	(void)state;
	osal_queue_cfg_t cfg;
	osal_queue_t *queue;
	char path[256];

	memset(&cfg, 0, sizeof(cfg));
	unique_name((char *)cfg.name, sizeof(cfg.name), "unlink", 0);
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

static void test_queue_pool_exhaustion(void **state)
{
	(void)state;
	osal_queue_t *queues[OSAL_QUEUE_NUM_MAX];
	osal_queue_t *extra;
	osal_queue_cfg_t cfg;
	int i;
	uint32_t use;
	uint32_t avail;

	for (i = 0; i < OSAL_QUEUE_NUM_MAX; i++) {
		use = osal_queue_use();
		assert_int_equal(use, (uint32_t)i);
		avail = osal_queue_avail();
		assert_int_equal(avail, OSAL_QUEUE_NUM_MAX - (uint32_t)i);

		memset(&cfg, 0, sizeof(cfg));
		unique_name((char *)cfg.name, sizeof(cfg.name), "pool", i);
		cfg.msglen = 8;
		cfg.qsize = 1;

		queues[i] = osal_queue_create(&cfg);
		assert_non_null(queues[i]);
	}

	memset(&cfg, 0, sizeof(cfg));
	unique_name((char *)cfg.name, sizeof(cfg.name), "pool_extra", 0);
	cfg.msglen = 8;
	cfg.qsize = 1;
	extra = osal_queue_create(&cfg);
	assert_null(extra);

	for (i = 0; i < OSAL_QUEUE_NUM_MAX; i++) {
		osal_queue_delete(queues[i]);
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
		cmocka_unit_test_setup_teardown(test_queue_init, setup, teardown),
		cmocka_unit_test_setup_teardown(test_queue_create_invalid, setup, teardown),
		cmocka_unit_test_setup_teardown(test_queue_send_recv, setup, teardown),
		cmocka_unit_test_setup_teardown(test_queue_full, setup, teardown),
		cmocka_unit_test_setup_teardown(test_queue_recv_timeout, setup, teardown),
		cmocka_unit_test_setup_teardown(test_queue_reopen_match, setup, teardown),
		cmocka_unit_test_setup_teardown(test_queue_reopen_mismatch, setup, teardown),
		cmocka_unit_test_setup_teardown(test_queue_delete_unlinks, setup, teardown),
		cmocka_unit_test_setup_teardown(test_queue_pool_exhaustion, setup, teardown),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
