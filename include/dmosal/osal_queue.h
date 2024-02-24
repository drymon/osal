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

typedef struct osal_queue osal_queue_t;

typedef struct {
	uint8_t name[OSAL_QUEUE_NAME_SIZE]; /**< name of the queue. */
	uint32_t msglen; /**< len of the message */
	uint32_t qsize /**< size of the queue */;
} osal_queue_cfg_t;

/**
 * @brief Initializes the OS abstraction layer queue subsystem.
 *
 * @return An error code indicating the status of the initialization.
 */
osal_error_t osal_queue_init(void);

/**
 * @brief Deinitializes the OS abstraction layer queue subsystem.
 */
void osal_queue_deinit(void);

/**
 * @brief Creates a queue in the OS abstraction layer.
 *
 * @param cfg Pointer to the queue configuration.
 * @return Pointer to the created queue.
 */
osal_queue_t *osal_queue_create(osal_queue_cfg_t *cfg);

/**
 * @brief Deletes a queue from the OS abstraction layer.
 *
 * @param queue Pointer to the queue to be deleted.
 */
void osal_queue_delete(osal_queue_t *queue);

/**
 * @brief Sends a message into the queue.
 *
 * @param queue Pointer to the queue.
 * @param msg Pointer to the message data.
 * @param msglen Length of the message.
 * @return An error code indicating the status of the send.
 */
osal_error_t osal_queue_send(osal_queue_t *queue, uint8_t *msg, uint32_t msglen);

/**
 * @brief Receives a message from the queue.
 *
 * @param queue Pointer to the queue.
 * @param msg Pointer to the received buffer.
 * @param bufsize Size of the buffer.
 * @param timeout_usec Timeout to wait on queue when having no message
 * @return An error code indicating the status of the receive.
 */
osal_error_t osal_queue_recv(osal_queue_t *queue, uint8_t *buf,
							 uint32_t bufsize, uint32_t timeout_usec);
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

#ifdef __cplusplus	/* extern "C" */
}
#endif

#endif //OSAL_QUEUE_H

/** @}*/
