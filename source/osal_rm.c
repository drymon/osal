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

#include <string.h>
#include "osal_assert.h"
#include "osal_rm.h"

osal_error_t osal_rm_init(osal_rm_t *rm, osal_rm_cfg_t *cfg)
{
	int i;

	if ((rm == NULL) || (cfg == NULL)) {
		return OSAL_E_PARAM;
	}
	memset(rm, 0, sizeof(osal_rm_t));
	rm->safe = cfg->safe;
	rm->n_resrces = cfg->n_resrces;

	osal_lifo_init(&rm->resrc_pool);
	for (i = 0; i < cfg->n_resrces; i++) {
		cfg->resrces[i].used = false;
		osal_lifo_push(&rm->resrc_pool, &cfg->resrces[i].node);
	}

	if (rm->safe == true) {
		if (cfg->mutex) {
			rm->mutex = cfg->mutex;
		} else {
			rm->mutex = osal_mutex_create();
			rm->mutex_created = true;
		}
		OSAL_ASSERT(rm->mutex != NULL);
	}
	return OSAL_E_OK;
}

void osal_rm_deinit(osal_rm_t *rm)
{
	int i;

	if (rm == NULL) {
		return;
	}
	if (rm->safe == true) {
		OSAL_ASSERT(rm->mutex != NULL);
		osal_mutex_lock(rm->mutex);
	}
	osal_lifo_init(&rm->resrc_pool);
	if (rm->safe == true) {
		OSAL_ASSERT(rm->mutex != NULL);
		osal_mutex_unlock(rm->mutex);
		if (rm->mutex_created == true) {
			osal_mutex_delete(rm->mutex);
		}
	}

	rm->safe = false;
	rm->mutex = NULL;
}

osal_resrc_t *osal_rm_alloc(osal_rm_t *rm)
{
	int i;
	osal_resrc_t *resrc = NULL;

	if (rm == NULL) {
		return NULL;
	}
	if (rm->safe == true) {
		OSAL_ASSERT(rm->mutex != NULL);
		osal_mutex_lock(rm->mutex);
	}
	resrc = (osal_resrc_t *)osal_lifo_pop(&rm->resrc_pool);
	if (resrc != NULL) {
		OSAL_ASSERT(resrc->used == false);
		resrc->used = true;
	}
	if (rm->safe == true) {
		OSAL_ASSERT(rm->mutex != NULL);
		osal_mutex_unlock(rm->mutex);
	}
	return resrc;
}

void osal_rm_free(osal_rm_t *rm, osal_resrc_t *resrc)
{
	int i;

	if ((rm == NULL) || (resrc == NULL)) {
		return;
	}
	if (rm->safe == true) {
		OSAL_ASSERT(rm->mutex != NULL);
		osal_mutex_lock(rm->mutex);
	}
	OSAL_ASSERT(resrc->used == true);
	resrc->used = false;
	osal_lifo_push(&rm->resrc_pool, &resrc->node);
	if (rm->safe == true) {
		OSAL_ASSERT(rm->mutex != NULL);
		osal_mutex_unlock(rm->mutex);
	}
}

uint32_t osal_rm_avail(osal_rm_t *rm)
{
	uint32_t avail;

	if (rm == NULL) {
		return 0;
	}
	if (rm->safe == true) {
		OSAL_ASSERT(rm->mutex != NULL);
		osal_mutex_lock(rm->mutex);
	}

	avail = osal_lifo_size(&rm->resrc_pool);
	OSAL_ASSERT(avail <= rm->n_resrces);

	if (rm->safe == true) {
		OSAL_ASSERT(rm->mutex != NULL);
		osal_mutex_unlock(rm->mutex);
	}
	return avail;
}

uint32_t osal_rm_use(osal_rm_t *rm)
{
	uint32_t avail;

	avail = osal_rm_avail(rm);
	return (rm->n_resrces - avail);
}
