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

#define QUEUE_NAME "example_queue"
#define QUEUE_SIZE 16
#define MSG_LEN 128

static void task_send(void *arg)
{
	osal_queue_t *queue = arg;
	int res;
	char msg[MSG_LEN] = {0};
	int i;

	printf("--task send is started--\n");
	for (i = 10; i > 0; i--) {
		memset(msg, 0, sizeof(msg));
		snprintf(msg, MSG_LEN, "hello main %d", i);
		res = osal_queue_send(queue, msg, strlen(msg));
		if (res != OSAL_E_OK) {
			printf("Send error: %s\n", osal_errstr(res));
		} else {
			printf("Task sent: %s\n", msg);
		}
	}
}

int main(void)
{
	int res = -1;
	int err;
	osal_task_t *task = NULL;
	osal_queue_t *queue = NULL;
	osal_task_cfg_t taskcfg = {
		.task_handler = &task_send,
	};
	osal_queue_cfg_t queuecfg = {
		.name = QUEUE_NAME,
		.qsize = QUEUE_SIZE,
		.msglen = MSG_LEN,
	};
	char msg[MSG_LEN] = {0};

	printf("DMOSAL version: %s\n", osal_version());

	err = osal_init();
	if (err != OSAL_E_OK) {
		printf("failed to init osal\n");
		return -1;
	}
	printf("osal_init() OK\n");

	queue = osal_queue_create(&queuecfg);
	if (queue == NULL) {
		printf("failed to create queue\n");
		goto exit;
	}
	printf("osal_queue_create() OK\n");

	taskcfg.task_arg = queue;
	task = osal_task_create(&taskcfg);
	if (task == NULL) {
		printf("failed to create task\n");
		goto exit;
	}

	printf("main() waiting message...\n");
	while (true) {
		memset(msg, 0, sizeof(msg));
		res = osal_queue_recv(queue, msg, sizeof(msg), OSAL_SEC_USEC);
		if (res == OSAL_E_OK) {
			printf("main() receive: %s\n", msg);
		} else if ((res == OSAL_E_TIMEOUT) || (res == OSAL_E_QEMPTY)) {
			printf("main(): %s\n", osal_errstr(res));
			break;
		} else {
			printf("failed to recv: %s\n", osal_errstr(res));
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
	osal_deinit();
	printf("osal_deinit\n");
	return res;
}
