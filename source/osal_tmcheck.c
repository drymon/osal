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
#include <inttypes.h>
#include "osal.h"
#define OSALOG_MODULE OSAL_LOG_MODULE_INDEX

#define TMCHECK_LOCK()                                                                             \
	if (s_tmcheck_man.mutex != NULL) {                                                             \
		osal_mutex_lock(s_tmcheck_man.mutex);                                                      \
	}
#define TMCHECK_UNLOCK()                                                                           \
	if (s_tmcheck_man.mutex != NULL) {                                                             \
		osal_mutex_unlock(s_tmcheck_man.mutex);                                                    \
	}

typedef struct {
	uint64_t ts;
	uint8_t inuse;
	char name[64];
} tmcheck_t;

typedef struct {
	uint8_t init;
	uint8_t n_tmchecks;
	osal_mutex_t *mutex;
	tmcheck_t tmchecks[OSAL_TMCHECK_NUM_MAX];
} tmcheck_man_t;

static tmcheck_man_t s_tmcheck_man;

osal_error_t osal_tmcheck_init(osal_mutex_t *mutex)
{
	if (s_tmcheck_man.init == 1) {
		return OSAL_E_OK;
	}
	memset(&s_tmcheck_man, 0, sizeof(s_tmcheck_man));
	s_tmcheck_man.mutex = mutex;
	s_tmcheck_man.init = 1;
	return OSAL_E_OK;
}

void osal_tmcheck_deinit(void)
{
	memset(&s_tmcheck_man, 0, sizeof(s_tmcheck_man));
}

int osal_tmcheck_create(const char *name)
{
	int new_idx = -1;
	tmcheck_t *tmcheck;
	char *errstr = NULL;
	int i;

	if (!name) {
		OSALOG_ERROR("Invalid param\n");
		return -1;
	}

	TMCHECK_LOCK();

	/* Step1: Check if the same name and ID are already there */
	for (i = 0; i < OSAL_TMCHECK_NUM_MAX; i++) {
		tmcheck = &s_tmcheck_man.tmchecks[i];
		if (strcmp(name, tmcheck->name) == 0) {
			errstr = "duplicated name";
			break;
		}
	}
	if (errstr) {
		goto end;
	}

	/* Step2: find an available slot */
	for (i = 0; i < OSAL_TMCHECK_NUM_MAX; i++) {
		tmcheck = &s_tmcheck_man.tmchecks[i];
		if (tmcheck->inuse == 0) {
			new_idx = i;
			break;
		}
	}
	if (new_idx >= 0) {
		tmcheck = &s_tmcheck_man.tmchecks[new_idx];
		strncpy(tmcheck->name, name, sizeof(tmcheck->name) - 1);
		tmcheck->inuse = 1;
		s_tmcheck_man.n_tmchecks++;
	}

end:
	TMCHECK_UNLOCK();

	if (errstr) {
		OSALOG_ERROR("%s\n", errstr);
	}
	return new_idx;
}

static bool validate_idx(int idx)
{
	if ((idx < 0) || (idx >= OSAL_TMCHECK_NUM_MAX)) {
		OSALOG_ERROR("Invalid checkpoint index %d\n", idx);
		return false;
	}
	return true;
}

void osal_tmcheck_delete(int idx)
{
	if (validate_idx(idx) == false) {
		return;
	}
	TMCHECK_LOCK();
	if (s_tmcheck_man.tmchecks[idx].inuse != 0) {
		memset(&s_tmcheck_man.tmchecks[idx], 0, sizeof(tmcheck_t));
		s_tmcheck_man.n_tmchecks--;
	}
	TMCHECK_UNLOCK();
}

uint32_t osal_tmcheck_use(void)
{
	uint32_t use;

	TMCHECK_LOCK();
	use = s_tmcheck_man.n_tmchecks;
	TMCHECK_UNLOCK();
	return use;
}

uint32_t osal_tmcheck_avail(void)
{
	uint32_t use = osal_tmcheck_use();
	return OSAL_TMCHECK_NUM_MAX - use;
}

void osal_tmcheck_capture_ts(int idx)
{
	if (validate_idx(idx) == false) {
		return;
	}
	if (s_tmcheck_man.tmchecks[idx].ts == 0) {
		osal_clock_time(&s_tmcheck_man.tmchecks[idx].ts);
	}
}

/*
 * Cold-path functions (print and name lookup) take the lock because they read
 * names / the inuse flag that create and delete modify. capture_ts, get_captured_ts,
 * reset, reset_all and get_diff are the hot path and stay lock-free: the caller serializes
 * access to a given checkpoint.
 */
static void snapshot_all(tmcheck_t *dst)
{
	TMCHECK_LOCK();
	memcpy(dst, s_tmcheck_man.tmchecks, sizeof(s_tmcheck_man.tmchecks));
	TMCHECK_UNLOCK();
}

static bool snapshot_idx(int idx, tmcheck_t *dst)
{
	if (validate_idx(idx) == false) {
		return false;
	}
	TMCHECK_LOCK();
	*dst = s_tmcheck_man.tmchecks[idx];
	TMCHECK_UNLOCK();
	return true;
}

/* Look up two names under the lock; returns true when both are found. */
static bool snapshot_names(const char *name1, const char *name2, tmcheck_t *dst1, tmcheck_t *dst2)
{
	bool found1 = false;
	bool found2 = false;
	int i;

	TMCHECK_LOCK();
	for (i = 0; i < OSAL_TMCHECK_NUM_MAX; i++) {
		const tmcheck_t *entry = &s_tmcheck_man.tmchecks[i];

		if (entry->inuse == 0) {
			continue;
		}
		if (!found1 && strcmp(entry->name, name1) == 0) {
			*dst1 = *entry;
			found1 = true;
		} else if (!found2 && strcmp(entry->name, name2) == 0) {
			*dst2 = *entry;
			found2 = true;
		}
		if (found1 && found2) {
			break;
		}
	}
	TMCHECK_UNLOCK();
	return found1 && found2;
}

void osal_tmcheck_print_all(void)
{
	tmcheck_t snap[OSAL_TMCHECK_NUM_MAX];
	int i;

	snapshot_all(snap);
	for (i = 0; i < OSAL_TMCHECK_NUM_MAX; i++) {
		if (snap[i].ts > 0) {
			OSALOG_INFO("[%s]=%" PRIu64 " ns\n", snap[i].name, snap[i].ts);
		}
	}
}

static void tmcheck_print_diff(const tmcheck_t *tmcheck1, const tmcheck_t *tmcheck2)
{
	if ((tmcheck1->ts > 0) && (tmcheck2->ts > 0)) {
		if (tmcheck2->ts > tmcheck1->ts) {
			OSALOG_INFO("[%s]-[%s]=%" PRId64 " ns\n",
						tmcheck2->name,
						tmcheck1->name,
						(int64_t)(tmcheck2->ts - tmcheck1->ts));
		} else {
			OSALOG_INFO("[%s]-[%s]=%" PRId64 " ns\n",
						tmcheck1->name,
						tmcheck2->name,
						(int64_t)(tmcheck1->ts - tmcheck2->ts));
		}
	}
}

void osal_tmcheck_print_diff(int idx1, int idx2)
{
	tmcheck_t snap1;
	tmcheck_t snap2;

	if (!snapshot_idx(idx1, &snap1) || !snapshot_idx(idx2, &snap2)) {
		return;
	}
	tmcheck_print_diff(&snap1, &snap2);
}

static int tmcheck_cmp(const void *a, const void *b)
{
	const tmcheck_t *tmcheck1 = (const tmcheck_t *)a;
	const tmcheck_t *tmcheck2 = (const tmcheck_t *)b;

	if (tmcheck1->ts < tmcheck2->ts) {
		return -1;
	}
	if (tmcheck1->ts > tmcheck2->ts) {
		return 1;
	}
	return 0;
}

void osal_tmcheck_print_diff_all(void)
{
	tmcheck_t sort_tmchecks[OSAL_TMCHECK_NUM_MAX];
	tmcheck_t *prev_tmcheck = NULL;
	tmcheck_t *first_tmcheck = NULL;
	int count = 0;
	int i;

	snapshot_all(sort_tmchecks);
	qsort(sort_tmchecks, OSAL_TMCHECK_NUM_MAX, sizeof(tmcheck_t), tmcheck_cmp);

	for (i = 0; i < OSAL_TMCHECK_NUM_MAX; i++) {
		if (sort_tmchecks[i].ts == 0) {
			continue;
		}
		if (first_tmcheck == NULL) {
			first_tmcheck = &sort_tmchecks[i];
		}
		if (prev_tmcheck != NULL) {
			tmcheck_print_diff(&sort_tmchecks[i], prev_tmcheck);
		}
		prev_tmcheck = &sort_tmchecks[i];
		count++;
	}
	if ((count > 2) && (first_tmcheck != NULL) && (prev_tmcheck != NULL)) {
		tmcheck_print_diff(prev_tmcheck, first_tmcheck);
	}
}

void osal_tmcheck_reset(int idx)
{
	if (validate_idx(idx) == false) {
		return;
	}
	s_tmcheck_man.tmchecks[idx].ts = 0;
}

void osal_tmcheck_reset_all(void)
{
	int i;

	for (i = 0; i < OSAL_TMCHECK_NUM_MAX; i++) {
		s_tmcheck_man.tmchecks[i].ts = 0;
	}
}

osal_error_t osal_tmcheck_get_diff(int idx1, int idx2, int64_t *out_ns)
{
	if (out_ns == NULL) {
		return OSAL_E_PARAM;
	}
	if ((validate_idx(idx1) == false) || (validate_idx(idx2) == false)) {
		return OSAL_E_PARAM;
	}
	*out_ns = (int64_t)s_tmcheck_man.tmchecks[idx2].ts - (int64_t)s_tmcheck_man.tmchecks[idx1].ts;
	return OSAL_E_OK;
}

void osal_tmcheck_name_print_diff(const char *name1, const char *name2)
{
	tmcheck_t snap1;
	tmcheck_t snap2;

	if ((name1 == NULL) || (name2 == NULL) || !snapshot_names(name1, name2, &snap1, &snap2)) {
		OSALOG_ERROR("Invalid checkpoint name %s or %s\n", name1, name2);
		return;
	}
	/* NOLINTNEXTLINE(readability-suspicious-call-argument): intentional order */
	tmcheck_print_diff(&snap2, &snap1);
}

osal_error_t osal_tmcheck_name_get_diff(const char *name1, const char *name2, int64_t *out_ns)
{
	tmcheck_t snap1;
	tmcheck_t snap2;

	if ((name1 == NULL) || (name2 == NULL) || (out_ns == NULL)) {
		return OSAL_E_PARAM;
	}
	if (!snapshot_names(name1, name2, &snap1, &snap2)) {
		OSALOG_ERROR("Invalid checkpoint name %s or %s\n", name1, name2);
		return OSAL_E_PARAM;
	}
	*out_ns = (int64_t)snap2.ts - (int64_t)snap1.ts;
	return OSAL_E_OK;
}

uint64_t osal_tmcheck_get_captured_ts(int idx)
{
	if (validate_idx(idx) == false) {
		return 0;
	}
	return s_tmcheck_man.tmchecks[idx].ts;
}

uint64_t osal_tmcheck_name_get_captured_ts(const char *name)
{
	int i;
	uint64_t ts = 0;
	bool found = false;

	if (name == NULL) {
		return 0;
	}
	TMCHECK_LOCK();
	for (i = 0; i < OSAL_TMCHECK_NUM_MAX; i++) {
		if ((s_tmcheck_man.tmchecks[i].inuse != 0) &&
			(strcmp(s_tmcheck_man.tmchecks[i].name, name) == 0)) {
			ts = s_tmcheck_man.tmchecks[i].ts;
			found = true;
			break;
		}
	}
	TMCHECK_UNLOCK();
	if (!found) {
		OSALOG_ERROR("Invalid checkpoint name %s\n", name);
	}
	return ts;
}
