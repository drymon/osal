/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

/* Zephyr backend (native kernel API). */

#include <stdbool.h>
#include <stddef.h>
#include <zephyr/kernel.h>
#include "osal_mutex.h"
#include "osal_rm.h"
#include "osal_assert.h"
#include "osal_config.h"

struct osal_mutex {
	struct k_mutex mutex;
	osal_resrc_t *resrc;
};

typedef struct {
	OSAL_RM_USEROBJMAN_DECLARE(struct osal_mutex, OSAL_MUTEX_NUM_MAX);
	bool init;
	/* Guards the pool; it cannot use osal_mutex itself. */
	struct k_mutex resrc_mutex;
} mutex_man_t;

static mutex_man_t s_mutex_man;

osal_error_t osal_mutex_init(void)
{
	if (s_mutex_man.init) {
		return OSAL_E_OK;
	}

	k_mutex_init(&s_mutex_man.resrc_mutex);
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
	s_mutex_man.init = false;
}

osal_mutex_t *osal_mutex_create(void)
{
	osal_resrc_t *resrc;
	osal_mutex_t *mutex;

	if (!s_mutex_man.init) {
		return NULL;
	}

	k_mutex_lock(&s_mutex_man.resrc_mutex, K_FOREVER);
	resrc = osal_rm_alloc(&s_mutex_man.rm);
	k_mutex_unlock(&s_mutex_man.resrc_mutex);
	if (resrc == NULL) {
		return NULL;
	}

	mutex = (osal_mutex_t *)resrc->data;
	OSAL_RUNTIME_ASSERT(mutex != NULL);
	mutex->resrc = resrc;

	k_mutex_init(&mutex->mutex);
	return mutex;
}

void osal_mutex_delete(osal_mutex_t *mutex)
{
	if (mutex == NULL) {
		return;
	}

	k_mutex_lock(&s_mutex_man.resrc_mutex, K_FOREVER);
	osal_rm_free(&s_mutex_man.rm, mutex->resrc);
	k_mutex_unlock(&s_mutex_man.resrc_mutex);
}

osal_error_t osal_mutex_lock(osal_mutex_t *mutex)
{
	if (mutex == NULL) {
		return OSAL_E_PARAM;
	}

	if (k_mutex_lock(&mutex->mutex, K_FOREVER) != 0) {
		return OSAL_E_OSCALL;
	}
	return OSAL_E_OK;
}

osal_error_t osal_mutex_unlock(osal_mutex_t *mutex)
{
	if (mutex == NULL) {
		return OSAL_E_PARAM;
	}

	if (k_mutex_unlock(&mutex->mutex) != 0) {
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
