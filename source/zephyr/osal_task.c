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
#include <zephyr/kernel.h>
#include "osal_task.h"
#include "osal_rm.h"
#include "osal_assert.h"
#include "osal_config.h"

struct osal_task {
	struct k_thread thread;
	osal_task_cfg_t taskcfg;
	osal_resrc_t *resrc;
};

typedef struct {
	OSAL_RM_USEROBJMAN_DECLARE(struct osal_task, OSAL_TASK_NUM_MAX);
	bool init;
} task_man_t;

static task_man_t s_task_man;

/* One static stack per pool slot; the slot index selects the stack. */
static K_THREAD_STACK_ARRAY_DEFINE(s_stacks, OSAL_TASK_NUM_MAX, CONFIG_DMOSAL_TASK_STACK_SIZE);

static void task_entry(void *p1, void *p2, void *p3)
{
	osal_task_t *task = p1;

	(void)p2;
	(void)p3;
	task->taskcfg.task_handler(task->taskcfg.task_arg);
}

osal_error_t osal_task_init(osal_mutex_t *mutex)
{
	if (s_task_man.init) {
		return OSAL_E_OK;
	}

	OSAL_RM_USEROBJMAN_INIT(&s_task_man, OSAL_TASK_NUM_MAX, mutex);
	s_task_man.init = true;
	return OSAL_E_OK;
}

void osal_task_deinit(void)
{
	if (!s_task_man.init) {
		return;
	}
	osal_rm_deinit(&s_task_man.rm);
	s_task_man.init = false;
}

osal_task_t *osal_task_create(const osal_task_cfg_t *cfg)
{
	osal_resrc_t *resrc;
	osal_task_t *task;

	if (!s_task_man.init || cfg == NULL || cfg->task_handler == NULL) {
		return NULL;
	}

	resrc = osal_rm_alloc(&s_task_man.rm);
	if (resrc == NULL) {
		return NULL;
	}

	task = (osal_task_t *)resrc->data;
	OSAL_RUNTIME_ASSERT(task != NULL);
	memcpy(&task->taskcfg, cfg, sizeof(*cfg));
	task->resrc = resrc;

	size_t idx = (size_t)(task - s_task_man.userobj);
	k_tid_t tid = k_thread_create(&task->thread,
								  s_stacks[idx],
								  K_THREAD_STACK_SIZEOF(s_stacks[idx]),
								  task_entry,
								  task,
								  NULL,
								  NULL,
								  K_PRIO_PREEMPT(CONFIG_DMOSAL_TASK_PRIORITY),
								  0,
								  K_NO_WAIT);
	if (tid == NULL) {
		osal_rm_free(&s_task_man.rm, resrc);
		return NULL;
	}
	if (task->taskcfg.name[0] != '\0') {
		k_thread_name_set(tid, (const char *)task->taskcfg.name);
	}
	return task;
}

void osal_task_delete(osal_task_t *task)
{
	if (task == NULL) {
		return;
	}

	k_thread_abort(&task->thread);
	osal_rm_free(&s_task_man.rm, task->resrc);
}

uint32_t osal_task_use(void)
{
	return s_task_man.init ? osal_rm_use(&s_task_man.rm) : 0;
}

uint32_t osal_task_avail(void)
{
	return s_task_man.init ? osal_rm_avail(&s_task_man.rm) : 0;
}
