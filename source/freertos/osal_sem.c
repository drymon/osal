/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

/* FreeRTOS backend (static allocation only). */

#include <stdbool.h>
#include <stddef.h>
#include "osal_sem.h"
#include "osal_rm.h"
#include "osal_assert.h"
#include "osal_config.h"
#include "osal_freertos_priv.h"
#include "semphr.h"

#define SEM_MAX_COUNT ((UBaseType_t)0x7FFFFFFF)

struct osal_sem {
#if DMOSAL_FREERTOS_STATIC
	StaticSemaphore_t storage;
#endif
	SemaphoreHandle_t handle;
	osal_resrc_t *resrc;
};

typedef struct {
	OSAL_RM_USEROBJMAN_DECLARE(struct osal_sem, OSAL_SEM_NUM_MAX);
	bool init;
} sem_man_t;

static sem_man_t s_sem_man;

osal_error_t osal_sem_init(osal_mutex_t *mutex)
{
	if (s_sem_man.init) {
		return OSAL_E_OK;
	}

	OSAL_RM_USEROBJMAN_INIT(&s_sem_man, OSAL_SEM_NUM_MAX, mutex);
	s_sem_man.init = true;
	return OSAL_E_OK;
}

void osal_sem_deinit(void)
{
	if (!s_sem_man.init) {
		return;
	}
	osal_rm_deinit(&s_sem_man.rm);
	s_sem_man.init = false;
}

osal_sem_t *osal_sem_create(void)
{
	osal_resrc_t *resrc;
	osal_sem_t *sem;

	if (!s_sem_man.init) {
		return NULL;
	}

	resrc = osal_rm_alloc(&s_sem_man.rm);
	if (resrc == NULL) {
		return NULL;
	}

	sem = (osal_sem_t *)resrc->data;
	OSAL_RUNTIME_ASSERT(sem != NULL);
	sem->resrc = resrc;

#if DMOSAL_FREERTOS_STATIC
	sem->handle = xSemaphoreCreateCountingStatic(SEM_MAX_COUNT, 0, &sem->storage);
#else
	sem->handle = xSemaphoreCreateCounting(SEM_MAX_COUNT, 0);
#endif
	if (sem->handle == NULL) {
		osal_rm_free(&s_sem_man.rm, resrc);
		return NULL;
	}
	return sem;
}

void osal_sem_delete(osal_sem_t *sem)
{
	if (sem == NULL) {
		return;
	}

	vSemaphoreDelete(sem->handle);
	osal_rm_free(&s_sem_man.rm, sem->resrc);
}

osal_error_t osal_sem_post(osal_sem_t *sem)
{
	if (sem == NULL) {
		return OSAL_E_PARAM;
	}

	return xSemaphoreGive(sem->handle) == pdTRUE ? OSAL_E_OK : OSAL_E_OSCALL;
}

osal_error_t osal_sem_wait(osal_sem_t *sem)
{
	if (sem == NULL) {
		return OSAL_E_PARAM;
	}

	return xSemaphoreTake(sem->handle, portMAX_DELAY) == pdTRUE ? OSAL_E_OK : OSAL_E_OSCALL;
}

osal_error_t osal_sem_waittime(osal_sem_t *sem, uint32_t usec)
{
	if (sem == NULL) {
		return OSAL_E_PARAM;
	}

	if (xSemaphoreTake(sem->handle, osal_freertos_usec_to_ticks(usec)) == pdTRUE) {
		return OSAL_E_OK;
	}
	return OSAL_E_TIMEOUT;
}

uint32_t osal_sem_use(void)
{
	return s_sem_man.init ? osal_rm_use(&s_sem_man.rm) : 0;
}

uint32_t osal_sem_avail(void)
{
	return s_sem_man.init ? osal_rm_avail(&s_sem_man.rm) : 0;
}
