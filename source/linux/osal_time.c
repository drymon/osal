/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "osal_time.h"

osal_error_t osal_sleep(uint32_t sec)
{
	sleep(sec);
	return OSAL_E_OK;
}

osal_error_t osal_usleep(uint32_t microsec)
{
	usleep(microsec);
	return OSAL_E_OK;
}

osal_error_t osal_clock_time(uint64_t *nsec)
{
	struct timespec nowts;

	if (nsec == NULL) {
		return OSAL_E_PARAM;
	}

	if (clock_gettime(CLOCK_MONOTONIC, &nowts) < 0) {
		perror("clock_gettime");
		return OSAL_E_OSCALL;
	}
	*nsec = ((uint64_t)nowts.tv_sec * OSAL_SEC_NSEC + (uint64_t)nowts.tv_nsec);
	return OSAL_E_OK;
}
