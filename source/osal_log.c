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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "osal_time.h"
#include "osal_log.h"

typedef struct {
	char name[OSAL_LOG_MODULE_NAME_SIZE];
	osal_log_level_t log_level;
	bool ts;
	bool inuse;
} osal_log_module_t;

static osal_log_module_t s_log_modules[OSAL_LOG_MODULE_NUM_MAX];
static osal_log_output_t s_log_output;
static const char *s_loglevel_strs[OSALOG_LEVEL_MAX] = {
	[OSALOG_LEVEL_FATAL] = "fatal",
	[OSALOG_LEVEL_ERROR] = "error",
	[OSALOG_LEVEL_WARN] = "warn",
	[OSALOG_LEVEL_INFO] = "info",
	[OSALOG_LEVEL_DEBUG] = "debug",
	[OSALOG_LEVEL_TRACE] = "trace"
};

osal_error_t osal_log_init(osal_log_output_t log_output)
{
	if (s_log_output != NULL) {
		return OSAL_E_OK;
	}
	if (log_output == NULL) {
		return OSAL_E_PARAM;
	}
	s_log_output = log_output;
	return OSAL_E_OK;
}

void osal_log_deinit(void)
{
	s_log_output = NULL;
	memset(s_log_modules, 0, sizeof(s_log_modules));
}

osal_error_t osal_log_print(uint32_t index, bool ts,
				osal_log_level_t level, const char *format, ...)
{
	va_list ap;
	char logstr[OSAL_LOG_STRING_SIZE] = {0};
	osal_log_module_t *module;
	int len;

	if (index >= OSAL_LOG_MODULE_NUM_MAX) {
		return OSAL_E_PARAM;
	}

	module = &s_log_modules[index];
	if ((s_log_output == NULL) || (module->inuse == false)) {
		return OSAL_E_NOINIT;
	}

	if(level > module->log_level) {
		return OSAL_E_OK;
	}

	/* add timestamp */
	if((ts == true) || (module->ts == true)) {
		uint64_t now = 0;
		uint32_t sec_low, usec;

		osal_clock_time(&now);
		sec_low = (now / OSAL_SEC_NSEC) % 100000;
		usec = (now % OSAL_SEC_NSEC) / OSAL_USEC_NSEC;

		snprintf(logstr, OSAL_LOG_STRING_SIZE, "%05u-%06u|", sec_low, usec);
	}

	/* add log level string */
	len = strlen(logstr);
	snprintf(&logstr[len], OSAL_LOG_STRING_SIZE-len, "%-5s|", s_loglevel_strs[level]);

	/* add module name */
	if (module->name[0] != 0) {
		len = strlen(logstr);
		snprintf(&logstr[len], OSAL_LOG_STRING_SIZE-len, "%s|", module->name);
	}

	/* add user log */
	len = strlen(logstr);
	va_start(ap, format);
	vsnprintf(&logstr[len], OSAL_LOG_STRING_SIZE-len, format, ap);
	va_end(ap);

	/* print out */
	s_log_output(logstr);

	return OSAL_E_OK;
}

osal_error_t osal_log_module_init(uint32_t index, char *name,
								  osal_log_level_t level, bool ts)
{
	osal_log_module_t *module;

	if ((level >= OSALOG_LEVEL_MAX) || (level < 0) ||
		(index >= OSAL_LOG_MODULE_NUM_MAX)) {
		return OSAL_E_PARAM;
	}
	module = &s_log_modules[index];
	if (module->inuse) {
		return OSAL_E_INUSE;
	}
	module->log_level = level;
	module->ts = ts;
	if (name != NULL) {
		snprintf(module->name, OSAL_LOG_MODULE_NAME_SIZE, "%s", name);
	}
	module->inuse = true;
	return OSAL_E_OK;
}

osal_error_t osal_log_module_change(uint32_t index, osal_log_level_t level)
{
	osal_log_module_t *module;

	if ((level >= OSALOG_LEVEL_MAX) || (level < 0) ||
		(index >= OSAL_LOG_MODULE_NUM_MAX)) {
		return OSAL_E_PARAM;
	}

	module = &s_log_modules[index];
	if ((s_log_output == NULL) || (module->inuse == false)) {
		return OSAL_E_NOINIT;
	}

	module->log_level = level;
	return OSAL_E_OK;
}
