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
#include <errno.h>
#include <zephyr/kernel.h>
#include "osal_queue.h"
#include "osal_rm.h"
#include "osal_assert.h"
#include "osal_config.h"

/* Shared by every handle created with the same name (mirrors POSIX mq_open attach). */
typedef struct {
	struct k_msgq msgq;
	char buf[CONFIG_DMOSAL_QUEUE_BUF_SIZE] __aligned(sizeof(void *));
	char name[OSAL_QUEUE_NAME_SIZE + 1];
	uint32_t msglen;
	uint32_t qsize;
	uint32_t refs;
} queue_core_t;

struct osal_queue {
	queue_core_t *core;
	osal_resrc_t *resrc;
};

typedef struct {
	OSAL_RM_USEROBJMAN_DECLARE(struct osal_queue, OSAL_QUEUE_NUM_MAX);
	queue_core_t cores[OSAL_QUEUE_NUM_MAX];
	osal_mutex_t *mutex; /* shared mutex, protects cores[]; may be NULL */
	bool init;
} queue_man_t;

static queue_man_t s_queue_man;

static void cores_lock(void)
{
	if (s_queue_man.mutex != NULL) {
		OSAL_RUNTIME_ASSERT(osal_mutex_lock(s_queue_man.mutex) == OSAL_E_OK);
	}
}

static void cores_unlock(void)
{
	if (s_queue_man.mutex != NULL) {
		OSAL_RUNTIME_ASSERT(osal_mutex_unlock(s_queue_man.mutex) == OSAL_E_OK);
	}
}

osal_error_t osal_queue_init(osal_mutex_t *mutex)
{
	if (s_queue_man.init) {
		return OSAL_E_OK;
	}

	OSAL_RM_USEROBJMAN_INIT(&s_queue_man, OSAL_QUEUE_NUM_MAX, mutex);
	memset(s_queue_man.cores, 0, sizeof(s_queue_man.cores));
	s_queue_man.mutex = mutex;
	s_queue_man.init = true;
	return OSAL_E_OK;
}

void osal_queue_deinit(void)
{
	if (!s_queue_man.init) {
		return;
	}
	osal_rm_deinit(&s_queue_man.rm);
	s_queue_man.init = false;
}

/* Find the core with this name, or claim a free one. Caller holds the lock. */
static queue_core_t *core_get(const osal_queue_cfg_t *cfg)
{
	queue_core_t *free_core = NULL;

	for (int i = 0; i < OSAL_QUEUE_NUM_MAX; i++) {
		queue_core_t *core = &s_queue_man.cores[i];

		if (core->refs == 0) {
			if (free_core == NULL) {
				free_core = core;
			}
		} else if (strncmp(core->name, (const char *)cfg->name, OSAL_QUEUE_NAME_SIZE) == 0) {
			if (core->msglen != cfg->msglen || core->qsize != cfg->qsize) {
				return NULL;
			}
			core->refs++;
			return core;
		}
	}

	if (free_core == NULL) {
		return NULL;
	}
	memset(free_core, 0, sizeof(*free_core));
	memcpy(free_core->name, cfg->name, OSAL_QUEUE_NAME_SIZE);
	free_core->msglen = cfg->msglen;
	free_core->qsize = cfg->qsize;
	free_core->refs = 1;
	k_msgq_init(&free_core->msgq, free_core->buf, cfg->msglen, cfg->qsize);
	return free_core;
}

osal_queue_t *osal_queue_create(const osal_queue_cfg_t *cfg)
{
	osal_resrc_t *resrc;
	osal_queue_t *queue;
	queue_core_t *core;

	if (!s_queue_man.init || cfg == NULL || cfg->name[0] == '\0' || cfg->msglen == 0 ||
		cfg->qsize == 0 || cfg->msglen > CONFIG_DMOSAL_QUEUE_MSG_MAX_SIZE ||
		cfg->msglen * cfg->qsize > CONFIG_DMOSAL_QUEUE_BUF_SIZE) {
		return NULL;
	}

	resrc = osal_rm_alloc(&s_queue_man.rm);
	if (resrc == NULL) {
		return NULL;
	}

	cores_lock();
	core = core_get(cfg);
	cores_unlock();
	if (core == NULL) {
		osal_rm_free(&s_queue_man.rm, resrc);
		return NULL;
	}

	queue = (osal_queue_t *)resrc->data;
	OSAL_RUNTIME_ASSERT(queue != NULL);
	queue->resrc = resrc;
	queue->core = core;
	return queue;
}

void osal_queue_delete(osal_queue_t *queue)
{
	if (queue == NULL) {
		return;
	}

	osal_resrc_t *resrc = queue->resrc;
	queue_core_t *core = queue->core;

	/* The queue and its pending messages live until the last handle is deleted. */
	cores_lock();
	if (--core->refs == 0) {
		k_msgq_purge(&core->msgq);
		memset(core, 0, sizeof(*core));
	}
	cores_unlock();

	memset(queue, 0, sizeof(*queue));
	osal_rm_free(&s_queue_man.rm, resrc);
}

osal_error_t osal_queue_send(osal_queue_t *queue, uint8_t *msg, uint32_t msglen)
{
	if (queue == NULL || msg == NULL || msglen == 0 || msglen > queue->core->msglen) {
		return OSAL_E_PARAM;
	}

	/* k_msgq copies a fixed msglen; zero-pad shorter messages. */
	uint8_t tmp[CONFIG_DMOSAL_QUEUE_MSG_MAX_SIZE] = {0};
	int res;

	memcpy(tmp, msg, msglen);
	res = k_msgq_put(&queue->core->msgq, tmp, K_NO_WAIT);
	if (res == -ENOMSG) {
		return OSAL_E_QFULL;
	}
	return res == 0 ? OSAL_E_OK : OSAL_E_OSCALL;
}

osal_error_t
osal_queue_recv(osal_queue_t *queue, uint8_t *buf, uint32_t bufsize, uint32_t timeout_usec)
{
	if (queue == NULL || buf == NULL || bufsize < queue->core->msglen) {
		return OSAL_E_PARAM;
	}

	int res = k_msgq_get(&queue->core->msgq, buf, K_USEC(timeout_usec));

	if (res == -ENOMSG || res == -EAGAIN) {
		return OSAL_E_TIMEOUT;
	}
	return res == 0 ? OSAL_E_OK : OSAL_E_OSCALL;
}

uint32_t osal_queue_use(void)
{
	return s_queue_man.init ? osal_rm_use(&s_queue_man.rm) : 0;
}

uint32_t osal_queue_avail(void)
{
	return s_queue_man.init ? osal_rm_avail(&s_queue_man.rm) : 0;
}
