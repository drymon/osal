#include "cmocka_include.h"
#include "osal_lifo.h"

#define NUM_NODES 100

typedef struct {
	osal_lifo_node_t node;
	int data;
} lifo_data_t;

static void test_lifo(void **state)
{
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

int main(int argc, char **argv)
{
	setenv("CMOCKA_TEST_ABORT", "1", 1);

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_lifo),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
