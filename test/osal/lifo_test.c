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
#include "osal_lifo.h"

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
		lifodatas[i].data = i+1;
		osal_lifo_push(&lifo, &lifodatas[i].node);
		size = osal_lifo_size(&lifo);
		assert_int_equal(size, i+1);
	}

	for (i = 0; i < NUM_NODES; i++) {
		size = osal_lifo_size(&lifo);
		assert_int_equal(size, NUM_NODES-i);

		nodedata = (lifo_data_t *)osal_lifo_pop(&lifo);
		assert_non_null(nodedata);
		assert_int_equal(nodedata->data, NUM_NODES-i);
	}
	size = osal_lifo_size(&lifo);
	assert_int_equal(size, 0);
	nodedata = (lifo_data_t *)osal_lifo_pop(&lifo);
	assert_null(nodedata);
}

int main(void)
{
	setenv("CMOCKA_TEST_ABORT", "1", 1);

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_lifo),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
