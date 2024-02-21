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

#include <semaphore.h>
#include <time.h>
#include <errno.h>
#include "osal_rm.h"
#include "osal_assert.h"
#include "osal_sem.h"
#include "osal_time.h"

struct osal_sem {
	sem_t psem;
	osal_resrc_t *resrc;
};

typedef struct {
	OSAL_RM_USEROBJMAN_DECLARE(
		struct osal_sem,
		OSAL_SEM_NUM_MAX);
	bool init;
} sem_man_t;

static sem_man_t s_sem_man;

osal_error_t osal_sem_init(void)
{
	if (s_sem_man.init == true) {
		return OSAL_E_OK;
	}
	OSAL_RM_USEROBJMAN_INIT(&s_sem_man, OSAL_SEM_NUM_MAX, true, g_osal_shared_mutex);
	s_sem_man.init = true;

	return OSAL_E_OK;
}

void osal_sem_deinit(void)
{
	if (s_sem_man.init == false) {
		return;
	}
	osal_rm_deinit(&s_sem_man.rm);
	s_sem_man.init = false;
}

osal_sem_t *osal_sem_create(void)
{
	osal_resrc_t *resrc;
	osal_sem_t *sem;

	resrc = osal_rm_alloc(&s_sem_man.rm);
	if (resrc == NULL) {
		return NULL;
	}
	sem = resrc->data;
	OSAL_ASSERT(sem != NULL);
	sem->resrc = resrc;
	sem_init(&sem->psem, 0, 0);
	return sem;
}

void osal_sem_delete(osal_sem_t *sem)
{
	if (sem == NULL) {
		return;
	}
	sem_destroy(&sem->psem);
	OSAL_ASSERT(sem->resrc != NULL);
	osal_rm_free(&s_sem_man.rm, sem->resrc);
}

osal_error_t osal_sem_post(osal_sem_t *sem)
{
	if (sem == NULL) {
		return OSAL_E_PARAM;
	}
	if (sem_post(&sem->psem) < 0) {
		perror("sem_post");
		return OSAL_E_OSCALL;
	}
	return OSAL_E_OK;
}

osal_error_t osal_sem_wait(osal_sem_t *sem)
{
	if (sem == NULL) {
		return OSAL_E_PARAM;
	}
	if (sem_wait(&sem->psem) < 0){
		perror("sem_wait");
		return OSAL_E_OSCALL;
	}
	return OSAL_E_OK;
}

osal_error_t osal_sem_waittime(osal_sem_t *sem, uint32_t usec)
{
	struct timespec ts;
	uint64_t ns;
	int res;

	if (sem == NULL) {
		return OSAL_E_PARAM;
	}

	if (clock_gettime(CLOCK_REALTIME, &ts) < 0){
		return OSAL_E_OSCALL;
	}

	ns = (uint64_t)ts.tv_nsec + ((uint64_t)usec * OSAL_USEC_NSEC);
	ts.tv_sec += ns / OSAL_SEC_NSEC;
	ts.tv_nsec = ns % OSAL_SEC_NSEC;

	res = sem_timedwait(&sem->psem, &ts);
	if (res < 0) {
		if (errno == ETIMEDOUT) {
			return OSAL_E_TIMEOUT;
		} else {
			perror("sem_timedwait");
			return OSAL_E_OSCALL;
		}
	}
	return OSAL_E_OK;
}

uint32_t osal_sem_use(void)
{
	if (s_sem_man.init == false) {
		return 0;
	}
	return osal_rm_use(&s_sem_man.rm);
}

uint32_t osal_sem_avail(void)
{
	if (s_sem_man.init == false) {
		return 0;
	}
	return osal_rm_avail(&s_sem_man.rm);
}
