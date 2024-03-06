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

static osal_log_module_t s_log_modules[OSAL_LOG_MODULE_NUM_MAX];
static osal_log_output_t s_log_output;
static const char *s_log_level_strings[OSALOG_LEVEL_MAX] = {
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

osal_error_t osal_log_print(uint32_t module_index, bool ts,
				osal_log_level_t level, const char *format, ...)
{
	va_list ap;
	char logstr[OSAL_LOG_STRING_SIZE];
	osal_log_module_t *manage_module;
	int len;

	if (module_index >= OSAL_LOG_MODULE_NUM_MAX) {
		return OSAL_E_PARAM;
	}
	manage_module = &s_log_modules[module_index];
	if ((s_log_output == NULL) || (manage_module->inuse == false)) {
		return OSAL_E_NOINIT;
	}

	if(level > manage_module->log_level) {
		return OSAL_E_OK;
	}

	if((ts == true) || (manage_module->enable_ts)) {
		uint64_t now = 0;
		uint32_t sec_scale, usec;

		osal_clock_time(&now);
		sec_scale = (now / OSAL_SEC_NSEC) % 100000;
		usec = (now % OSAL_SEC_NSEC) / OSAL_USEC_NSEC;
		snprintf(logstr, OSAL_LOG_STRING_SIZE, "|%5s|%05u-%06u|%s|",
				 s_log_level_strings[level],
				 sec_scale , usec, manage_module->name);
	} else {
		snprintf(logstr, OSAL_LOG_STRING_SIZE, "|%5s|%s|",
				 s_log_level_strings[level], manage_module->name);
	}
	len = strlen(logstr);
	va_start(ap, format);
	vsnprintf(&logstr[len], OSAL_LOG_STRING_SIZE-len, format, ap);
	va_end(ap);

	s_log_output(logstr);

	return OSAL_E_OK;
}

osal_error_t osal_log_init_module(osal_log_module_t *module)
{
	osal_log_module_t *manage_module;

	if ((module == NULL) || (module->log_level >= OSALOG_LEVEL_MAX) ||
		(module->log_level < 0) || (module->module_index >= OSAL_LOG_MODULE_NUM_MAX)) {
		return OSAL_E_PARAM;
	}
	manage_module = &s_log_modules[module->module_index];
	if (manage_module->inuse) {
		return OSAL_E_INUSE;
	}
	memcpy(manage_module, module, sizeof(osal_log_module_t));
	if (manage_module->name[0] == 0) {
		snprintf(manage_module->name, OSAL_LOG_MODULE_NAME_SIZE, "unkown");
	}
	manage_module->inuse = true;
	return OSAL_E_OK;
}
