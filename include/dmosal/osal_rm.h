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

#ifndef OSAL_RM_H
#define OSAL_RM_H

#include <stdbool.h>
#include <stdint.h>
#include "osal_mutex.h"
#include "osal_lifo.h"

typedef struct {
	/* single list is used to reduce memory consumption */
	osal_lifo_node_t node;
	bool used;
	/* below are assigned by the user, this layer does not touch it */
	void *data;
} osal_resrc_t;

typedef struct {
	osal_mutex_t *mutex;
	bool safe;
	osal_lifo_t resrc_pool;
	uint32_t n_resrces;
} osal_rm_t; /* resource manager */

typedef struct {
	/* thread-safe */
	bool safe;
	uint32_t n_resrces;
	/* an array of resources, must be non-stack array */
	osal_resrc_t *resrces;
} osal_rm_cfg_t;

osal_error_t osal_rm_init(osal_rm_t *rm, osal_rm_cfg_t *cfg);
void osal_rm_deinit(osal_rm_t *rm);
osal_resrc_t *osal_rm_alloc(osal_rm_t *rm);
void osal_rm_free(osal_rm_t *rm, osal_resrc_t *resrc);
uint32_t osal_rm_avail(osal_rm_t *rm);
uint32_t osal_rm_use(osal_rm_t *rm);

#define OSAL_RM_USEROBJMAN_DECLARE(userobj_type, userobj_num) \
	userobj_type userobj[userobj_num]; \
	osal_resrc_t resrces[userobj_num]; \
	osal_rm_t rm;

#define OSAL_RM_USEROBJMAN_INIT(userobjman_ptr, userobj_num, issafe) \
	{ \
		osal_rm_cfg_t rmcfg; \
		int i; \
		int res; \
		for (i = 0; i < userobj_num; i++) { \
			(userobjman_ptr)->resrces[i].data = &(userobjman_ptr)->userobj[i]; \
		} \
		rmcfg.safe = issafe; \
		rmcfg.n_resrces = userobj_num; \
		rmcfg.resrces = (userobjman_ptr)->resrces; \
		res = osal_rm_init(&(userobjman_ptr)->rm, &rmcfg); \
		OSAL_ASSERT(res == OSAL_E_OK); \
	}

#endif //OSAL_RM_H
