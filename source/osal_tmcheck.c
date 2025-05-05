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

#define TMCHECK_LOCK() \
	if (s_tmcheck_man.mutex != NULL) { \
		osal_mutex_lock(s_tmcheck_man.mutex); \
	}
#define TMCHECK_UNLOCK() \
	if (s_tmcheck_man.mutex != NULL) { \
		osal_mutex_unlock(s_tmcheck_man.mutex); \
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

int osal_tmcheck_create(char *name)
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
	memset(&s_tmcheck_man.tmchecks[idx], 0, sizeof(tmcheck_t));
	s_tmcheck_man.tmchecks[idx].inuse = 0;
	s_tmcheck_man.n_tmchecks--;
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

void osal_tmcheck_print_all(void)
{
	int i;
	tmcheck_t *tmcheck = NULL;

	for (i = 0; i < OSAL_TMCHECK_NUM_MAX; i++) {
		tmcheck = &s_tmcheck_man.tmchecks[i];
		if (tmcheck->ts > 0) {
			OSALOG_INFO("[%s]=%"PRIu64" ns\n", tmcheck->name, tmcheck->ts);
		}
	}
}

static void tmcheck_print_diff(tmcheck_t *tmcheck1, tmcheck_t *tmcheck2)
{
	if ((tmcheck1->ts > 0) && (tmcheck2->ts > 0)) {
		if (tmcheck2->ts > tmcheck1->ts) {
			OSALOG_INFO("[%s]-[%s]=%d ns\n",
						tmcheck2->name, tmcheck1->name,
						(int)(tmcheck2->ts - tmcheck1->ts));
		} else {
			OSALOG_INFO("[%s]-[%s]=%d ns\n",
						tmcheck1->name, tmcheck2->name,
						(int)(tmcheck1->ts - tmcheck2->ts));
		}
	}
}

void osal_tmcheck_print_diff(int idx1, int idx2)
{
	if ((validate_idx(idx1) == false) || (validate_idx(idx2) == false)) {
		return;
	}
	tmcheck_print_diff(&s_tmcheck_man.tmchecks[idx1],
					   &s_tmcheck_man.tmchecks[idx2]);
}

static int tmcheck_cmp(const void *a, const void *b)
{
	const tmcheck_t *tmcheck1 = (const tmcheck_t *)a;
	const tmcheck_t *tmcheck2 = (const tmcheck_t *)b;

	if (tmcheck1->ts < tmcheck2->ts) {
		return -1;
	} else if (tmcheck1->ts > tmcheck2->ts) {
		return 1;
	} else {
		return 0;
	}
}

void osal_tmcheck_print_diff_all(void)
{
	tmcheck_t sort_tmchecks[OSAL_TMCHECK_NUM_MAX] = {0};
	tmcheck_t *prev_tmcheck = NULL;
	tmcheck_t *first_tmcheck = NULL;
	int count = 0;
	int i;

	memcpy(sort_tmchecks, s_tmcheck_man.tmchecks, sizeof(sort_tmchecks));
	qsort(sort_tmchecks, OSAL_TMCHECK_NUM_MAX,
	      sizeof(tmcheck_t), tmcheck_cmp);

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

int32_t osal_tmcheck_get_diff(int idx1, int idx2)
{
	if ((validate_idx(idx1) == false) || (validate_idx(idx2) == false)) {
		return -1;
	}
	return s_tmcheck_man.tmchecks[idx2].ts - s_tmcheck_man.tmchecks[idx1].ts;
}

void osal_tmcheck_name_print_diff(char *name1, char *name2)
{
	int i;
	tmcheck_t *tmcheck1 = NULL;
	tmcheck_t *tmcheck2 = NULL;

	for (i = 0; i < OSAL_TMCHECK_NUM_MAX; i++) {
		if (strcmp(s_tmcheck_man.tmchecks[i].name, name1) == 0) {
			tmcheck1 = &s_tmcheck_man.tmchecks[i];
		} else if (strcmp(s_tmcheck_man.tmchecks[i].name, name2) == 0) {
			tmcheck2 = &s_tmcheck_man.tmchecks[i];
		}
		if ((tmcheck1 != NULL) && (tmcheck2 != NULL)) {
			break;
		}
	}
	if ((tmcheck1 != NULL) && (tmcheck2 != NULL)) {
		tmcheck_print_diff(tmcheck2, tmcheck1);
	} else {
		OSALOG_ERROR("Invalid checkpoint name %s or %s\n", name1, name2);
	}
}

int32_t osal_tmcheck_name_get_diff(char *name1, char *name2)
{
	int i;
	tmcheck_t *tmcheck1 = NULL;
	tmcheck_t *tmcheck2 = NULL;

	for (i = 0; i < OSAL_TMCHECK_NUM_MAX; i++) {
		if (strcmp(s_tmcheck_man.tmchecks[i].name, name1) == 0) {
			tmcheck1 = &s_tmcheck_man.tmchecks[i];
		} else if (strcmp(s_tmcheck_man.tmchecks[i].name, name2) == 0) {
			tmcheck2 = &s_tmcheck_man.tmchecks[i];
		}
		if ((tmcheck1 != NULL) && (tmcheck2 != NULL)) {
			break;
		}
	}
	if ((tmcheck1 != NULL) && (tmcheck2 != NULL)) {
		return tmcheck2->ts - tmcheck1->ts;
	}
	OSALOG_ERROR("Invalid checkpoint name %s or %s\n", name1, name2);
	return -1;
}

uint64_t osal_tmcheck_get_captured_ts(int idx)
{
	if (validate_idx(idx) == false) {
		return 0;
	}
	return s_tmcheck_man.tmchecks[idx].ts;
}

uint64_t osal_tmcheck_name_get_captured_ts(char *name)
{
	int i;
	tmcheck_t *tmcheck = NULL;

	for (i = 0; i < OSAL_TMCHECK_NUM_MAX; i++) {
		if ((s_tmcheck_man.tmchecks[i].inuse != 0) &&
			(strcmp(s_tmcheck_man.tmchecks[i].name, name) == 0)) {
			tmcheck = &s_tmcheck_man.tmchecks[i];
			break;
		}
	}
	if (tmcheck != NULL) {
		return tmcheck->ts;
	}
	OSALOG_ERROR("Invalid checkpoint name %s\n", name);
	return 0;
}
