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
		OSAL_ASSERT(err == OSAL_E_OK);
	}

	osal_lifo_init(&rm->resrc_pool);

	if (rm->mutex != NULL) {
		err = osal_mutex_unlock(rm->mutex);
		OSAL_ASSERT(err == OSAL_E_OK);
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
		OSAL_ASSERT(err == OSAL_E_OK);
	}

	resrc = (osal_resrc_t *)osal_lifo_pop(&rm->resrc_pool);
	if (resrc != NULL) {
		OSAL_ASSERT(resrc->used == false);
		resrc->used = true;
	}

	if (rm->mutex != NULL) {
		err = osal_mutex_unlock(rm->mutex);
		OSAL_ASSERT(err == OSAL_E_OK);
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
		OSAL_ASSERT(err == OSAL_E_OK);
	}

	OSAL_ASSERT(resrc->used == true);
	resrc->used = false;
	osal_lifo_push(&rm->resrc_pool, &resrc->node);

	if (rm->mutex != NULL) {
		err = osal_mutex_unlock(rm->mutex);
		OSAL_ASSERT(err == OSAL_E_OK);
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
		OSAL_ASSERT(err == OSAL_E_OK);
	}

	avail = osal_lifo_size(&rm->resrc_pool);
	OSAL_ASSERT(avail <= rm->n_resrces);

	if (rm->mutex != NULL) {
		err = osal_mutex_unlock(rm->mutex);
		OSAL_ASSERT(err == OSAL_E_OK);
	}

	return avail;
}

uint32_t osal_rm_use(osal_rm_t *rm)
{
	uint32_t avail;

	avail = osal_rm_avail(rm);
	return (rm->n_resrces - avail);
}
