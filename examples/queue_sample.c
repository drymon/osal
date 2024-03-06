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
#include <stdbool.h>
#include <string.h>
#include <dmosal/osal.h>
#include <dmosal/osal_task.h>
#include <dmosal/osal_queue.h>
#include <dmosal/osal_time.h>
#define OSALOG_MODULE 2

#define QUEUE_NAME "example_queue"
#define QUEUE_SIZE 10
#define MSG_LEN 128

static void task_send(void *arg)
{
	osal_queue_t *queue = arg;
	int res;
	char msg[MSG_LEN] = {0};
	int i;

	OSALOG_INFO("--task send is started--\n");
	for (i = 10; i > 0; i--) {
		memset(msg, 0, sizeof(msg));
		snprintf(msg, MSG_LEN, "hello main %d", i);
		res = osal_queue_send(queue, (uint8_t *)msg, strlen(msg));
		if (res != OSAL_E_OK) {
			OSALOG_ERROR("Send error: %s\n", osal_errstr(res));
		} else {
			OSALOG_TRACE("Task sent: %s\n", msg);
		}
	}
}

int main(void)
{
	int res = -1;
	int err;
	osal_task_t *task = NULL;
	osal_queue_t *queue = NULL;
	osal_log_module_t log_mod;
	osal_task_cfg_t taskcfg = {
		.task_handler = &task_send,
	};
	osal_queue_cfg_t queuecfg = {
		.name = QUEUE_NAME,
		.qsize = QUEUE_SIZE,
		.msglen = MSG_LEN,
	};
	char msg[MSG_LEN] = {0};

	err = osal_init(NULL);
	if (err != OSAL_E_OK) {
		return -1;
	}

	memset(&log_mod, 0, sizeof(log_mod));
	snprintf(log_mod.name, OSAL_LOG_MODULE_NAME_SIZE, "queue");
	log_mod.enable_ts = true;
	log_mod.log_level = OSALOG_LEVEL_TRACE;
	log_mod.module_index = OSALOG_MODULE;

	err = osal_log_init_module(&log_mod);
	if (err != OSAL_E_OK) {
		return -1;
	}

	/* log can work from now */

	OSALOG_INFO("osal_init() OK\n");

	OSALOG_INFO("DMOSAL version: %s\n", osal_version());

	queue = osal_queue_create(&queuecfg);
	if (queue == NULL) {
		OSALOG_ERROR("failed to create queue\n");
		goto exit;
	}
	OSALOG_INFO("osal_queue_create() OK\n");

	taskcfg.task_arg = queue;
	task = osal_task_create(&taskcfg);
	if (task == NULL) {
		OSALOG_ERROR("failed to create task\n");
		goto exit;
	}
	osal_print_resource();

	OSALOG_DEBUG("main() waiting message...\n");
	while (true) {
		memset(msg, 0, sizeof(msg));
		res = osal_queue_recv(queue, (uint8_t *)msg, sizeof(msg), OSAL_SEC_USEC);
		if (res == OSAL_E_OK) {
			OSALOG_TRACE("main() receive: %s\n", msg);
		} else if ((res == OSAL_E_TIMEOUT) || (res == OSAL_E_QEMPTY)) {
			OSALOG_ERROR("main(): %s\n", osal_errstr(res));
			break;
		} else {
			OSALOG_ERROR("failed to recv: %s\n", osal_errstr(res));
			goto exit;
		}
	}

	res = 0;

exit:
	if (task) {
		osal_task_delete(task);
	}
	if (queue) {
		osal_queue_delete(queue);
	}
	OSALOG_INFO("osal_deinit\n");
	osal_deinit();
	return res;
}
