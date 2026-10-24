/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

/* FreeRTOS backend (static allocation only).
 *
 * expire() runs in the FreeRTOS timer service task, which executes callbacks one at a time.
 * osal_timer_delete() queues the kernel delete and then a pended call that releases the slot, so
 * the slot is only reused after any running callback has returned. Needs
 * INCLUDE_xTimerPendFunctionCall. */

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "osal_timer.h"
#include "osal_rm.h"
#include "osal_assert.h"
#include "osal_config.h"
#include "osal_freertos_priv.h"
#include "timers.h"

struct osal_timer {
#if DMOSAL_FREERTOS_STATIC
	StaticTimer_t storage;
#endif
	TimerHandle_t handle;
	void (*expire)(void *arg);
	void *arg;
	volatile bool in_callback;
	osal_resrc_t *resrc;
};

typedef struct {
	OSAL_RM_USEROBJMAN_DECLARE(struct osal_timer, OSAL_TIMER_NUM_MAX);
	bool init;
} timer_man_t;

static timer_man_t s_timer_man;

static void timer_callback(TimerHandle_t handle)
{
	osal_timer_t *timer = pvTimerGetTimerID(handle);

	timer->in_callback = true;
	timer->expire(timer->arg);
	timer->in_callback = false;
}

osal_error_t osal_timer_init(osal_mutex_t *mutex)
{
	if (s_timer_man.init) {
		return OSAL_E_OK;
	}

	OSAL_RM_USEROBJMAN_INIT(&s_timer_man, OSAL_TIMER_NUM_MAX, mutex);
	s_timer_man.init = true;
	return OSAL_E_OK;
}

void osal_timer_deinit(void)
{
	if (!s_timer_man.init) {
		return;
	}
	osal_rm_deinit(&s_timer_man.rm);
	s_timer_man.init = false;
}

osal_timer_t *osal_timer_create(void (*expire)(void *arg), void *arg)
{
	osal_resrc_t *resrc;
	osal_timer_t *timer;

	if (!s_timer_man.init || expire == NULL) {
		return NULL;
	}

	resrc = osal_rm_alloc(&s_timer_man.rm);
	if (resrc == NULL) {
		return NULL;
	}

	timer = (osal_timer_t *)resrc->data;
	OSAL_RUNTIME_ASSERT(timer != NULL);
	memset(timer, 0, sizeof(*timer));
	timer->resrc = resrc;
	timer->expire = expire;
	timer->arg = arg;

	/* The period is set by osal_timer_start(). */
#if DMOSAL_FREERTOS_STATIC
	timer->handle = xTimerCreateStatic("osal", 1, pdFALSE, timer, timer_callback, &timer->storage);
#else
	timer->handle = xTimerCreate("osal", 1, pdFALSE, timer, timer_callback);
#endif
	if (timer->handle == NULL) {
		osal_rm_free(&s_timer_man.rm, resrc);
		return NULL;
	}
	return timer;
}

osal_error_t osal_timer_start(osal_timer_t *timer, uint32_t usec, bool repeat)
{
	if (timer == NULL || usec == 0) {
		return OSAL_E_PARAM;
	}

	vTimerSetReloadMode(timer->handle, repeat ? pdTRUE : pdFALSE);
	/* Changing the period of a timer also (re)starts it. */
	if (xTimerChangePeriod(timer->handle, osal_freertos_usec_to_ticks(usec), portMAX_DELAY) !=
		pdPASS) {
		return OSAL_E_OSCALL;
	}
	return OSAL_E_OK;
}

void osal_timer_stop(osal_timer_t *timer)
{
	if (timer == NULL) {
		return;
	}

	xTimerStop(timer->handle, portMAX_DELAY);
}

static void timer_release(void *param1, uint32_t param2)
{
	osal_timer_t *timer = param1;

	(void)param2;
	osal_rm_free(&s_timer_man.rm, timer->resrc);
}

void osal_timer_delete(osal_timer_t *timer)
{
	if (timer == NULL) {
		return;
	}

	/* Deleting from inside the expire callback is not supported */
	if (timer->in_callback) {
		return;
	}

	xTimerDelete(timer->handle, portMAX_DELAY);
	/* The timer service task runs this after the delete command above. */
	BaseType_t queued = xTimerPendFunctionCall(timer_release, timer, 0, portMAX_DELAY);

	OSAL_RUNTIME_ASSERT(queued == pdPASS);
	(void)queued;
}

uint32_t osal_timer_use(void)
{
	return s_timer_man.init ? osal_rm_use(&s_timer_man.rm) : 0;
}

uint32_t osal_timer_avail(void)
{
	return s_timer_man.init ? osal_rm_avail(&s_timer_man.rm) : 0;
}
