/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

/* FreeRTOS backend (static allocation only). */

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "osal_task.h"
#include "osal_rm.h"
#include "osal_assert.h"
#include "osal_config.h"
#include "osal_freertos_priv.h"

/* All build-mode differences (static/dynamic allocation, embedded stack) live in this block. */
#if DMOSAL_FREERTOS_STATIC && OSAL_TASK_STACK_EMBEDDED
#define EMBEDDED_STACK_WORDS (OSAL_TASK_STACK_SIZE_DEFAULT / sizeof(StackType_t))
#define TASK_STACK_MEMBER	 StackType_t stack[EMBEDDED_STACK_WORDS];
#define EMBEDDED_STACK(t)	 ((t)->stack)
#else
#define EMBEDDED_STACK_WORDS 0
#define TASK_STACK_MEMBER
#define EMBEDDED_STACK(t) ((StackType_t *)NULL)
#endif

#if DMOSAL_FREERTOS_STATIC
#define TASK_KERNEL_MEMBER StaticTask_t tcb;
#else
#define TASK_KERNEL_MEMBER
#endif

struct osal_task {
	TASK_KERNEL_MEMBER
	TASK_STACK_MEMBER
	TaskHandle_t handle;
	osal_task_cfg_t taskcfg;
	osal_resrc_t *resrc;
};

static void task_entry(void *arg)
{
	osal_task_t *task = arg;

	/* Set before the handler runs: the creator may not have stored the handle yet. */
	task->handle = xTaskGetCurrentTaskHandle();
	task->taskcfg.task_handler(task->taskcfg.task_arg);

	/* A FreeRTOS task must not return, and a self-deleted task keeps its TCB until the idle
	 * task cleans it up. Park instead, so osal_task_delete() can always remove it safely. */
	for (;;) {
		vTaskSuspend(NULL);
	}
}

#if DMOSAL_FREERTOS_STATIC
/* The stack is the caller's (stack_addr) or, if enabled, the one embedded in the task object. */
static bool task_kernel_create(osal_task_t *task, const char *name, UBaseType_t priority)
{
	const osal_task_cfg_t *cfg = &task->taskcfg;
	StackType_t *stack = cfg->stack_addr;
	uint32_t words;

	if (stack != NULL) {
		if (cfg->stack_size == 0 || ((uintptr_t)stack % portBYTE_ALIGNMENT) != 0) {
			return false;
		}
		words = cfg->stack_size / (uint32_t)sizeof(StackType_t);
	} else {
		stack = EMBEDDED_STACK(task);
		words = cfg->stack_size != 0 ? cfg->stack_size / (uint32_t)sizeof(StackType_t)
									 : EMBEDDED_STACK_WORDS;
		if (stack == NULL || words > EMBEDDED_STACK_WORDS) {
			return false;
		}
	}
	if (words == 0) {
		return false;
	}
	task->handle = xTaskCreateStatic(task_entry, name, words, task, priority, stack, &task->tcb);
	return task->handle != NULL;
}
#else
/* The kernel allocates the stack from its heap, so stack_addr is not used. */
static bool task_kernel_create(osal_task_t *task, const char *name, UBaseType_t priority)
{
	uint32_t size =
		task->taskcfg.stack_size != 0 ? task->taskcfg.stack_size : OSAL_TASK_STACK_SIZE_DEFAULT;
	uint32_t words = size / (uint32_t)sizeof(StackType_t);

	if (words == 0 ||
		xTaskCreate(task_entry, name, words, task, priority, &task->handle) != pdPASS) {
		task->handle = NULL;
	}
	return task->handle != NULL;
}
#endif

typedef struct {
	OSAL_RM_USEROBJMAN_DECLARE(struct osal_task, OSAL_TASK_NUM_MAX);
	bool init;
} task_man_t;

static task_man_t s_task_man;

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
	UBaseType_t priority;

	if (!s_task_man.init || cfg == NULL || cfg->task_handler == NULL) {
		return NULL;
	}
	priority = cfg->priority != 0 ? cfg->priority : OSAL_TASK_PRIORITY_DEFAULT;
	if (priority >= configMAX_PRIORITIES) {
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

	const char *name = task->taskcfg.name[0] != '\0' ? (const char *)task->taskcfg.name : "osal";
	if (!task_kernel_create(task, name, priority)) {
		osal_rm_free(&s_task_man.rm, resrc);
		return NULL;
	}
	return task;
}

void osal_task_delete(osal_task_t *task)
{
	if (task == NULL) {
		return;
	}

	/* A task cannot delete itself: its TCB is only released by the idle task, so the slot
	 * must not be reused yet. */
	if (task->handle == xTaskGetCurrentTaskHandle()) {
		return;
	}

	vTaskDelete(task->handle);
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
