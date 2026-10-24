/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

/**
 * @addtogroup dmosal
 * @{
 * @file osal.h
 * @brief OS Abstract Layer Definitions
 * @copyright Copyright (c) 2023, nguyenvannam142@gmail.com
 * @author Nam Nguyen Van(nguyenvannam142@gmail.com)
 */
#ifndef OSAL_H
#define OSAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "osal_error.h"
#include "osal_log.h"
#include "osal_assert.h"
#include "osal_mutex.h"
#include "osal_task.h"
#include "osal_timer.h"
#include "osal_time.h"
#include "osal_sem.h"
#include "osal_queue.h"
#include "osal_rm.h"
#include "osal_tmcheck.h"
#include "osal_lifo.h"
#include "osal_version.h"

/**
 * @brief Configuration for @ref osal_init.
 */
typedef struct {
	osal_log_output_t log_output; /**< Logging output function. */
	osal_log_level_t osal_level;  /**< Log level for the internal OSAL log module. */
} osal_config_t;

/**
 * @brief Initializes the OS abstraction layer.
 *
 * @param config Pointer to the configuration struct, or NULL to use the
 *   built-in defaults. Defaults: @c log_output writes to standard output,
 *   @c osal_level is @ref OSALOG_LEVEL_INFO. When @p config is non-NULL, a
 *   NULL @c config->log_output field also falls back to the default writer.
 * @return An error code of type ::osal_error_t indicating the status of the
 *   initialization.
 */
osal_error_t osal_init(const osal_config_t *config);

/**
 * @brief Retrieves the version of the OS abstraction layer.
 *
 * @return Pointer to a static, null-terminated version string owned by the
 * library. The caller must not free it. Valid for the lifetime of the process.
 */
char *osal_version(void);

/**
 * @brief Print the resource usage of each OSAL submodule.
 */
void osal_print_resource(void);

/**
 * @brief Deinitializes the OS abstraction layer.
 *
 * This function deinitializes the OS abstraction layer.
 */
void osal_deinit(void);

#ifdef __cplusplus /* extern "C" */
}
#endif

#endif //OSAL_H

/** @}*/
