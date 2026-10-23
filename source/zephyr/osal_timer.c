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

/* Zephyr backend (native kernel API). */

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <zephyr/kernel.h>
#include "osal_timer.h"
#include "osal_rm.h"
#include "osal_assert.h"
#include "osal_config.h"

struct osal_timer {
	/* k_timer expiry runs in ISR context; the work item runs expire() in thread context. */
	struct k_timer timer;
	struct k_work work;
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

static void timer_work_handler(struct k_work *work)
{
	osal_timer_t *timer = CONTAINER_OF(work, osal_timer_t, work);

	timer->in_callback = true;
	timer->expire(timer->arg);
	timer->in_callback = false;
}

static void timer_expiry(struct k_timer *ktimer)
{
	osal_timer_t *timer = CONTAINER_OF(ktimer, osal_timer_t, timer);

	k_work_submit(&timer->work);
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

	k_timer_init(&timer->timer, timer_expiry, NULL);
	k_work_init(&timer->work, timer_work_handler);
	return timer;
}

osal_error_t osal_timer_start(osal_timer_t *timer, uint32_t usec, bool repeat)
{
	if (timer == NULL || usec == 0) {
		return OSAL_E_PARAM;
	}

	k_timer_start(&timer->timer, K_USEC(usec), repeat ? K_USEC(usec) : K_NO_WAIT);
	return OSAL_E_OK;
}

void osal_timer_stop(osal_timer_t *timer)
{
	if (timer == NULL) {
		return;
	}

	k_timer_stop(&timer->timer);
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

	struct k_work_sync sync;

	osal_timer_stop(timer);
	k_work_cancel_sync(&timer->work, &sync);
	osal_rm_free(&s_timer_man.rm, timer->resrc);
}

uint32_t osal_timer_use(void)
{
	return s_timer_man.init ? osal_rm_use(&s_timer_man.rm) : 0;
}

uint32_t osal_timer_avail(void)
{
	return s_timer_man.init ? osal_rm_avail(&s_timer_man.rm) : 0;
}
