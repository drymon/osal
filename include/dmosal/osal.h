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
#include "osal_sem.h"
#include "osal_queue.h"
#include "osal_rm.h"
#include "osal_version.h"

typedef struct {
	osal_log_output_t log_output; /**< Pointer to the logging output function. Set NULL to use the default output */
	osal_log_level_t osal_level; /**< Log level of the OSAL layer */
} osal_config_t;

/**
 * @brief Initializes the OS abstraction layer.
 *
 * This function initializes the OS abstraction layer.
 *
 * @param config Pointer to the configuration struct. Set to NULL to use the default config.
 * @return An error code of type ::osal_error_t indicating the status of
 * the initialization.
 */
osal_error_t osal_init(osal_config_t *config);

/**
 * @brief Retrieves the version of the OS abstraction layer.
 *
 * This function returns a string containing the version of the OS abstraction layer.
 *
 * @return A pointer to a string representing the version.
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

#ifdef __cplusplus	/* extern "C" */
}
#endif

#endif //OSAL_H

/** @}*/
