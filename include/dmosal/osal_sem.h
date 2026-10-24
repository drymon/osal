/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

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
#include "osal_mutex.h"

/**
 * @brief Forward declaration of the OS abstraction layer semaphore structure.
 */
typedef struct osal_sem osal_sem_t;

/**
 * @brief Initializes the OS abstraction layer semaphore subsystem.
 *
 * @param mutex Mutex to protect the internal resource.
 * @return An error code indicating the status of the initialization.
 */
osal_error_t osal_sem_init(osal_mutex_t *mutex);

/**
 * @brief Deinitializes the OS abstraction layer semaphore subsystem.
 */
void osal_sem_deinit(void);

/**
 * @brief Creates a semaphore in the OS abstraction layer.
 *
 * The semaphore is created with an initial count of 0.
 *
 * @return Pointer to the created semaphore, or NULL if the semaphore pool
 *   is exhausted or the backend fails to create the semaphore.
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
 * @param usec Timeout in microseconds.
 * @return OSAL_E_OK on success, OSAL_E_TIMEOUT if the deadline elapses,
 *   OSAL_E_OSCALL on other backend errors, OSAL_E_PARAM if @p sem is NULL.
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

#ifdef __cplusplus /* extern "C" */
}
#endif

#endif //OSAL_SEM_H

/** @}*/
