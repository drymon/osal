/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

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
 * Thread-safe: multiple threads may call this concurrently.
 *
 * @return Pointer to the created mutex, or NULL if the mutex pool is exhausted.
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

#ifdef __cplusplus /* extern "C" */
}
#endif

#endif //OSAL_MUTEX_H

/** @}*/
