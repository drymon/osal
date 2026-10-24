/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

/* FreeRTOS backend. Time has tick resolution (1 / configTICK_RATE_HZ). */

#include <stddef.h>
#include "osal_time.h"
#include "osal_freertos_priv.h"

_Static_assert(sizeof(TickType_t) >= 4, "16-bit ticks are not supported");

osal_error_t osal_sleep(uint32_t sec)
{
	/* Sleep one second at a time so the tick count cannot overflow. */
	while (sec-- > 0) {
		vTaskDelay(osal_freertos_usec_to_ticks(1000000U));
	}
	return OSAL_E_OK;
}

osal_error_t osal_usleep(uint32_t microsec)
{
	TickType_t ticks = osal_freertos_usec_to_ticks(microsec);

	if (ticks == 0) {
		taskYIELD();
	} else {
		vTaskDelay(ticks);
	}
	return OSAL_E_OK;
}

osal_error_t osal_clock_time(uint64_t *nsec)
{
	static TickType_t s_last_ticks;
	static uint64_t s_wraps;
	uint64_t ticks;

	if (nsec == NULL) {
		return OSAL_E_PARAM;
	}

	/* The tick count is 32 bits wide: extend it to 64 bits by counting the wrap-arounds. */
	taskENTER_CRITICAL();
	{
		TickType_t now = xTaskGetTickCount();

		if (now < s_last_ticks) {
			s_wraps++;
		}
		s_last_ticks = now;
		ticks = sizeof(TickType_t) < sizeof(uint64_t) ? (s_wraps << 32) | now : now;
	}
	taskEXIT_CRITICAL();

	/* Callers such as tmcheck use 0 as "not set"; the tick count is 0 right after boot. */
	*nsec = ticks * (1000000000ULL / configTICK_RATE_HZ) + 1U;
	return OSAL_E_OK;
}
