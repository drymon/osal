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
 * @file osal_mutex.h
 * @brief OS Abstraction Layer Mutex Definitions
 * @copyright Copyright (c) 2023, nguyenvannam142@gmail.com
 * @author Nam Nguyen Van(nguyenvannam142@gmail.com)
 */
#ifndef OSAL_MUTEX_H
#define OSAL_MUTEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "osal_error.h"
#include "osal_config.h"

/**
 * @brief Forward declaration of the OS abstraction layer mutex structure.
 */
typedef struct osal_mutex osal_mutex_t;

/**
 * @brief Global mutex for shared resources.
 *
 * A global mutex, created within the @ref osal_mutex_init() and deleted within
 * the @ref osal_mutex_deinit(). It is designed to minimize the number of mutex
 * instances. In certain use cases, utilizing this shared mutex may be sufficient.
 *
 * @note Ensure proper initialization using @ref osal_mutex_init() before
 * accessing this mutex.
 */
extern osal_mutex_t *g_osal_shared_mutex;

/**
 * @brief Initializes the OS abstraction layer mutex subsystem.
 *
 * @return An error code indicating the status of the initialization.
 */
osal_error_t osal_mutex_init(void);

/**
 * @brief Deinitializes the OS abstraction layer mutex subsystem.
 */
void osal_mutex_deinit(void);

/**
 * @brief Creates a mutex in the OS abstraction layer.
 *
 * @return Pointer to the created mutex.
 */
osal_mutex_t *osal_mutex_create(void);

/**
 * @brief Deletes a mutex from the OS abstraction layer.
 *
 * @param mutex Pointer to the mutex to be deleted.
 */
void osal_mutex_delete(osal_mutex_t *mutex);

/**
 * @brief Locks a mutex.
 *
 * @param mutex Pointer to the mutex to be locked.
 * @return An error code of type ::osal_error_t indicating the status of the lock acquisition.
 */
osal_error_t osal_mutex_lock(osal_mutex_t *mutex);

/**
 * @brief Releases the lock on the mutex.
 *
 * This function releases the lock on the specified mutex.
 *
 * @param mutex Pointer to the mutex to be unlocked.
 * @return An error code of type ::osal_error_t indicating the status of the lock release.
 */
osal_error_t osal_mutex_unlock(osal_mutex_t *mutex);

/**
 * @brief Retrieves the count of used mutexes.
 *
 * @return The count of currently used mutexes.
 */
uint32_t osal_mutex_use(void);

/**
 * @brief Retrieves the count of available mutexes.
 *
 * @return The count of currently available (unused) mutexes.
 */
uint32_t osal_mutex_avail(void);

#ifdef __cplusplus	/* extern "C" */
}
#endif

#endif //OSAL_MUTEX_H

/** @}*/
