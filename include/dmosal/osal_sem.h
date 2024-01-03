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
 * @file osal_sem.h
 * @brief OS Abstraction Layer Semaphore Definitions
 * @copyright Copyright (c) 2023, nguyenvannam142@gmail.com
 * @author Nam Nguyen Van(nguyenvannam142@gmail.com)
 */
#ifndef OSAL_SEM_H
#define OSAL_SEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "osal_error.h"
#include "osal_config.h"

/**
 * @brief Forward declaration of the OS abstraction layer semaphore structure.
 */
typedef struct osal_sem osal_sem_t;

/**
 * @brief Initializes the OS abstraction layer semaphore subsystem.
 *
 * @return An error code indicating the status of the initialization.
 */
osal_error_t osal_sem_init(void);

/**
 * @brief Deinitializes the OS abstraction layer semaphore subsystem.
 */
void osal_sem_deinit(void);

/**
 * @brief Creates a semaphore in the OS abstraction layer.
 *
 * @return Pointer to the created semaphore.
 */
osal_sem_t *osal_sem_create(void);

/**
 * @brief Deletes a semaphore from the OS abstraction layer.
 *
 * @param sem Pointer to the semaphore to be deleted.
 */
void osal_sem_delete(osal_sem_t *sem);

/**
 * @brief Posts (signals) a semaphore.
 *
 * @param sem Pointer to the semaphore to be posted.
 * @return An error code indicating the status of the semaphore post operation.
 */
osal_error_t osal_sem_post(osal_sem_t *sem);

/**
 * @brief Waits on a semaphore indefinitely.
 *
 * @param sem Pointer to the semaphore to be waited upon.
 * @return An error code indicating the status of the semaphore wait operation.
 */
osal_error_t osal_sem_wait(osal_sem_t *sem);

/**
 * @brief Waits on a semaphore for a specified time.
 *
 * @param sem Pointer to the semaphore to be waited upon.
 * @param usec Time in microseconds to wait.
 * @return An error code indicating the status of the semaphore wait operation.
 */
osal_error_t osal_sem_waittime(osal_sem_t *sem, uint32_t usec);

/**
 * @brief Retrieves the count of used semaphores.
 *
 * @return The count of currently used semaphores.
 */
uint32_t osal_sem_use(void);

/**
 * @brief Retrieves the count of available semaphores.
 *
 * @return The count of currently available (unused) semaphores.
 */
uint32_t osal_sem_avail(void);

#ifdef __cplusplus	/* extern "C" */
}
#endif

#endif //OSAL_SEM_H

/** @}*/
