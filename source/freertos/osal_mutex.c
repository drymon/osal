/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

/* FreeRTOS backend (static allocation only). */

#include <stdbool.h>
#include <stddef.h>
#include "osal_mutex.h"
#include "osal_rm.h"
#include "osal_assert.h"
#include "osal_config.h"
#include "osal_freertos_priv.h"
#include "semphr.h"

struct osal_mutex {
#if DMOSAL_FREERTOS_STATIC
	StaticSemaphore_t storage;
#endif
	SemaphoreHandle_t handle;
	osal_resrc_t *resrc;
};

typedef struct {
	OSAL_RM_USEROBJMAN_DECLARE(struct osal_mutex, OSAL_MUTEX_NUM_MAX);
	bool init;
	/* Guards the pool; it cannot use osal_mutex itself. */
#if DMOSAL_FREERTOS_STATIC
	StaticSemaphore_t resrc_storage;
#endif
	SemaphoreHandle_t resrc_mutex;
} mutex_man_t;

static mutex_man_t s_mutex_man;

osal_error_t osal_mutex_init(void)
{
	if (s_mutex_man.init) {
		return OSAL_E_OK;
	}

#if DMOSAL_FREERTOS_STATIC
	s_mutex_man.resrc_mutex = xSemaphoreCreateMutexStatic(&s_mutex_man.resrc_storage);
#else
	s_mutex_man.resrc_mutex = xSemaphoreCreateMutex();
#endif
	OSAL_RUNTIME_ASSERT(s_mutex_man.resrc_mutex != NULL);
	OSAL_RM_USEROBJMAN_INIT(&s_mutex_man, OSAL_MUTEX_NUM_MAX, NULL);
	s_mutex_man.init = true;
	return OSAL_E_OK;
}

void osal_mutex_deinit(void)
{
	if (!s_mutex_man.init) {
		return;
	}
	osal_rm_deinit(&s_mutex_man.rm);
	vSemaphoreDelete(s_mutex_man.resrc_mutex);
	s_mutex_man.init = false;
}

osal_mutex_t *osal_mutex_create(void)
{
	osal_resrc_t *resrc;
	osal_mutex_t *mutex;

	if (!s_mutex_man.init) {
		return NULL;
	}

	xSemaphoreTake(s_mutex_man.resrc_mutex, portMAX_DELAY);
	resrc = osal_rm_alloc(&s_mutex_man.rm);
	xSemaphoreGive(s_mutex_man.resrc_mutex);
	if (resrc == NULL) {
		return NULL;
	}

	mutex = (osal_mutex_t *)resrc->data;
	OSAL_RUNTIME_ASSERT(mutex != NULL);
	mutex->resrc = resrc;
#if DMOSAL_FREERTOS_STATIC
	mutex->handle = xSemaphoreCreateMutexStatic(&mutex->storage);
#else
	mutex->handle = xSemaphoreCreateMutex();
#endif
	OSAL_RUNTIME_ASSERT(mutex->handle != NULL);
	return mutex;
}

void osal_mutex_delete(osal_mutex_t *mutex)
{
	if (mutex == NULL) {
		return;
	}

	vSemaphoreDelete(mutex->handle);
	xSemaphoreTake(s_mutex_man.resrc_mutex, portMAX_DELAY);
	osal_rm_free(&s_mutex_man.rm, mutex->resrc);
	xSemaphoreGive(s_mutex_man.resrc_mutex);
}

osal_error_t osal_mutex_lock(osal_mutex_t *mutex)
{
	if (mutex == NULL) {
		return OSAL_E_PARAM;
	}

	if (xSemaphoreTake(mutex->handle, portMAX_DELAY) != pdTRUE) {
		return OSAL_E_OSCALL;
	}
	return OSAL_E_OK;
}

osal_error_t osal_mutex_unlock(osal_mutex_t *mutex)
{
	if (mutex == NULL) {
		return OSAL_E_PARAM;
	}

	/* Fails when the caller is not the owner. */
	if (xSemaphoreGive(mutex->handle) != pdTRUE) {
		return OSAL_E_OSCALL;
	}
	return OSAL_E_OK;
}

uint32_t osal_mutex_use(void)
{
	return s_mutex_man.init ? osal_rm_use(&s_mutex_man.rm) : 0;
}

uint32_t osal_mutex_avail(void)
{
	return s_mutex_man.init ? osal_rm_avail(&s_mutex_man.rm) : 0;
}
