/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

/* FreeRTOS backend (static allocation only). */

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "osal_queue.h"
#include "osal_rm.h"
#include "osal_assert.h"
#include "osal_config.h"
#include "osal_freertos_priv.h"
#include "queue.h"

/* Shared by every handle created with the same name (mirrors Linux mq_open attach). */
typedef struct {
#if DMOSAL_FREERTOS_STATIC
	StaticQueue_t storage;
	uint8_t buf[OSAL_QUEUE_BUF_SIZE];
#endif
	QueueHandle_t handle;
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
		osal_error_t res = osal_mutex_lock(s_queue_man.mutex);

		OSAL_RUNTIME_ASSERT(res == OSAL_E_OK);
		(void)res;
	}
}

static void cores_unlock(void)
{
	if (s_queue_man.mutex != NULL) {
		osal_error_t res = osal_mutex_unlock(s_queue_man.mutex);

		OSAL_RUNTIME_ASSERT(res == OSAL_E_OK);
		(void)res;
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
#if DMOSAL_FREERTOS_STATIC
	free_core->handle =
		xQueueCreateStatic(cfg->qsize, cfg->msglen, free_core->buf, &free_core->storage);
#else
	free_core->handle = xQueueCreate(cfg->qsize, cfg->msglen);
#endif
	if (free_core->handle == NULL) {
		return NULL;
	}
	memcpy(free_core->name, cfg->name, OSAL_QUEUE_NAME_SIZE);
	free_core->msglen = cfg->msglen;
	free_core->qsize = cfg->qsize;
	free_core->refs = 1;
	return free_core;
}

osal_queue_t *osal_queue_create(const osal_queue_cfg_t *cfg)
{
	osal_resrc_t *resrc;
	osal_queue_t *queue;
	queue_core_t *core;

	if (!s_queue_man.init || cfg == NULL || cfg->name[0] == '\0' || cfg->msglen == 0 ||
		cfg->qsize == 0 || cfg->msglen > OSAL_QUEUE_MSG_MAX_SIZE ||
		cfg->qsize > OSAL_QUEUE_MSG_NUM_MAX) {
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
		vQueueDelete(core->handle);
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

	/* The queue copies a fixed msglen; zero-pad shorter messages. */
	uint8_t tmp[OSAL_QUEUE_MSG_MAX_SIZE] = {0};

	memcpy(tmp, msg, msglen);
	return xQueueSend(queue->core->handle, tmp, 0) == pdTRUE ? OSAL_E_OK : OSAL_E_QFULL;
}

osal_error_t
osal_queue_recv(osal_queue_t *queue, uint8_t *buf, uint32_t bufsize, uint32_t timeout_usec)
{
	if (queue == NULL || buf == NULL || bufsize < queue->core->msglen) {
		return OSAL_E_PARAM;
	}

	if (xQueueReceive(queue->core->handle, buf, osal_freertos_usec_to_ticks(timeout_usec)) ==
		pdTRUE) {
		return OSAL_E_OK;
	}
	return OSAL_E_TIMEOUT;
}

uint32_t osal_queue_use(void)
{
	return s_queue_man.init ? osal_rm_use(&s_queue_man.rm) : 0;
}

uint32_t osal_queue_avail(void)
{
	return s_queue_man.init ? osal_rm_avail(&s_queue_man.rm) : 0;
}
