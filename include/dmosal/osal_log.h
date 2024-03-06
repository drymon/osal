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

/**
 * @addtogroup dmosal
 * @{
 * @file osal_log.h
 * @brief OS Abstraction Layer Logging Definitions
 * @copyright Copyright (c) 2024, nguyenvannam142@gmail.com
 * @author Nam Nguyen Van(nguyenvannam142@gmail.com)
 */
#ifndef OSAL_LOG_H
#define OSAL_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "osal_error.h"
#include "osal_config.h"


/**
 * @brief The log module index reserved for the OSAL module.
 * The application should use the index starts from 1.
 */
#define OSAL_LOG_MODULE_INDEX 0

/**
 * @brief Default logging build level configuration
 *
 * The macro defines the default logging build level for the library.
 * If not explicitly defined, it is set to the trace level (`OSALOG_LEVEL_TRACE`).
 * The build level determines the verbosity of logging messages during compilation.
 * Developers can modify this macro to adjust the logging granularity as needed.
 *
 * @note To override the default build level, define `OSALOG_BUILD_LEVEL`
 * before including the log headers.
 */
#ifndef OSALOG_BUILD_LEVEL
#define OSALOG_BUILD_LEVEL OSALOG_LEVEL_TRACE
#endif

/**
 * @brief Macro to extract the source file name from the full path.
 */
#define OSALOG_SRCFILE (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)

/**
 * @brief Macro to provide a generic logging helper function.
 * User needs to define OSALOG_MODULE at the begining of the source file.
 *
 * @param level The log level (FATAL, ERROR, WARN, INFO, DEBUG, TRACE).
 * @param ts Flag indicating whether to include timestamp in the log.
 * @param args The format string for the log message.
 * @param ... Additional arguments for the log message.
 */
#define OSALOG_HELPER(level, ts, args, ...) \
	osal_log_print(OSALOG_MODULE, ts, OSALOG_LEVEL_##level, \
				   "%s:%d|"args, OSALOG_SRCFILE, __LINE__, ##__VA_ARGS__)

/**
 * @brief Macro to check if the specified log level is enabled during compilation.
 *
 * @param level The log level to check.
 */
#define OSALOG_IS_BUILT(level) (OSALOG_BUILD_LEVEL >= OSALOG_LEVEL_##level)

/**
 * @brief Macro to log FATAL messages with or without timestamp based on
 * compilation settings.
 */
#if OSALOG_IS_BUILT(FATAL)
#define OSALOG_FATAL(args, ...)	OSALOG_HELPER(FATAL, false, args, ##__VA_ARGS__)
#define OSALOGTS_FATAL(args, ...) OSALOG_HELPER(FATAL, true, args, ##__VA_ARGS__)
#else
#define OSALOG_FATAL(args, ...)
#define OSALOGTS_FATAL(args, ...)
#endif

/**
 * @brief Macro to log ERROR messages with or without timestamp based on
 * compilation settings.
 */
#if OSALOG_IS_BUILT(ERROR)
#define OSALOG_ERROR(args, ...)	OSALOG_HELPER(ERROR, false, args, ##__VA_ARGS__)
#define OSALOGTS_ERROR(args, ...) OSALOG_HELPER(ERROR, true, args, ##__VA_ARGS__)
#else
#define OSALOG_ERROR(args, ...)
#define OSALOGTS_ERROR(args, ...)
#endif

/**
 * @brief Macro to log WARN messages with or without timestamp based on
 * compilation settings.
 */
#if OSALOG_IS_BUILT(WARN)
#define OSALOG_WARN(args, ...) OSALOG_HELPER(WARN, false, args, ##__VA_ARGS__)
#define OSALOGTS_WARN(args, ...) OSALOG_HELPER(WARN, true, args, ##__VA_ARGS__)
#else
#define OSALOG_WARN(args, ...)
#define OSALOGTS_WARN(args, ...)
#endif

/**
 * @brief Macro to log INFO messages with or without timestamp based on
 * compilation settings.
 */
#if OSALOG_IS_BUILT(INFO)
#define OSALOG_INFO(args, ...) OSALOG_HELPER(INFO, false, args, ##__VA_ARGS__)
#define OSALOGTS_INFO(args, ...) OSALOG_HELPER(INFO, true, args, ##__VA_ARGS__)
#else
#define OSALOG_INFO(args, ...)
#define OSALOGTS_INFO(args, ...)
#endif

/**
 * @brief Macro to log DEBUG messages with or without timestamp based on
 * compilation settings.
 */
#if OSALOG_IS_BUILT(DEBUG)
#define OSALOG_DEBUG(args, ...)	OSALOG_HELPER(DEBUG, false, args, ##__VA_ARGS__)
#define OSALOGTS_DEBUG(args, ...)	OSALOG_HELPER(DEBUG, true, args, ##__VA_ARGS__)
#else
#define OSALOG_DEBUG(args, ...)
#define OSALOGTS_DEBUG(args, ...)
#endif

/**
 * @brief Macro to log TRACE messages with or without timestamp based on
 * compilation settings.
 */
#if OSALOG_IS_BUILT(TRACE)
#define OSALOG_TRACE(args, ...) OSALOG_HELPER(TRACE, false, args, ##__VA_ARGS__)
#define OSALOGTS_TRACE(args, ...) OSALOG_HELPER(TRACE, true, args, ##__VA_ARGS__)
#else
#define OSALOG_TRACE(args, ...)
#define OSALOGTS_TRACE(args, ...)
#endif

/**
 * @brief Typedef for the logging output function.
 */
typedef void (*osal_log_output_t)(char *logstr);

/**
 * @brief Enumeration defining log levels.
 */
typedef enum {
	OSALOG_LEVEL_NONE, /**< No log at all */
	OSALOG_LEVEL_FATAL, /**< Fatal error level. */
	OSALOG_LEVEL_ERROR, /**< Error level. */
	OSALOG_LEVEL_WARN, /**< Warning level. */
	OSALOG_LEVEL_INFO, /**< Informational level. */
	OSALOG_LEVEL_DEBUG, /**< Debugging level. */
	OSALOG_LEVEL_TRACE, /**< Trace level. */
	OSALOG_LEVEL_MAX /**< Maximum log level. */
} osal_log_level_t;

/**
 * @brief Initializes the logging system.
 *
 * @param log_output Pointer to the logging output function.
 * @return An error code indicating the status of the initialization.
 */
osal_error_t osal_log_init(osal_log_output_t log_output);

/**
 * @brief Deinitializes the logging system.
 */
void osal_log_deinit(void);

/**
 * @brief Initializes a logging module.
 *
 * @param index Index of the module.
 * @param name Name of the logging module. Set NULL to not print module name.
 * @param level Log level for the module.
 * @param ts Flag indicating whether to include timestamp.
 * @return An error code indicating the status of the initialization.
 */
osal_error_t osal_log_module_init(uint32_t index, char *name,
								  osal_log_level_t level, bool ts);

/**
 * @brief Prints a log message.
 *
 * @param index Index of the logging module.
 * @param ts Flag indicating whether to include timestamp.
 * @param level Log level.
 * @param format Format string for the log message.
 * @param ... Additional arguments for the log message.
 * @return An error code indicating the status of the print.
 */
osal_error_t osal_log_print(uint32_t index, bool ts,
				osal_log_level_t level, const char *format, ...);

/**
 * @brief Changes a log level of a module at the runtime.
 *
 * @param index Index of the logging module.
 * @param level Log level. Set to @ref OSALOG_LEVEL_NONE to disable the log.
 * @return An error code indicating the status of the change.
 */
osal_error_t osal_log_module_change(uint32_t index, osal_log_level_t level);

#ifdef __cplusplus	/* extern "C" */
}
#endif

#endif //OSAL_LOG_H

/** @}*/
