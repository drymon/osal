/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

/* Private helpers and tunables of the FreeRTOS backend (not a public header).
 *
 * The backend uses only the FreeRTOS user-space API and adapts to the kernel
 * configuration supplied by the SoC vendor or application:
 *  - configSUPPORT_STATIC_ALLOCATION = 1: every kernel object lives in the
 *    static pools, no heap is used. The application then provides the
 *    kernel's own idle/timer task memory hooks.
 *  - otherwise the kernel's dynamic API is used (configSUPPORT_DYNAMIC_ALLOCATION
 *    and a heap implementation are required). The pools stay static, only the
 *    kernel objects come from the heap.
 * Always required: configUSE_MUTEXES, configUSE_COUNTING_SEMAPHORES, configUSE_TIMERS,
 * INCLUDE_vTaskDelete, INCLUDE_vTaskSuspend, INCLUDE_xTaskGetCurrentTaskHandle and
 * INCLUDE_xTimerPendFunctionCall.
 */

#ifndef OSAL_FREERTOS_PRIV_H
#define OSAL_FREERTOS_PRIV_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"

#if !configUSE_MUTEXES || !configUSE_COUNTING_SEMAPHORES || !configUSE_TIMERS
#error "dmosal needs configUSE_MUTEXES, configUSE_COUNTING_SEMAPHORES and configUSE_TIMERS"
#endif
#if !INCLUDE_vTaskDelete || !INCLUDE_vTaskSuspend || !INCLUDE_xTaskGetCurrentTaskHandle ||         \
	!INCLUDE_xTimerPendFunctionCall
#error                                                                                             \
	"dmosal needs INCLUDE_vTaskDelete, _vTaskSuspend, _xTaskGetCurrentTaskHandle, _xTimerPendFunctionCall"
#endif

/* Prefer the static API when the kernel has it, else fall back to the heap. */
#if configSUPPORT_STATIC_ALLOCATION == 1
#define DMOSAL_FREERTOS_STATIC 1
#elif configSUPPORT_DYNAMIC_ALLOCATION == 1
#define DMOSAL_FREERTOS_STATIC 0
#else
#error "dmosal needs configSUPPORT_STATIC_ALLOCATION or configSUPPORT_DYNAMIC_ALLOCATION"
#endif

/* Microseconds to ticks, rounded up so a non-zero timeout never becomes "no wait". */
static inline TickType_t osal_freertos_usec_to_ticks(uint32_t usec)
{
	uint64_t ticks = ((uint64_t)usec * configTICK_RATE_HZ + 999999U) / 1000000U;

	/* portMAX_DELAY means "forever" and must not be produced by a finite timeout. */
	return ticks >= portMAX_DELAY ? portMAX_DELAY - 1U : (TickType_t)ticks;
}

#endif /* OSAL_FREERTOS_PRIV_H */
