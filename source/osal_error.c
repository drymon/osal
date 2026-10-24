/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

#include <stddef.h>
#include "osal_error.h"

#define OSAL_E(e) [OSAL_E_##e] = "OSAL_E_" #e

static const char *s_osal_errstr[OSAL_E_MAX] = {
	OSAL_E(OK),
	OSAL_E(PARAM),
	OSAL_E(NOINIT),
	OSAL_E(RESRC),
	OSAL_E(FAILURE),
	OSAL_E(OSCALL),
	OSAL_E(TIMEOUT),
	OSAL_E(QFULL),
	OSAL_E(QEMPTY),
	OSAL_E(INUSE),
};

const char *osal_errstr(osal_error_t e)
{
	if ((e < OSAL_E_OK) || (e >= OSAL_E_MAX)) {
		return "OSAL_UNKNOWN";
	}
	return s_osal_errstr[e];
}
