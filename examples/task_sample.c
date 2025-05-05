/* BSD 2-Clause License
*
* Copyright (c) 2023, nguyenvannam142@gmail.com
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
*    list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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
