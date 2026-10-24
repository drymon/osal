/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

#include <pthread.h>
#include <string.h>
#include "osal.h"

struct osal_task {
	pthread_t tid;
	osal_resrc_t *resrc;
	osal_task_cfg_t taskcfg;
};

typedef struct {
	OSAL_RM_USEROBJMAN_DECLARE(struct osal_task, OSAL_TASK_NUM_MAX);
	bool init;
} task_man_t;

static task_man_t s_task_man;

osal_error_t osal_task_init(osal_mutex_t *mutex)
{
	if (s_task_man.init == true) {
		return OSAL_E_OK;
	}
	OSAL_RM_USEROBJMAN_INIT(&s_task_man, OSAL_TASK_NUM_MAX, mutex);
	s_task_man.init = true;

	return OSAL_E_OK;
}

void osal_task_deinit(void)
{
	if (s_task_man.init == false) {
		return;
	}
	osal_rm_deinit(&s_task_man.rm);
	s_task_man.init = false;
}

static void *task_run(void *arg)
{
	osal_task_t *task = arg;

	task->taskcfg.task_handler(task->taskcfg.task_arg);
	pthread_exit(NULL);
	return NULL;
}

osal_task_t *osal_task_create(const osal_task_cfg_t *cfg)
{
	osal_task_t *task;
	osal_resrc_t *resrc;
	int res;

	if ((cfg == NULL) || (cfg->task_handler == NULL)) {
		return NULL;
	}

	resrc = osal_rm_alloc(&s_task_man.rm);
	if (resrc == NULL) {
		return NULL;
	}
	task = resrc->data;
	OSAL_RUNTIME_ASSERT(task != NULL);
	task->resrc = resrc;
	memcpy(&task->taskcfg, cfg, sizeof(osal_task_cfg_t));
	res = pthread_create(&task->tid, NULL, task_run, task);
	OSAL_RUNTIME_ASSERT(res == 0);

	return task;
}

void osal_task_delete(osal_task_t *task)
{
	if (task == NULL) {
		return;
	}
	pthread_cancel(task->tid);
	pthread_join(task->tid, NULL);
	osal_rm_free(&s_task_man.rm, task->resrc);
}

uint32_t osal_task_use(void)
{
	if (s_task_man.init == false) {
		return 0;
	}
	return osal_rm_use(&s_task_man.rm);
}

uint32_t osal_task_avail(void)
{
	if (s_task_man.init == false) {
		return 0;
	}
	return osal_rm_avail(&s_task_man.rm);
}
