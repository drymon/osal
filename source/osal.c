/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */
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

static void teardown(void)
{
	/* Tear down subsystems that use the shared mutex FIRST, their
	 * osal_rm_deinit locks the shared mutex. Deleting the shared mutex
	 * before this point would leave those locks operating on a destroyed
	 * mutex.
	 */
	osal_sem_deinit();
	osal_task_deinit();
	osal_timer_deinit();
	osal_queue_deinit();
	osal_tmcheck_deinit();

	/* Log is independent of the shared mutex. */
	osal_log_deinit();

	/* Now safe to destroy the shared mutex and tear down the mutex pool. */
	if (s_shared_mutex != NULL) {
		osal_mutex_delete(s_shared_mutex);
		s_shared_mutex = NULL;
	}
	osal_mutex_deinit();
}

/* Runs one init step and unwinds everything on failure. */
#define INIT_STEP(call)                                                                            \
	do {                                                                                           \
		res = (call);                                                                              \
		if (res != OSAL_E_OK) {                                                                    \
			goto fail;                                                                             \
		}                                                                                          \
	} while (0)

osal_error_t osal_init(const osal_config_t *config)
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
	INIT_STEP(osal_mutex_init());

	/* log initialization */
	INIT_STEP(osal_log_init(log_output));

	/* log module initialization */
	INIT_STEP(osal_log_module_init(OSAL_LOG_MODULE_INDEX, "osal", log_level, false));

	/* create a shared local mutex */
	OSAL_RUNTIME_ASSERT(s_shared_mutex == NULL);
	s_shared_mutex = osal_mutex_create();
	if (s_shared_mutex == NULL) {
		res = OSAL_E_RESRC;
		goto fail;
	}

	/* semaphore initialization */
	INIT_STEP(osal_sem_init(s_shared_mutex));

	/* task initialization */
	INIT_STEP(osal_task_init(s_shared_mutex));

	/* timer initialization */
	INIT_STEP(osal_timer_init(s_shared_mutex));

	/* queue initialization */
	INIT_STEP(osal_queue_init(s_shared_mutex));

	INIT_STEP(osal_tmcheck_init(s_shared_mutex));

	/* initialization done */
	s_initialized = true;

	return OSAL_E_OK;

fail:
	/* Every deinit ignores a subsystem that was not initialized. */
	teardown();
	return res;
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
	OSALOG_INFO("osal: mutex=%u/%u\n", use, use + avail);

	use = osal_sem_use();
	avail = osal_sem_avail();
	OSALOG_INFO("osal: semaphore=%u/%u\n", use, use + avail);

	use = osal_task_use();
	avail = osal_task_avail();
	OSALOG_INFO("osal: task=%u/%u\n", use, use + avail);

	use = osal_timer_use();
	avail = osal_timer_avail();
	OSALOG_INFO("osal: timer=%u/%u\n", use, use + avail);

	use = osal_queue_use();
	avail = osal_queue_avail();
	OSALOG_INFO("osal: queue=%u/%u\n", use, use + avail);

	use = osal_tmcheck_use();
	avail = osal_tmcheck_avail();
	OSALOG_INFO("osal: tmcheck=%u/%u\n", use, use + avail);

	OSALOG_INFO("osal: ---------------\n");
}

void osal_deinit(void)
{
	if (s_initialized == false) {
		return;
	}
	teardown();
	s_initialized = false;
}
