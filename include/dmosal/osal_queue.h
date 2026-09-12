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
 * @file osal_queue.h
 * @brief OS Abstraction Layer Message Queue Definitions
 * @copyright Copyright (c) 2023, nguyenvannam142@gmail.com
 * @author Nam Nguyen Van(nguyenvannam142@gmail.com)
 */
#ifndef OSAL_QUEUE_H
#define OSAL_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "osal_error.h"
#include "osal_config.h"
#include "osal_mutex.h"

/**
 * @brief Forward declaration of the OS abstraction layer queue structure.
 */
typedef struct osal_queue osal_queue_t;

/**
 * @brief Structure defining the configuration for an OS abstraction layer queue.
 */
typedef struct {
	uint8_t name[OSAL_QUEUE_NAME_SIZE]; /**< Name of the queue (null-terminated). */
	uint32_t msglen;					/**< Maximum length in bytes of a single message. */
	uint32_t qsize;						/**< Maximum number of messages the queue can hold. */
} osal_queue_cfg_t;

/**
 * @brief Initializes the OS abstraction layer queue subsystem.
 *
 * @param mutex Mutex to protect the internal resource.
 * @return An error code indicating the status of the initialization.
 */
osal_error_t osal_queue_init(osal_mutex_t *mutex);

/**
 * @brief Deinitializes the OS abstraction layer queue subsystem.
 */
void osal_queue_deinit(void);

/**
 * @brief Creates a queue in the OS abstraction layer.
 *
 * If a system-visible queue with the same name already exists, this
 * function attaches to it; in that case @c cfg->msglen and @c cfg->qsize
 * must match the existing queue's configuration or the call fails.
 *
 * @param cfg Pointer to the queue configuration. @c name, @c msglen, and
 *   @c qsize must all be non-empty / non-zero.
 * @return Pointer to the created queue, or NULL on bad config, pool
 *   exhaustion, a backend error, or a name-clash size mismatch.
 */
osal_queue_t *osal_queue_create(const osal_queue_cfg_t *cfg);

/**
 * @brief Deletes a queue from the OS abstraction layer.
 *
 * Closes this handle. If this handle originally created the queue (rather
 * than attaching to a pre-existing one), the queue's system name is also
 * released so it becomes free for reuse.
 *
 * @param queue Pointer to the queue to be deleted. No-op if NULL.
 */
void osal_queue_delete(osal_queue_t *queue);

/**
 * @brief Sends a message into the queue.
 *
 * Non-blocking: returns @ref OSAL_E_QFULL immediately if the queue is full.
 *
 * @param queue Pointer to the queue.
 * @param msg Pointer to the message data. Must be non-NULL.
 * @param msglen Length of the message in bytes. Must be > 0 and
 *   <= the @c msglen configured in @ref osal_queue_cfg_t.
 * @return OSAL_E_OK on success, OSAL_E_QFULL if the queue is full,
 *   OSAL_E_PARAM on invalid arguments, OSAL_E_OSCALL on other errors.
 */
osal_error_t osal_queue_send(osal_queue_t *queue, uint8_t *msg, uint32_t msglen);

/**
 * @brief Receives a message from the queue.
 *
 * Blocks up to @p timeout_usec waiting for a message.
 *
 * @param queue Pointer to the queue.
 * @param buf Buffer to receive into. Must be non-NULL.
 * @param bufsize Size of @p buf in bytes.
 * @param timeout_usec Maximum time to wait in microseconds. 0 means
 *   effectively non-blocking / immediate poll.
 * @return OSAL_E_OK on success, OSAL_E_TIMEOUT if no message arrived within
 *   the timeout, OSAL_E_QEMPTY if woken without a readable message,
 *   OSAL_E_PARAM on invalid arguments, OSAL_E_OSCALL on other errors.
 */
osal_error_t
osal_queue_recv(osal_queue_t *queue, uint8_t *buf, uint32_t bufsize, uint32_t timeout_usec);
/**
 * @brief Retrieves the count of used queues.
 *
 * @return The count of currently used queues.
 */
uint32_t osal_queue_use(void);

/**
 * @brief Retrieves the count of available queues.
 *
 * @return The count of currently available (unused) queues.
 */
uint32_t osal_queue_avail(void);

#ifdef __cplusplus /* extern "C" */
}
#endif

#endif //OSAL_QUEUE_H

/** @}*/
