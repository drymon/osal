/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

/* Zephyr backend (native kernel API). */

#include <stddef.h>
#include <zephyr/kernel.h>
#include "osal_time.h"

osal_error_t osal_sleep(uint32_t sec)
{
	k_sleep(K_SECONDS(sec));
	return OSAL_E_OK;
}

osal_error_t osal_usleep(uint32_t microsec)
{
	k_usleep((int32_t)microsec);
	return OSAL_E_OK;
}

osal_error_t osal_clock_time(uint64_t *nsec)
{
	if (nsec == NULL) {
		return OSAL_E_PARAM;
	}

	/* Uptime is 0 right after boot (always so on native_sim before time advances); callers
	 * such as tmcheck use 0 as "not set", so never return it. */
	*nsec = k_ticks_to_ns_floor64(k_uptime_ticks()) + 1U;
	return OSAL_E_OK;
}
