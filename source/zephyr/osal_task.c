/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

/* Zephyr backend (native kernel API). */

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <zephyr/kernel.h>
#include "osal_task.h"
#include "osal_rm.h"
#include "osal_assert.h"
#include "osal_config.h"

/* Tasks are kernel threads (no user mode), so they use kernel stacks. */
#if OSAL_TASK_STACK_EMBEDDED
#define TASK_STACK_MEMBER	   K_KERNEL_STACK_MEMBER(stack, OSAL_TASK_STACK_SIZE_DEFAULT);
#define EMBEDDED_STACK(t)	   ((t)->stack)
#define EMBEDDED_STACK_SIZE(t) K_KERNEL_STACK_SIZEOF((t)->stack)
#else
#define TASK_STACK_MEMBER
#define EMBEDDED_STACK(t)	   ((k_thread_stack_t *)NULL)
#define EMBEDDED_STACK_SIZE(t) 0
#endif

struct osal_task {
	struct k_thread thread;
	TASK_STACK_MEMBER
	osal_task_cfg_t taskcfg;
	osal_resrc_t *resrc;
};

/* The stack is the caller's (stack_addr) or, if enabled, the one embedded in the task object. */
static bool task_stack_get(osal_task_t *task, k_thread_stack_t **stack, size_t *size)
{
	const osal_task_cfg_t *cfg = &task->taskcfg;

	if (cfg->stack_addr != NULL) {
		*stack = cfg->stack_addr;
		*size = cfg->stack_size;
	} else {
		*stack = EMBEDDED_STACK(task);
		*size = cfg->stack_size != 0 ? cfg->stack_size : EMBEDDED_STACK_SIZE(task);
		if (cfg->stack_size > EMBEDDED_STACK_SIZE(task)) {
			return false;
		}
	}
	return *stack != NULL && *size != 0;
}

typedef struct {
	OSAL_RM_USEROBJMAN_DECLARE(struct osal_task, OSAL_TASK_NUM_MAX);
	bool init;
} task_man_t;

static task_man_t s_task_man;

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
	k_thread_stack_t *stack;
	size_t stack_size;
	uint16_t priority;

	if (!s_task_man.init || cfg == NULL || cfg->task_handler == NULL) {
		return NULL;
	}
	priority = cfg->priority != 0 ? cfg->priority : OSAL_TASK_PRIORITY_DEFAULT;
	if (priority >= CONFIG_NUM_PREEMPT_PRIORITIES) {
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

	if (!task_stack_get(task, &stack, &stack_size)) {
		osal_rm_free(&s_task_man.rm, resrc);
		return NULL;
	}

	k_tid_t tid = k_thread_create(&task->thread,
								  stack,
								  stack_size,
								  task_entry,
								  task,
								  NULL,
								  NULL,
								  K_PRIO_PREEMPT(priority),
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
