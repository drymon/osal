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
 * @file osal_lifo.h
 * @brief OS Abstraction Layer Last-In-First-Out (LIFO) Definitions
 * @copyright Copyright (c) 2023, nguyenvannam142@gmail.com
 * @author Nam Nguyen Van(nguyenvannam142@gmail.com)
 */
#ifndef OSAL_LIFO_H
#define OSAL_LIFO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "osal_error.h"

/**
 * @brief Structure defining a node for a Last-In-First-Out (LIFO) data structure.
 */
typedef struct osal_lifo_node {
	struct osal_lifo_node *next; /**< Pointer to the next node in the LIFO. */
} osal_lifo_node_t;

/**
 * @brief Structure defining a Last-In-First-Out (LIFO) data structure.
 */
typedef struct {
	osal_lifo_node_t *head; /**< Pointer to the head of the LIFO. */
	uint32_t size; /**< Current size of the LIFO. */
} osal_lifo_t;

/**
 * @brief Initializes a Last-In-First-Out (LIFO) data structure.
 *
 * @param lifo Pointer to the LIFO structure to be initialized.
 */
void osal_lifo_init(osal_lifo_t *lifo);

/**
 * @brief Pushes a node onto the Last-In-First-Out (LIFO) data structure.
 *
 * @param lifo Pointer to the LIFO structure where the node will be pushed.
 * @param node Pointer to the node to be pushed onto the LIFO.
 */
void osal_lifo_push(osal_lifo_t *lifo, osal_lifo_node_t *node);

/**
 * @brief Pops a node from the Last-In-First-Out (LIFO) data structure.
 *
 * @param lifo Pointer to the LIFO structure from where the node will be popped.
 * @return Pointer to the popped node, or NULL if the LIFO is empty.
 */
osal_lifo_node_t *osal_lifo_pop(osal_lifo_t *lifo);

/**
 * @brief Gets the current size of the Last-In-First-Out (LIFO) data structure.
 *
 * @param lifo Pointer to the LIFO structure for which the size will be retrieved.
 * @return Current size of the LIFO.
 */
uint32_t osal_lifo_size(osal_lifo_t *lifo);

/**
 * @brief Checks if the Last-In-First-Out (LIFO) data structure is empty.
 *
 * @param lifo Pointer to the LIFO structure to be checked.
 * @return true if the LIFO is empty, false otherwise.
 */
bool osal_lifo_is_empty(osal_lifo_t *lifo);

/**
 * @brief Macro for iterating through a Last-In-First-Out (LIFO) data structure.
 *
 * Allows iteration through each node in the LIFO structure.
 *
 * Example usage:
 * ```
 * osal_lifo_node_t *node;
 * OSAL_LIFO_FOREACH(lifo, node) {
 *     // Iterate through each node in 'lifo'
 * }
 * ```
 */

#define OSAL_LIFO_FOREACH(lifo, node) \
	for((node)=(lifo)->head; (node)!=NULL; (node)=(node)->next)

#ifdef __cplusplus	/* extern "C" */
}
#endif

#endif //OSAL_LIFO_H

/** @}*/
