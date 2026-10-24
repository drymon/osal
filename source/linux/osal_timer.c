/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

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
#include "osal_log.h"
#define OSALOG_MODULE OSAL_LOG_MODULE_INDEX

struct osal_timer {
	osal_resrc_t *resrc;
	void (*expire)(void *arg);
	void *arg;
	timer_t timerid;
	/* True while this timer's expire callback is executing. Used to reject
	 * osal_timer_delete calls made from within the callback (or from any
	 * other thread while the callback is still running).
	 */
	bool in_callback;
};

typedef struct {
	OSAL_RM_USEROBJMAN_DECLARE(struct osal_timer, OSAL_TIMER_NUM_MAX);
	bool init;
} timer_man_t;

static timer_man_t s_timer_man;

osal_error_t osal_timer_init(osal_mutex_t *mutex)
{
	if (s_timer_man.init == true) {
		return OSAL_E_OK;
	}
	OSAL_RM_USEROBJMAN_INIT(&s_timer_man, OSAL_TIMER_NUM_MAX, mutex);

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

	/* Mark the timer as inside its callback so osal_timer_delete can reject
	 * a concurrent delete. Users must serialize delete against callback
	 * execution externally (e.g. osal_timer_stop first).
	 */
	timer->in_callback = true;

	if (timer->expire != NULL) {
		timer->expire(timer->arg);
	}

	timer->in_callback = false;
}

osal_timer_t *osal_timer_create(void (*expire)(void *arg), void *arg)
{
	osal_timer_t *timer;
	osal_resrc_t *resrc;
	struct sigevent sev;

	if (expire == NULL) {
		return NULL;
	}

	resrc = osal_rm_alloc(&s_timer_man.rm);
	if (resrc == NULL) {
		return NULL;
	}
	timer = resrc->data;
	OSAL_RUNTIME_ASSERT(timer != NULL);
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
	osal_resrc_t *resrc;

	if (timer == NULL) {
		return;
	}
	/* Reject delete while the callback is executing. Freeing the timer out
	 * from under the still-running callback is a programming error, whether
	 * the delete is called from the callback itself or from another thread.
	 */
	if (timer->in_callback) {
		OSALOG_ERROR("osal_timer_delete: called while the timer's callback is running\n");
		return;
	}
	/* Stop future expirations so no new SIGEV_THREAD handler fires. */
	if (timer->timerid) {
		timer_delete(timer->timerid);
	}
	resrc = timer->resrc;
	memset(timer, 0, sizeof(osal_timer_t));
	osal_rm_free(&s_timer_man.rm, resrc);
}

osal_error_t osal_timer_start(osal_timer_t *timer, uint32_t usec, bool repeat)
{
	struct itimerspec its;

	if ((timer == NULL) || (usec == 0)) {
		return OSAL_E_PARAM;
	}
	memset(&its, 0, sizeof(its));
	its.it_value.tv_sec = (time_t)(usec / OSAL_SEC_USEC);
	its.it_value.tv_nsec = (long)((usec % OSAL_SEC_USEC) * OSAL_USEC_NSEC);
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
