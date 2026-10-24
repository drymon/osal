/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

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

uint32_t osal_lifo_size(const osal_lifo_t *lifo)
{
	return lifo->size;
}

bool osal_lifo_is_empty(const osal_lifo_t *lifo)
{
	return lifo->size == 0;
}
