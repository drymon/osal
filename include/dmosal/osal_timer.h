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
 * @file osal_timer.h
 * @brief OS Abstraction Layer Timer Definitions
 * @copyright Copyright (c) 2023, nguyenvannam142@gmail.com
 * @author Nam Nguyen Van(nguyenvannam142@gmail.com)
 */
#ifndef OSAL_TIMER_H
#define OSAL_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "osal_error.h"
#include "osal_config.h"
#include "osal_mutex.h"

/**
 * @brief Forward declaration of the OS abstraction layer timer structure.
 */
typedef struct osal_timer osal_timer_t;

/**
 * @brief Initializes the OS abstraction layer timer subsystem.
 *
 * @param mutex Mutex to protect the internal resource.
 * @return An error code indicating the status of the initialization.
 */
osal_error_t osal_timer_init(osal_mutex_t *mutex);

/**
 * @brief Deinitializes the OS abstraction layer timer subsystem.
 */
void osal_timer_deinit(void);

/**
 * @brief Creates a timer in the OS abstraction layer.
 *
 * @param expire Timer expiration callback. Must be non-NULL. The callback
 *   is invoked from an unspecified context managed by the backend, not
 *   the caller's thread; user code must not assume it runs on any
 *   particular thread.
 * @param arg Opaque pointer passed unchanged as the argument to @p expire.
 * @return Pointer to the created timer, or NULL if @p expire is NULL, the
 *   timer pool is exhausted, or the backend fails to create the timer.
 */
osal_timer_t *osal_timer_create(void (*expire)(void *arg), void *arg);

/**
 * @brief Starts a timer with a specified timeout and repetition setting.
 *
 * @param timer Pointer to the timer to be started.
 * @param usec Time in microseconds until the first expiration. Must be > 0.
 * @param repeat If true, the timer re-arms with the same interval after each
 *   expiration. If false, the timer fires once.
 * @return OSAL_E_OK on success. OSAL_E_PARAM if @p timer is NULL or @p usec is 0.
 *   OSAL_E_OSCALL if the backend cannot arm the timer.
 */
osal_error_t osal_timer_start(osal_timer_t *timer, uint32_t usec, bool repeat);

/**
 * @brief Stops a running timer.
 *
 * @param timer Pointer to the timer to be stopped.
 */
void osal_timer_stop(osal_timer_t *timer);

/**
 * @brief Deletes a timer from the OS abstraction layer.
 *
 * @param timer Pointer to the timer to be deleted.
 *
 * @warning Must not be called while the timer's expire callback is running
 * (from within the callback itself or concurrently from any other thread).
 * Doing so aborts the process via @ref OSAL_RUNTIME_ASSERT — the delete
 * would free the timer out from under the callback. To retire a timer
 * safely, stop it first with @ref osal_timer_stop and ensure any in-flight
 * callback has returned before calling this function.
 */
void osal_timer_delete(osal_timer_t *timer);

/**
 * @brief Retrieves the count of used timers.
 *
 * @return The count of currently used timers.
 */
uint32_t osal_timer_use(void);

/**
 * @brief Retrieves the count of available timers.
 *
 * @return The count of currently available (unused) timers.
 */
uint32_t osal_timer_avail(void);

#ifdef __cplusplus /* extern "C" */
}
#endif

#endif //OSAL_TIMER_H

/** @}*/
