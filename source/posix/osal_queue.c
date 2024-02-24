/* BSD 2-Clause License
*
* Copyright (c) 2024, nguyenvannam142@gmail.com
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

#include <string.h>
#include <time.h>
#include <mqueue.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <errno.h>
#include "osal_time.h"
#include "osal_queue.h"
#include "osal_assert.h"
#include "osal_rm.h"

struct osal_queue {
	osal_resrc_t *resrc;
	bool create;
	char name[OSAL_QUEUE_NAME_SIZE+1];
	uint32_t msglen;
	mqd_t fd;
};

typedef struct {
	OSAL_RM_USEROBJMAN_DECLARE(
		struct osal_queue,
		OSAL_QUEUE_NUM_MAX);
	bool init;
} queue_man_t;

static queue_man_t s_queue_man;

osal_error_t osal_queue_init(void)
{
	if (s_queue_man.init == true) {
		return OSAL_E_OK;
	}
	OSAL_RM_USEROBJMAN_INIT(&s_queue_man, OSAL_QUEUE_NUM_MAX, true, g_osal_shared_mutex);
	s_queue_man.init = true;

	return OSAL_E_OK;
}

void osal_queue_deinit(void)
{
	if (s_queue_man.init == false) {
		return;
	}
	osal_rm_deinit(&s_queue_man.rm);
	s_queue_man.init = false;
}

osal_queue_t *osal_queue_create(osal_queue_cfg_t *cfg)
{
	osal_queue_t *queue;
	osal_resrc_t *resrc;
	int res = 0;
	struct mq_attr attr;

	if (cfg == NULL) {
		return NULL;
	}
	if ((cfg->name[0] == 0) || (cfg->msglen == 0) || (cfg->qsize == 0)) {
		return NULL;
	}

	resrc = osal_rm_alloc(&s_queue_man.rm);
	if (resrc == NULL) {
		return NULL;
	}
	queue = resrc->data;
	OSAL_ASSERT(queue != NULL);
	memset(queue, 0, sizeof(osal_queue_t));
	queue->resrc = resrc;
	snprintf(queue->name, OSAL_QUEUE_NAME_SIZE+1, "/%s", cfg->name);
	queue->msglen = cfg->msglen;
	memset(&attr, 0, sizeof(attr));
	attr.mq_maxmsg = cfg->qsize;
	attr.mq_msgsize = cfg->msglen;
	/* Opening queue is very important and happen at the beginning.
	 * If we can not open the queue, we should terminate the program */
	res = mq_open(queue->name, O_CREAT|O_RDWR|O_EXCL|O_NONBLOCK, S_IRWXU, &attr);
	if (res < 0) {
		if (errno != EEXIST) {
			perror("1st mq_open()");
			printf("queue->name=%s\n", queue->name);
			OSAL_ASSERT(false);
		}
		res = mq_open(queue->name, O_RDWR);
		if (res < 0) {
			perror("2nd mq_open()");
			OSAL_ASSERT(false);
		}
		printf("Open existing queue: %s\n", queue->name);
	} else {
		printf("Open new queue: %s qsize=%d msglen=%d\n",
			   queue->name, cfg->qsize, cfg->msglen);
		queue->create = true;
	}
	queue->fd = res;

	return queue;
}

osal_error_t osal_queue_send(osal_queue_t *queue, uint8_t *msg, uint32_t msglen)
{
	int res;

	if ((queue == NULL) || (queue->fd <= 0) || (msg == NULL) ||
		(msglen == 0) || (msglen > queue->msglen)) {
		return OSAL_E_PARAM;
	}
	res = mq_send(queue->fd, (const char *)msg, msglen, 0);
	if (res < 0) {
		if (errno == EAGAIN) {
			return OSAL_E_QFULL;
		}
		perror("mq_send");
		return OSAL_E_OSCALL;
	}
	return OSAL_E_OK;
}

osal_error_t osal_queue_recv(osal_queue_t *queue, uint8_t *buf,
							 uint32_t bufsize, uint32_t timeout_usec)
{
	int res;
	struct timeval tvtout;
	fd_set rfds;

	if ((queue == NULL) || (queue->fd <= 0) ||
		(buf == NULL) || (bufsize == 0)) {
		return OSAL_E_PARAM;
	}

	FD_ZERO(&rfds);
	FD_SET(queue->fd, &rfds);
	memset(&tvtout, 0, sizeof(tvtout));
	tvtout.tv_sec = timeout_usec / OSAL_SEC_USEC;
	tvtout.tv_usec = timeout_usec % OSAL_SEC_USEC;

	res = select(queue->fd+1, &rfds, NULL, NULL, &tvtout);
	if (res < 0) {
		perror("select");
		return OSAL_E_OSCALL;
	} else if (res == 0) {
		return OSAL_E_TIMEOUT;
	}
	if (FD_ISSET(queue->fd, &rfds) == false) {
		return OSAL_E_QEMPTY;
	}
	res = mq_receive(queue->fd, buf, bufsize, NULL);
	if (res < 0) {
		perror("mq_receive");
		return OSAL_E_OSCALL;
	}

	return OSAL_E_OK;
}

void osal_queue_delete(osal_queue_t *queue)
{
	if (queue == NULL) {
		return;
	}
	if (queue->fd > 0) {
		mq_close(queue->fd);
	}
	if (queue->create) {
		mq_unlink(queue->name);
	}
	memset(queue, 0, sizeof(osal_queue_t));
	osal_rm_free(&s_queue_man.rm, queue->resrc);
}

uint32_t osal_queue_use(void)
{
	if (s_queue_man.init == false) {
		return 0;
	}
	return osal_rm_use(&s_queue_man.rm);
}

uint32_t osal_queue_avail(void)
{
	if (s_queue_man.init == false) {
		return 0;
	}
	return osal_rm_avail(&s_queue_man.rm);
}
