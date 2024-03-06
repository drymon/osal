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

#include <string.h>
#include <stdio.h>
#include "osal.h"
#define OSALOG_MODULE OSAL_LOG_MODULE_INDEX

static osal_mutex_t *s_shared_mutex;
static bool s_initialized;

static void log_output_default(char *logstr)
{
	printf("%s", logstr);
}

osal_error_t osal_init(osal_config_t *config)
{
	osal_error_t res;
	osal_log_output_t log_output = log_output_default;
	osal_log_level_t log_level = OSALOG_LEVEL_INFO;

	if (s_initialized) {
		return OSAL_E_OK;
	}

	if (config != NULL) {
		if (config->log_output != NULL) {
			log_output = config->log_output;
		}
		log_level = config->osal_level;
	}

	/* mutex must be init first since it is used in other osal modules */
	res = osal_mutex_init();
	OSAL_ASSERT(res == OSAL_E_OK);

	/* log initialization */
	res = osal_log_init(log_output);
	OSAL_ASSERT(res == OSAL_E_OK);

	/* log module initialization */
	res = osal_log_module_init(OSAL_LOG_MODULE_INDEX, "osal", log_level, false);
	OSAL_ASSERT(res == OSAL_E_OK);

	/* create a shared local mutex */
	OSAL_ASSERT(s_shared_mutex == NULL);
	s_shared_mutex = osal_mutex_create();
	OSAL_ASSERT(s_shared_mutex != NULL);

	/* semaphore initialization */
	res = osal_sem_init(s_shared_mutex);
	OSAL_ASSERT(res == OSAL_E_OK);

	/* task initialization */
	res = osal_task_init(s_shared_mutex);
	OSAL_ASSERT(res == OSAL_E_OK);

	/* timer initialization */
	res = osal_timer_init(s_shared_mutex);
	OSAL_ASSERT(res == OSAL_E_OK);

	/* queue initialization */
	res = osal_queue_init(s_shared_mutex);
	OSAL_ASSERT(res == OSAL_E_OK);

	/* initialization done */
	s_initialized = true;

	return OSAL_E_OK;
}

char *osal_version(void)
{
	return OSAL_VERSION;
}

void osal_print_resource(void)
{
	uint32_t use;
	uint32_t avail;

	use = osal_mutex_use();
	avail = osal_mutex_avail();
	OSALOG_INFO("osal: ---resource: <module>=used/total---\n");
	OSALOG_INFO("osal: mutex=%u/%u\n", use, use+avail);

	use = osal_sem_use();
	avail = osal_sem_avail();
	OSALOG_INFO("osal: semaphore=%u/%u\n", use, use+avail);

	use = osal_task_use();
	avail = osal_task_avail();
	OSALOG_INFO("osal: task=%u/%u\n", use, use+avail);

	use = osal_timer_use();
	avail = osal_timer_avail();
	OSALOG_INFO("osal: timer=%u/%u\n", use, use+avail);

	use = osal_queue_use();
	avail = osal_queue_avail();
	OSALOG_INFO("osal: queue=%u/%u\n", use, use+avail);
	OSALOG_INFO("osal: ---------------\n");
}

void osal_deinit(void)
{
	if (s_initialized == false) {
		return;
	}
	OSAL_ASSERT(s_shared_mutex != NULL);
	osal_mutex_delete(s_shared_mutex);
	s_shared_mutex = NULL;

	osal_sem_deinit();
	osal_task_deinit();
	osal_timer_deinit();
	osal_queue_deinit();
	osal_log_deinit();
	osal_mutex_deinit();
	s_initialized = false;
}
