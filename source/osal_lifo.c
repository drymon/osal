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

#include <stddef.h>
#include "osal_lifo.h"

void osal_lifo_init(osal_lifo_t *lifo)
{
	lifo->head = NULL;
	lifo->size = 0;
}

void osal_lifo_push(osal_lifo_t *lifo, osal_lifo_node_t *node)
{
	if (lifo->head != NULL) {
		node->next = lifo->head;
		lifo->head = node;
	} else {
		lifo->head = node;
		node->next = NULL;
	}
	lifo->size++;
}

osal_lifo_node_t *osal_lifo_pop(osal_lifo_t *lifo)
{
	osal_lifo_node_t *node;

	node = lifo->head;
	if (node == NULL) {
		return NULL;
	}
	lifo->head = node->next;
	lifo->size--;
	return node;
}

uint32_t osal_lifo_size(osal_lifo_t *lifo)
{
	return lifo->size;
}
