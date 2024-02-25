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
 * @param expire Function pointer to the timer expiration callback.
 * @param arg Pointer to the argument to be passed to the expiration callback.
 * @return Pointer to the created timer.
 */
osal_timer_t *osal_timer_create(void (*expire)(void *arg), void *arg);

/**
 * @brief Starts a timer with a specified timeout and repetition setting.
 *
 * @param timer Pointer to the timer to be started.
 * @param usec Time in microseconds for the timer's timeout.
 * @param repeat Boolean flag indicating whether the timer should repeat after expiration.
 * @return An error code indicating the status of the timer start operation.
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

#ifdef __cplusplus	/* extern "C" */
}
#endif

#endif //OSAL_TIMER_H

/** @}*/
