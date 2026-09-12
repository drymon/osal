/* BSD 2-Clause License
*
* Copyright (c) 2025, nguyenvannam142@gmail.com
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

#include <stdint.h>
#include "cmocka_include.h"
#include "osal.h"

static void unique_name(char *out, size_t outsize, const char *tag, int idx)
{
	snprintf(out, outsize, "tmc_%d_%s_%d", (int)getpid(), tag, idx);
}

static char g_log_buf[OSAL_LOG_STRING_SIZE];
static int g_log_calls;

static void capture_log_output(char *logstr)
{
	g_log_calls++;
	snprintf(g_log_buf, sizeof(g_log_buf), "%s", logstr);
}

static void reset_capture(void)
{
	g_log_calls = 0;
	g_log_buf[0] = 0;
}

static void test_tmcheck_init(void **state)
{
	(void)state;
	osal_error_t res;
	uint32_t use;
	uint32_t avail;

	/* reinit while already initialized (via setup's osal_init) is a no-op */
	res = osal_tmcheck_init(NULL);
	assert_int_equal(res, OSAL_E_OK);

	use = osal_tmcheck_use();
	assert_int_equal(use, 0);
	avail = osal_tmcheck_avail();
	assert_int_equal(avail, OSAL_TMCHECK_NUM_MAX);
}

static void test_tmcheck_create_invalid(void **state)
{
	(void)state;
	int idx;

	idx = osal_tmcheck_create(NULL);
	assert_int_equal(idx, -1);
}

static void test_tmcheck_duplicate_name(void **state)
{
	(void)state;
	int idx1;
	int idx2;
	char name[32];

	unique_name(name, sizeof(name), "dup", 0);

	idx1 = osal_tmcheck_create(name);
	assert_true(idx1 >= 0);

	idx2 = osal_tmcheck_create(name);
	assert_int_equal(idx2, -1);

	osal_tmcheck_delete(idx1);
}

static void test_tmcheck_pool_exhaustion(void **state)
{
	(void)state;
	int idxs[OSAL_TMCHECK_NUM_MAX];
	int extra;
	char name[32];
	int i;
	uint32_t use;
	uint32_t avail;

	for (i = 0; i < OSAL_TMCHECK_NUM_MAX; i++) {
		use = osal_tmcheck_use();
		assert_int_equal(use, (uint32_t)i);
		avail = osal_tmcheck_avail();
		assert_int_equal(avail, OSAL_TMCHECK_NUM_MAX - (uint32_t)i);

		unique_name(name, sizeof(name), "pool", i);
		idxs[i] = osal_tmcheck_create(name);
		assert_true(idxs[i] >= 0);
	}

	unique_name(name, sizeof(name), "pool_extra", 0);
	extra = osal_tmcheck_create(name);
	assert_int_equal(extra, -1);

	for (i = 0; i < OSAL_TMCHECK_NUM_MAX; i++) {
		osal_tmcheck_delete(idxs[i]);
	}
}

static void test_tmcheck_capture_first_hit_only(void **state)
{
	(void)state;
	int idx;
	char name[32];
	uint64_t ts1;
	uint64_t ts2;
	uint64_t ts3;

	unique_name(name, sizeof(name), "capture", 0);
	idx = osal_tmcheck_create(name);
	assert_true(idx >= 0);

	osal_tmcheck_capture_ts(idx);
	ts1 = osal_tmcheck_get_captured_ts(idx);
	assert_true(ts1 > 0);

	usleep(20000);
	/* Second capture on the same checkpoint is a no-op until reset. */
	osal_tmcheck_capture_ts(idx);
	ts2 = osal_tmcheck_get_captured_ts(idx);
	assert_int_equal(ts2, ts1);

	osal_tmcheck_reset(idx);
	assert_int_equal(osal_tmcheck_get_captured_ts(idx), 0);

	osal_tmcheck_capture_ts(idx);
	ts3 = osal_tmcheck_get_captured_ts(idx);
	assert_true(ts3 > ts1);

	osal_tmcheck_delete(idx);
}

static void test_tmcheck_double_delete_noop(void **state)
{
	(void)state;
	int idx;
	char name[32];

	unique_name(name, sizeof(name), "double_del", 0);
	idx = osal_tmcheck_create(name);
	assert_true(idx >= 0);
	assert_int_equal(osal_tmcheck_use(), 1u);

	osal_tmcheck_delete(idx);
	assert_int_equal(osal_tmcheck_use(), 0u);

	/* Deleting an already-freed index must not double-decrement the
	 * in-use count (Phase 2 regression: guarded on the `inuse` flag). */
	osal_tmcheck_delete(idx);
	assert_int_equal(osal_tmcheck_use(), 0u);
}

static void test_tmcheck_get_diff_invalid(void **state)
{
	(void)state;
	osal_error_t res;
	int idx;
	char name[32];
	int64_t out_ns;

	unique_name(name, sizeof(name), "diff_inv", 0);
	idx = osal_tmcheck_create(name);
	assert_true(idx >= 0);

	res = osal_tmcheck_get_diff(idx, idx, NULL);
	assert_int_equal(res, OSAL_E_PARAM);

	res = osal_tmcheck_get_diff(-1, idx, &out_ns);
	assert_int_equal(res, OSAL_E_PARAM);

	res = osal_tmcheck_get_diff(idx, OSAL_TMCHECK_NUM_MAX, &out_ns);
	assert_int_equal(res, OSAL_E_PARAM);

	osal_tmcheck_delete(idx);
}

static void test_tmcheck_name_apis(void **state)
{
	(void)state;
	osal_error_t res;
	int idx1;
	int idx2;
	char name1[32];
	char name2[32];
	int64_t out_ns;
	uint64_t ts1;
	uint64_t ts2;

	unique_name(name1, sizeof(name1), "name_a", 0);
	unique_name(name2, sizeof(name2), "name_b", 0);

	idx1 = osal_tmcheck_create(name1);
	assert_true(idx1 >= 0);
	idx2 = osal_tmcheck_create(name2);
	assert_true(idx2 >= 0);

	osal_tmcheck_capture_ts(idx1);
	usleep(20000);
	osal_tmcheck_capture_ts(idx2);

	res = osal_tmcheck_name_get_diff(name1, name2, &out_ns);
	assert_int_equal(res, OSAL_E_OK);
	assert_true(out_ns > 0);

	ts1 = osal_tmcheck_name_get_captured_ts(name1);
	ts2 = osal_tmcheck_name_get_captured_ts(name2);
	assert_true(ts1 > 0);
	assert_true(ts2 > ts1);

	res = osal_tmcheck_name_get_diff(name1, "no-such-checkpoint", &out_ns);
	assert_int_equal(res, OSAL_E_PARAM);

	res = osal_tmcheck_name_get_diff(NULL, name2, &out_ns);
	assert_int_equal(res, OSAL_E_PARAM);

	osal_tmcheck_delete(idx1);
	osal_tmcheck_delete(idx2);
}

static void test_tmcheck_diff_ns_large(void **state)
{
	(void)state;
	osal_error_t res;
	int idx1;
	int idx2;
	char name1[32];
	char name2[32];
	int64_t diff_ns;

	unique_name(name1, sizeof(name1), "large_a", 0);
	unique_name(name2, sizeof(name2), "large_b", 0);

	idx1 = osal_tmcheck_create(name1);
	assert_true(idx1 >= 0);
	idx2 = osal_tmcheck_create(name2);
	assert_true(idx2 >= 0);

	osal_tmcheck_capture_ts(idx1);
	/* Sleep past the ~2.147s boundary that the old 32-bit
	 * osal_tmcheck_get_diff silently wrapped on; this is a regression test
	 * for the int64_t widening fix. */
	osal_usleep(2200000);
	osal_tmcheck_capture_ts(idx2);

	res = osal_tmcheck_get_diff(idx1, idx2, &diff_ns);
	assert_int_equal(res, OSAL_E_OK);
	assert_true(diff_ns > 2000000000LL);
	assert_true(diff_ns < 3000000000LL);

	osal_tmcheck_delete(idx1);
	osal_tmcheck_delete(idx2);
}

#define TMCHECK_MT_TASKS 8

static int g_mt_idx[TMCHECK_MT_TASKS];
static bool g_mt_ok[TMCHECK_MT_TASKS];

static void tmcheck_mt_task(void *arg)
{
	intptr_t i = (intptr_t)arg;
	char name[32];
	uint64_t ts;
	int idx;

	unique_name(name, sizeof(name), "mt", (int)i);
	idx = osal_tmcheck_create(name);
	assert_true(idx >= 0);
	g_mt_idx[i] = idx;
	/* Each task only touches its own checkpoint slot, so this stresses
	 * the create/delete pool locking concurrently across threads without
	 * racing on a single tmcheck_t's fields (which are documented as
	 * caller-serialized / [owner], not internally locked). */
	osal_tmcheck_capture_ts(idx);
	ts = osal_tmcheck_get_captured_ts(idx);
	g_mt_ok[i] = (ts > 0);
}

static void test_tmcheck_multithread_capture(void **state)
{
	(void)state;
	osal_task_t *tasks[TMCHECK_MT_TASKS];
	osal_task_cfg_t cfg;
	int i;

	memset(g_mt_idx, 0xff, sizeof(g_mt_idx));
	memset(g_mt_ok, 0, sizeof(g_mt_ok));

	for (i = 0; i < TMCHECK_MT_TASKS; i++) {
		memset(&cfg, 0, sizeof(cfg));
		cfg.task_handler = tmcheck_mt_task;
		cfg.task_arg = (void *)(intptr_t)i;
		tasks[i] = osal_task_create(&cfg);
		assert_non_null(tasks[i]);
	}
	for (i = 0; i < TMCHECK_MT_TASKS; i++) {
		osal_task_delete(tasks[i]); /* joins the thread */
	}

	for (i = 0; i < TMCHECK_MT_TASKS; i++) {
		assert_true(g_mt_ok[i]);
		assert_true(g_mt_idx[i] >= 0);
		osal_tmcheck_delete(g_mt_idx[i]);
	}
}

static void test_tmcheck_print_diff(void **state)
{
	(void)state;
	int idx1;
	int idx2;
	char name1[32];
	char name2[32];

	unique_name(name1, sizeof(name1), "pdiff_a", 0);
	unique_name(name2, sizeof(name2), "pdiff_b", 0);
	idx1 = osal_tmcheck_create(name1);
	assert_true(idx1 >= 0);
	idx2 = osal_tmcheck_create(name2);
	assert_true(idx2 >= 0);

	osal_tmcheck_capture_ts(idx1);
	usleep(20000);
	osal_tmcheck_capture_ts(idx2);

	reset_capture();
	osal_tmcheck_print_diff(idx1, idx2);
	assert_int_equal(g_log_calls, 1);
	assert_non_null(strstr(g_log_buf, name1));
	assert_non_null(strstr(g_log_buf, name2));

	/* Invalid indices are rejected: validate_idx() itself logs one error
	 * line, but no diff line is emitted for it. */
	reset_capture();
	osal_tmcheck_print_diff(-1, idx2);
	assert_int_equal(g_log_calls, 1);
	assert_null(strstr(g_log_buf, name2));

	osal_tmcheck_delete(idx1);
	osal_tmcheck_delete(idx2);
}

static void test_tmcheck_print_diff_uncaptured(void **state)
{
	(void)state;
	int idx1;
	int idx2;
	char name1[32];
	char name2[32];

	unique_name(name1, sizeof(name1), "pdiff_unc_a", 0);
	unique_name(name2, sizeof(name2), "pdiff_unc_b", 0);
	idx1 = osal_tmcheck_create(name1);
	assert_true(idx1 >= 0);
	idx2 = osal_tmcheck_create(name2);
	assert_true(idx2 >= 0);

	/* Neither checkpoint has been captured (ts == 0): print_diff must be a
	 * silent no-op rather than printing a bogus "0 ns" diff. */
	reset_capture();
	osal_tmcheck_print_diff(idx1, idx2);
	assert_int_equal(g_log_calls, 0);

	osal_tmcheck_delete(idx1);
	osal_tmcheck_delete(idx2);
}

static void test_tmcheck_print_all(void **state)
{
	(void)state;
	int idx;
	char name[32];

	unique_name(name, sizeof(name), "pall", 0);
	idx = osal_tmcheck_create(name);
	assert_true(idx >= 0);

	/* Uncaptured checkpoint (ts == 0) must not be printed. */
	reset_capture();
	osal_tmcheck_print_all();
	assert_int_equal(g_log_calls, 0);

	osal_tmcheck_capture_ts(idx);
	reset_capture();
	osal_tmcheck_print_all();
	assert_true(g_log_calls >= 1);
	assert_non_null(strstr(g_log_buf, name));

	osal_tmcheck_delete(idx);
}

static void test_tmcheck_print_diff_all(void **state)
{
	(void)state;
	int idx1;
	int idx2;
	int idx3;
	char name1[32];
	char name2[32];
	char name3[32];

	unique_name(name1, sizeof(name1), "pdiffall_a", 0);
	unique_name(name2, sizeof(name2), "pdiffall_b", 0);
	unique_name(name3, sizeof(name3), "pdiffall_c", 0);
	idx1 = osal_tmcheck_create(name1);
	assert_true(idx1 >= 0);
	idx2 = osal_tmcheck_create(name2);
	assert_true(idx2 >= 0);
	idx3 = osal_tmcheck_create(name3);
	assert_true(idx3 >= 0);

	osal_tmcheck_capture_ts(idx1);
	usleep(10000);
	osal_tmcheck_capture_ts(idx2);
	usleep(10000);
	osal_tmcheck_capture_ts(idx3);

	/* With 3 captured checkpoints, print_diff_all prints consecutive
	 * diffs plus a final wrap-around (last-to-first) diff: 3 lines total. */
	reset_capture();
	osal_tmcheck_print_diff_all();
	assert_true(g_log_calls >= 3);

	osal_tmcheck_delete(idx1);
	osal_tmcheck_delete(idx2);
	osal_tmcheck_delete(idx3);
}

static void test_tmcheck_reset_all(void **state)
{
	(void)state;
	int idx1;
	int idx2;
	char name1[32];
	char name2[32];

	unique_name(name1, sizeof(name1), "resetall_a", 0);
	unique_name(name2, sizeof(name2), "resetall_b", 0);
	idx1 = osal_tmcheck_create(name1);
	assert_true(idx1 >= 0);
	idx2 = osal_tmcheck_create(name2);
	assert_true(idx2 >= 0);

	osal_tmcheck_capture_ts(idx1);
	osal_tmcheck_capture_ts(idx2);
	assert_true(osal_tmcheck_get_captured_ts(idx1) > 0);
	assert_true(osal_tmcheck_get_captured_ts(idx2) > 0);

	osal_tmcheck_reset_all();
	assert_int_equal(osal_tmcheck_get_captured_ts(idx1), 0);
	assert_int_equal(osal_tmcheck_get_captured_ts(idx2), 0);

	/* Checkpoints remain allocated after reset_all (only ts is cleared). */
	assert_int_equal(osal_tmcheck_use(), 2u);

	osal_tmcheck_delete(idx1);
	osal_tmcheck_delete(idx2);
}

static void test_tmcheck_name_print_diff(void **state)
{
	(void)state;
	int idx1;
	int idx2;
	char name1[32];
	char name2[32];

	unique_name(name1, sizeof(name1), "npdiff_a", 0);
	unique_name(name2, sizeof(name2), "npdiff_b", 0);
	idx1 = osal_tmcheck_create(name1);
	assert_true(idx1 >= 0);
	idx2 = osal_tmcheck_create(name2);
	assert_true(idx2 >= 0);

	osal_tmcheck_capture_ts(idx1);
	usleep(20000);
	osal_tmcheck_capture_ts(idx2);

	reset_capture();
	osal_tmcheck_name_print_diff(name1, name2);
	assert_int_equal(g_log_calls, 1);
	assert_non_null(strstr(g_log_buf, name1));
	assert_non_null(strstr(g_log_buf, name2));

	/* An unknown checkpoint name must log an error, not a diff line, and
	 * must not crash on a NULL name. */
	reset_capture();
	osal_tmcheck_name_print_diff(name1, "no-such-checkpoint");
	assert_int_equal(g_log_calls, 1);
	assert_null(strstr(g_log_buf, name2));

	osal_tmcheck_delete(idx1);
	osal_tmcheck_delete(idx2);
}

static void test_tmcheck_get_captured_ts_invalid(void **state)
{
	(void)state;

	assert_int_equal(osal_tmcheck_get_captured_ts(-1), 0);
	assert_int_equal(osal_tmcheck_get_captured_ts(OSAL_TMCHECK_NUM_MAX), 0);
}

static void test_tmcheck_name_get_captured_ts_invalid(void **state)
{
	(void)state;
	uint64_t ts;

	reset_capture();
	ts = osal_tmcheck_name_get_captured_ts("no-such-checkpoint");
	assert_int_equal(ts, 0);
	assert_int_equal(g_log_calls, 1);
}

static int setup(void **state)
{
	osal_config_t cfg;

	(void)state;
	memset(&cfg, 0, sizeof(cfg));
	cfg.log_output = capture_log_output;
	cfg.osal_level = OSALOG_LEVEL_INFO;
	osal_init(&cfg);
	reset_capture();
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
		cmocka_unit_test_setup_teardown(test_tmcheck_init, setup, teardown),
		cmocka_unit_test_setup_teardown(test_tmcheck_create_invalid, setup, teardown),
		cmocka_unit_test_setup_teardown(test_tmcheck_duplicate_name, setup, teardown),
		cmocka_unit_test_setup_teardown(test_tmcheck_pool_exhaustion, setup, teardown),
		cmocka_unit_test_setup_teardown(test_tmcheck_capture_first_hit_only, setup, teardown),
		cmocka_unit_test_setup_teardown(test_tmcheck_double_delete_noop, setup, teardown),
		cmocka_unit_test_setup_teardown(test_tmcheck_get_diff_invalid, setup, teardown),
		cmocka_unit_test_setup_teardown(test_tmcheck_name_apis, setup, teardown),
		cmocka_unit_test_setup_teardown(test_tmcheck_diff_ns_large, setup, teardown),
		cmocka_unit_test_setup_teardown(test_tmcheck_multithread_capture, setup, teardown),
		cmocka_unit_test_setup_teardown(test_tmcheck_print_diff, setup, teardown),
		cmocka_unit_test_setup_teardown(test_tmcheck_print_diff_uncaptured, setup, teardown),
		cmocka_unit_test_setup_teardown(test_tmcheck_print_all, setup, teardown),
		cmocka_unit_test_setup_teardown(test_tmcheck_print_diff_all, setup, teardown),
		cmocka_unit_test_setup_teardown(test_tmcheck_reset_all, setup, teardown),
		cmocka_unit_test_setup_teardown(test_tmcheck_name_print_diff, setup, teardown),
		cmocka_unit_test_setup_teardown(test_tmcheck_get_captured_ts_invalid, setup, teardown),
		cmocka_unit_test_setup_teardown(test_tmcheck_name_get_captured_ts_invalid, setup, teardown),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
