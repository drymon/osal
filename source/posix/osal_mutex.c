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

#include <pthread.h>
#include "osal_rm.h"
#include "osal_assert.h"
#include "osal_mutex.h"

struct osal_mutex {
	pthread_mutex_t pthmutex;
	osal_resrc_t *resrc;
};

typedef struct {
	OSAL_RM_USEROBJMAN_DECLARE(
		struct osal_mutex,
		OSAL_MUTEX_NUM_MAX);
	/* specical case, we can not use the mutex from resource mananager because
	 * it use the osal_mutex_create() function that can be used only after
	 * osal_mutex_init().
	 */
	pthread_mutex_t resrc_mutex;
	bool init;
} mutex_man_t;

static mutex_man_t s_mutex_man;

osal_error_t osal_mutex_init(void)
{
	if (s_mutex_man.init == true) {
		return OSAL_E_OK;
	}
	OSAL_RM_USEROBJMAN_INIT(&s_mutex_man, OSAL_MUTEX_NUM_MAX, NULL);

	pthread_mutex_init(&s_mutex_man.resrc_mutex, NULL);

	s_mutex_man.init = true;

	return OSAL_E_OK;
}

osal_mutex_t *osal_mutex_create(void)
{
	osal_resrc_t *resrc;
	osal_mutex_t *mutex;

	pthread_mutex_lock(&s_mutex_man.resrc_mutex);
	resrc = osal_rm_alloc(&s_mutex_man.rm);
	pthread_mutex_unlock(&s_mutex_man.resrc_mutex);
	if (resrc == NULL) {
		return NULL;
	}
	mutex = resrc->data;
	OSAL_RUNTIME_ASSERT(mutex != NULL);
	mutex->resrc = resrc;
	pthread_mutex_init(&mutex->pthmutex, NULL);
	return mutex;
}

void osal_mutex_delete(osal_mutex_t *mutex)
{
	if (mutex == NULL) {
		return;
	}
	OSAL_RUNTIME_ASSERT(mutex->resrc != NULL);
	pthread_mutex_destroy(&mutex->pthmutex);

	pthread_mutex_lock(&s_mutex_man.resrc_mutex);
	osal_rm_free(&s_mutex_man.rm, mutex->resrc);
	pthread_mutex_unlock(&s_mutex_man.resrc_mutex);
}

osal_error_t osal_mutex_lock(osal_mutex_t *mutex)
{
	if (mutex == NULL) {
		return OSAL_E_PARAM;
	}
	/* if it fail, mean a fundamental issue occured, we will abort program */
	if (pthread_mutex_lock(&mutex->pthmutex) < 0) {
		perror("pthread_mutex_lock()");
		OSAL_RUNTIME_ASSERT(0);
		return OSAL_E_OSCALL;
	}
	return OSAL_E_OK;
}

osal_error_t osal_mutex_unlock(osal_mutex_t *mutex)
{
	if (mutex == NULL) {
		return OSAL_E_PARAM;
	}
	/* if it fail, mean a fundamental issue occured, we will abort program */
	if (pthread_mutex_unlock(&mutex->pthmutex) < 0) {
		perror("pthread_mutex_unlock()");
		OSAL_RUNTIME_ASSERT(0);
		return OSAL_E_OSCALL;
	}
	return OSAL_E_OK;
}

void osal_mutex_deinit(void)
{
	if (s_mutex_man.init == false) {
		return;
	}
	osal_rm_deinit(&s_mutex_man.rm);
	pthread_mutex_destroy(&s_mutex_man.resrc_mutex);
	s_mutex_man.init = false;
}

uint32_t osal_mutex_use(void)
{
	if (s_mutex_man.init == false) {
		return 0;
	}
	return osal_rm_use(&s_mutex_man.rm);
}

uint32_t osal_mutex_avail(void)
{
	if (s_mutex_man.init == false) {
		return 0;
	}
	return osal_rm_avail(&s_mutex_man.rm);
}
