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

/**
 * @addtogroup dmosal
 * @{
 * @file osal_task.h
 * @brief OS Abstraction Layer Task Definitions
 * @copyright Copyright (c) 2023, nguyenvannam142@gmail.com
 * @author Nam Nguyen Van(nguyenvannam142@gmail.com)
 */
#ifndef OSAL_TASK_H
#define OSAL_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "osal_error.h"
#include "osal_config.h"

/**
 * @brief Forward declaration of the OS abstraction layer task structure.
 */
typedef struct osal_task osal_task_t;

/**
 * @brief Structure defining the configuration for an OS abstraction layer task.
 */
typedef struct {
	void *stack_addr;  /**< Optional pointer to the task's stack memory. */
	uint32_t stack_size; /**< Optinal size of the task's stack in bytes. */
	uint16_t priority; /**< Optional priority of the task. */
	uint8_t name[OSAL_TASK_NAME_SIZE]; /**< Optional name of the task. */
	void (*task_handler)(void *arg); /**< Pointer to the task's handler function. */
	void *task_arg; /**< Argument to be passed to the task's handler function. */
} osal_task_cfg_t;

/**
 * @brief Initializes the OS abstraction layer task subsystem.
 *
 * @return An error code indicating the status of the initialization.
 */
osal_error_t osal_task_init(void);

/**
 * @brief Deinitializes the OS abstraction layer task subsystem.
 */
void osal_task_deinit(void);

/**
 * @brief Creates a task in the OS abstraction layer.
 *
 * @param cfg Pointer to the task configuration.
 * @return Pointer to the created task.
 */
osal_task_t *osal_task_create(osal_task_cfg_t *cfg);

/**
 * @brief Deletes a task from the OS abstraction layer.
 *
 * @param task Pointer to the task to be deleted.
 */
void osal_task_delete(osal_task_t *task);

/**
 * @brief Retrieves the count of used tasks.
 *
 * @return The count of currently used tasks.
 */
uint32_t osal_task_use(void);

/**
 * @brief Retrieves the count of available tasks.
 *
 * @return The count of currently available (unused) tasks.
 */
uint32_t osal_task_avail(void);

#ifdef __cplusplus	/* extern "C" */
}
#endif

#endif //OSAL_TASK_H

/** @}*/
