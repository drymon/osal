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

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include "osal_assert.h"
#include "osal_rm.h"
#include "osal_timer.h"
#include "osal_time.h"

struct osal_timer {
	osal_resrc_t *resrc;
	void (*expire)(void *arg);
	void *arg;
	timer_t timerid;
};

typedef struct {
	OSAL_RM_USEROBJMAN_DECLARE(
		struct osal_timer,
		OSAL_TIMER_NUM_MAX);
	bool init;
} timer_man_t;

static timer_man_t s_timer_man;

osal_error_t osal_timer_init(void)
{
	if (s_timer_man.init == true) {
		return OSAL_E_OK;
	}
	OSAL_RM_USEROBJMAN_INIT(&s_timer_man, OSAL_TIMER_NUM_MAX, true, g_osal_shared_mutex);

	s_timer_man.init = true;
	return OSAL_E_OK;
}

void osal_timer_deinit(void)
{
	if (s_timer_man.init == false) {
		return;
	}
	osal_rm_deinit(&s_timer_man.rm);
	memset(&s_timer_man, 0, sizeof(timer_man_t));
}

static void timer_handler(union sigval sv)
{
	osal_timer_t *timer = sv.sival_ptr;

	if (timer->expire) {
		timer->expire(timer->arg);
	}
}

osal_timer_t *osal_timer_create(void (*expire)(void *arg), void *arg)
{
	osal_timer_t *timer;
	osal_resrc_t *resrc;
	int res;
	struct sigevent sev;

	if (expire == NULL) {
		return NULL;
	}

	resrc = osal_rm_alloc(&s_timer_man.rm);
	if (resrc == NULL) {
		return NULL;
	}
	timer = resrc->data;
	OSAL_ASSERT(timer != NULL);
	memset(timer, 0, sizeof(osal_timer_t));

	timer->resrc = resrc;
	timer->expire = expire;
	timer->arg = arg;

	memset(&sev, 0, sizeof(struct sigevent));
	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_notify_function = timer_handler;
	sev.sigev_value.sival_ptr = timer;
	if (timer_create(CLOCK_REALTIME, &sev, &timer->timerid) < 0) {
		osal_rm_free(&s_timer_man.rm, timer->resrc);
		perror("timer_create");
		return NULL;
	}

	return timer;
}

void osal_timer_delete(osal_timer_t *timer)
{
	if (timer == NULL) {
		return;
	}
	if (timer->timerid) {
		timer_delete(timer->timerid);
	}
	memset(timer, 0, sizeof(osal_timer_t));
	osal_rm_free(&s_timer_man.rm, timer->resrc);
}

osal_error_t osal_timer_start(osal_timer_t *timer, uint32_t usec, bool repeat)
{
	struct itimerspec its;

	if ((timer == NULL) || (usec == 0)) {
		return OSAL_E_PARAM;
	}
	memset(&its, 0, sizeof(its));
	its.it_value.tv_sec = usec / OSAL_SEC_USEC;
	its.it_value.tv_nsec = (usec % OSAL_SEC_USEC) * OSAL_USEC_NSEC;
	if (repeat == true) {
		its.it_interval.tv_sec = its.it_value.tv_sec;
		its.it_interval.tv_nsec = its.it_value.tv_nsec;
	}
	if (timer_settime(timer->timerid, 0, &its, NULL) < 0) {
		perror("timer_settime");
		return OSAL_E_OSCALL;
	}
	return OSAL_E_OK;
}

void osal_timer_stop(osal_timer_t *timer)
{
	struct itimerspec its;

	if (timer == NULL) {
		return;
	}
	memset(&its, 0, sizeof(its));
	if (timer_settime(timer->timerid, 0, &its, NULL) < 0) {
		perror("timer_settime");
	}
}

uint32_t osal_timer_use(void)
{
	if (s_timer_man.init == false) {
		return 0;
	}
	return osal_rm_use(&s_timer_man.rm);
}

uint32_t osal_timer_avail(void)
{
	if (s_timer_man.init == false) {
		return 0;
	}
	return osal_rm_avail(&s_timer_man.rm);
}
