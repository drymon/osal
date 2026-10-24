/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

#include <stdio.h>
#include <dmosal/osal.h>

#define OSALOG_MODULE 1

static void task_handler(void *arg)
{
	(void)arg;
	OSALOG_INFO("--task is started--\n");
}

int main(void)
{
	int res = -1;
	int err;
	osal_task_t *task = NULL;
	osal_task_cfg_t cfg = {
		.task_handler = &task_handler,
	};

	err = osal_init(NULL);
	if (err != OSAL_E_OK) {
		return -1;
	}

	err = osal_log_module_init(OSALOG_MODULE, "task", OSALOG_LEVEL_DEBUG, true);
	if (err != OSAL_E_OK) {
		return -1;
	}

	/* log can work from now */

	OSALOG_DEBUG("osal_log_module_init() OK\n");

	OSALOG_DEBUG("osal_init() OK\n");

	OSALOG_INFO("DMOSAL version: %s\n", osal_version());

	task = osal_task_create(&cfg);
	if (task == NULL) {
		OSALOG_ERROR("failed to create task\n");
		goto exit;
	}
	osal_task_delete(task);
	res = 0;

exit:
	OSALOG_DEBUG("osal_deinit\n");
	osal_deinit();
	return res;
}
