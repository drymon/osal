/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

#include <string.h>
#include "osal_assert.h"
#include "osal_rm.h"

osal_error_t osal_rm_init(osal_rm_t *rm, const osal_rm_cfg_t *cfg)
{
	uint32_t i;

	if ((rm == NULL) || (cfg == NULL)) {
		return OSAL_E_PARAM;
	}
	memset(rm, 0, sizeof(osal_rm_t));
	rm->n_resrces = cfg->n_resrces;

	osal_lifo_init(&rm->resrc_pool);
	for (i = 0; i < cfg->n_resrces; i++) {
		cfg->resrces[i].used = false;
		osal_lifo_push(&rm->resrc_pool, &cfg->resrces[i].node);
	}
	rm->mutex = cfg->mutex;

	return OSAL_E_OK;
}

void osal_rm_deinit(osal_rm_t *rm)
{
	osal_error_t err;

	if (rm == NULL) {
		return;
	}
	if (rm->mutex != NULL) {
		err = osal_mutex_lock(rm->mutex);
		OSAL_RUNTIME_ASSERT(err == OSAL_E_OK);
	}

	osal_lifo_init(&rm->resrc_pool);

	if (rm->mutex != NULL) {
		err = osal_mutex_unlock(rm->mutex);
		OSAL_RUNTIME_ASSERT(err == OSAL_E_OK);
	}
	rm->mutex = NULL;
}

osal_resrc_t *osal_rm_alloc(osal_rm_t *rm)
{
	osal_resrc_t *resrc = NULL;
	osal_error_t err;

	if (rm == NULL) {
		return NULL;
	}

	if (rm->mutex != NULL) {
		err = osal_mutex_lock(rm->mutex);
		OSAL_RUNTIME_ASSERT(err == OSAL_E_OK);
	}

	resrc = (osal_resrc_t *)osal_lifo_pop(&rm->resrc_pool);
	if (resrc != NULL) {
		OSAL_RUNTIME_ASSERT(resrc->used == false);
		resrc->used = true;
	}

	if (rm->mutex != NULL) {
		err = osal_mutex_unlock(rm->mutex);
		OSAL_RUNTIME_ASSERT(err == OSAL_E_OK);
	}

	return resrc;
}

void osal_rm_free(osal_rm_t *rm, osal_resrc_t *resrc)
{
	osal_error_t err;

	if ((rm == NULL) || (resrc == NULL)) {
		return;
	}

	if (rm->mutex != NULL) {
		err = osal_mutex_lock(rm->mutex);
		OSAL_RUNTIME_ASSERT(err == OSAL_E_OK);
	}

	OSAL_RUNTIME_ASSERT(resrc->used == true);
	resrc->used = false;
	osal_lifo_push(&rm->resrc_pool, &resrc->node);

	if (rm->mutex != NULL) {
		err = osal_mutex_unlock(rm->mutex);
		OSAL_RUNTIME_ASSERT(err == OSAL_E_OK);
	}
}

uint32_t osal_rm_avail(osal_rm_t *rm)
{
	uint32_t avail;
	osal_error_t err;

	if (rm == NULL) {
		return 0;
	}

	if (rm->mutex != NULL) {
		err = osal_mutex_lock(rm->mutex);
		OSAL_RUNTIME_ASSERT(err == OSAL_E_OK);
	}

	avail = osal_lifo_size(&rm->resrc_pool);
	OSAL_RUNTIME_ASSERT(avail <= rm->n_resrces);

	if (rm->mutex != NULL) {
		err = osal_mutex_unlock(rm->mutex);
		OSAL_RUNTIME_ASSERT(err == OSAL_E_OK);
	}

	return avail;
}

uint32_t osal_rm_use(osal_rm_t *rm)
{
	uint32_t avail;

	avail = osal_rm_avail(rm);
	return (rm->n_resrces - avail);
}
