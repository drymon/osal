/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

/* Runs the cmocka test whose main() was renamed to dmosal_test_main, then ends the
 * native_sim process with the test result so CI can use the exit code. */
#include <stdlib.h>

extern int dmosal_test_main(void);

int main(void)
{
	exit(dmosal_test_main() == 0 ? 0 : 1);
}
