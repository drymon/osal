/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

/* Runs a dmosal example or cmocka test on the FreeRTOS POSIX simulator.
 *
 * The example/test keeps its own main(): the build renames it to dmosal_app_main, and this file
 * runs it in a FreeRTOS task, then ends the process with its result so CI can use the exit
 * code. With static allocation (default) the idle and timer tasks get their memory from here;
 * with DMOSAL_APP_DYNAMIC_ALLOC the kernel allocates everything from the heap. */
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"

extern int dmosal_app_main(void);

/* Bytes; the test and example code runs here, so keep room for cmocka and local buffers. */
#define APP_STACK_SIZE (256U * 1024U)
#define APP_PRIORITY   (tskIDLE_PRIORITY + 2) /* above the tasks created by dmosal */

#ifndef DMOSAL_APP_DYNAMIC_ALLOC
static StaticTask_t s_app_tcb;
static StackType_t s_app_stack[APP_STACK_SIZE / sizeof(StackType_t)];
#endif

static void app_task(void *arg)
{
	int rc;

	(void)arg;
	rc = dmosal_app_main();
	fflush(stdout);
	exit(rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}

int main(void)
{
#ifdef DMOSAL_APP_DYNAMIC_ALLOC
	xTaskCreate(app_task, "app", APP_STACK_SIZE / sizeof(StackType_t), NULL, APP_PRIORITY, NULL);
#else
	xTaskCreateStatic(app_task,
					  "app",
					  sizeof(s_app_stack) / sizeof(s_app_stack[0]),
					  NULL,
					  APP_PRIORITY,
					  s_app_stack,
					  &s_app_tcb);
#endif
	vTaskStartScheduler();
	return EXIT_FAILURE;
}

void vAssertCalled(const char *file, int line)
{
	fprintf(stderr, "FreeRTOS assert failed: %s:%d\n", file, line);
	abort();
}

#ifndef DMOSAL_APP_DYNAMIC_ALLOC
void vApplicationGetIdleTaskMemory(StaticTask_t **tcb,
								   StackType_t **stack,
								   configSTACK_DEPTH_TYPE *depth)
{
	static StaticTask_t s_tcb;
	static StackType_t s_stack[configMINIMAL_STACK_SIZE];

	*tcb = &s_tcb;
	*stack = s_stack;
	*depth = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(StaticTask_t **tcb,
									StackType_t **stack,
									configSTACK_DEPTH_TYPE *depth)
{
	static StaticTask_t s_tcb;
	static StackType_t s_stack[configTIMER_TASK_STACK_DEPTH];

	*tcb = &s_tcb;
	*stack = s_stack;
	*depth = configTIMER_TASK_STACK_DEPTH;
}
#endif
