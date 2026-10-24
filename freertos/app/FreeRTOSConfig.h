/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

/* FreeRTOS configuration for running dmosal on the FreeRTOS POSIX/Linux simulator port.
 * By default everything is statically allocated: the kernel does no heap allocation.
 * With DMOSAL_APP_DYNAMIC_ALLOC (cmake -DDYNAMIC_ALLOC=ON) the kernel's dynamic API is used
 * instead, to check that dmosal works with either kind of vendor configuration. */
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#define configUSE_PREEMPTION					1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_TIME_SLICING					1
#define configUSE_IDLE_HOOK						0
#define configUSE_TICK_HOOK						0
#define configTICK_RATE_HZ						1000
#define configMAX_PRIORITIES					7
/* Words (not bytes) of the idle task stack; the port turns it into a pthread stack. */
#define configMINIMAL_STACK_SIZE 4096
#define configMAX_TASK_NAME_LEN	 32
#define configUSE_16_BIT_TICKS	 0
#define configIDLE_SHOULD_YIELD	 1

#ifdef DMOSAL_APP_DYNAMIC_ALLOC
#define configSUPPORT_STATIC_ALLOCATION	 0
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#define configTOTAL_HEAP_SIZE			 (8U * 1024U * 1024U)
#else
#define configSUPPORT_STATIC_ALLOCATION		1
#define configSUPPORT_DYNAMIC_ALLOCATION	0
#define configKERNEL_PROVIDED_STATIC_MEMORY 0
#endif

#define configUSE_MUTEXES			  1
#define configUSE_RECURSIVE_MUTEXES	  0
#define configUSE_COUNTING_SEMAPHORES 1
#define configQUEUE_REGISTRY_SIZE	  0
#define configUSE_QUEUE_SETS		  0
#define configUSE_TASK_NOTIFICATIONS  1

/* Callbacks of osal timers run in the timer service task: give it a priority above every
 * other task (see source/freertos/osal_timer.c) and a stack that fits user callbacks. */
#define configUSE_TIMERS			 1
#define configTIMER_TASK_PRIORITY	 (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH	 16
#define configTIMER_TASK_STACK_DEPTH 8192

#define INCLUDE_vTaskDelete				  1
#define INCLUDE_vTaskSuspend			  1
#define INCLUDE_vTaskDelay				  1
#define INCLUDE_xTaskGetCurrentTaskHandle 1
#define INCLUDE_xTimerPendFunctionCall	  1

extern void vAssertCalled(const char *file, int line);
#define configASSERT(x)                                                                            \
	do {                                                                                           \
		if (!(x))                                                                                  \
			vAssertCalled(__FILE__, __LINE__);                                                     \
	} while (0)

#endif /* FREERTOS_CONFIG_H */
