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

#include "cmocka_include.h"
#include "osal.h"

#define NUM_NODES 100

typedef struct {
	osal_lifo_node_t node;
	int data;
} lifo_data_t;

static void test_lifo(void **state)
{
	(void)state;
	lifo_data_t lifodatas[NUM_NODES];
	osal_lifo_t lifo;
	int size;
	int i;
	lifo_data_t *nodedata;

	osal_lifo_init(&lifo);
	size = osal_lifo_size(&lifo);
	assert_int_equal(size, 0);

	for (i = 0; i < NUM_NODES; i++) {
		lifodatas[i].data = i + 1;
		osal_lifo_push(&lifo, &lifodatas[i].node);
		size = osal_lifo_size(&lifo);
		assert_int_equal(size, i + 1);
	}

	for (i = 0; i < NUM_NODES; i++) {
		size = osal_lifo_size(&lifo);
		assert_int_equal(size, NUM_NODES - i);

		nodedata = (lifo_data_t *)osal_lifo_pop(&lifo);
		assert_non_null(nodedata);
		assert_int_equal(nodedata->data, NUM_NODES - i);
	}
	size = osal_lifo_size(&lifo);
	assert_int_equal(size, 0);
	nodedata = (lifo_data_t *)osal_lifo_pop(&lifo);
	assert_null(nodedata);
}

static void test_lifo_is_empty(void **state)
{
	(void)state;
	osal_lifo_t lifo;
	lifo_data_t nodedata;
	osal_lifo_node_t *popped;

	osal_lifo_init(&lifo);
	assert_true(osal_lifo_is_empty(&lifo));

	nodedata.data = 42;
	osal_lifo_push(&lifo, &nodedata.node);
	assert_false(osal_lifo_is_empty(&lifo));

	popped = osal_lifo_pop(&lifo);
	assert_ptr_equal(popped, &nodedata.node);
	assert_true(osal_lifo_is_empty(&lifo));
}

static void test_lifo_pop_empty_repeated(void **state)
{
	(void)state;
	osal_lifo_t lifo;

	osal_lifo_init(&lifo);

	/* Popping an already-empty LIFO repeatedly must stay safe and keep
	 * returning NULL without corrupting size/head. */
	assert_null(osal_lifo_pop(&lifo));
	assert_null(osal_lifo_pop(&lifo));
	assert_int_equal(osal_lifo_size(&lifo), 0);
	assert_true(osal_lifo_is_empty(&lifo));
}

static void test_lifo_lifo_order(void **state)
{
	(void)state;
	osal_lifo_t lifo;
	lifo_data_t a;
	lifo_data_t b;
	lifo_data_t c;
	lifo_data_t *popped;

	osal_lifo_init(&lifo);
	a.data = 1;
	b.data = 2;
	c.data = 3;

	/* Push order a, b, c must pop back in strict reverse order c, b, a. */
	osal_lifo_push(&lifo, &a.node);
	osal_lifo_push(&lifo, &b.node);
	osal_lifo_push(&lifo, &c.node);

	popped = (lifo_data_t *)osal_lifo_pop(&lifo);
	assert_ptr_equal(popped, &c);
	popped = (lifo_data_t *)osal_lifo_pop(&lifo);
	assert_ptr_equal(popped, &b);
	popped = (lifo_data_t *)osal_lifo_pop(&lifo);
	assert_ptr_equal(popped, &a);
	assert_true(osal_lifo_is_empty(&lifo));
}

static void test_lifo_foreach(void **state)
{
	(void)state;
	osal_lifo_t lifo;
	lifo_data_t nodes[3];
	osal_lifo_node_t *iter;
	int seen[3];
	int count = 0;
	int i;

	osal_lifo_init(&lifo);
	for (i = 0; i < 3; i++) {
		nodes[i].data = i;
		osal_lifo_push(&lifo, &nodes[i].node);
	}

	/* OSAL_LIFO_FOREACH must visit every pushed node exactly once, in
	 * head-to-tail (LIFO) order, without mutating the structure. */
	OSAL_LIFO_FOREACH(&lifo, iter)
	{
		lifo_data_t *nodedata = (lifo_data_t *)iter;
		assert_true(count < 3);
		seen[count] = nodedata->data;
		count++;
	}
	assert_int_equal(count, 3);
	assert_int_equal(seen[0], 2);
	assert_int_equal(seen[1], 1);
	assert_int_equal(seen[2], 0);

	/* The LIFO itself must be untouched by iteration. */
	assert_int_equal(osal_lifo_size(&lifo), 3);
}

int main(void)
{
	setenv("CMOCKA_TEST_ABORT", "1", 1);

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_lifo),
		cmocka_unit_test(test_lifo_is_empty),
		cmocka_unit_test(test_lifo_pop_empty_repeated),
		cmocka_unit_test(test_lifo_lifo_order),
		cmocka_unit_test(test_lifo_foreach),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
