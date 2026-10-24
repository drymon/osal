/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

/**
 * @addtogroup dmosal
 * @{
 * @file osal_assert.h
 * @brief OS Abstraction Layer Assertion Definitions
 * @copyright Copyright (c) 2023, nguyenvannam142@gmail.com
 * @author Nam Nguyen Van(nguyenvannam142@gmail.com)
 */
#ifndef OSAL_ASSERT_H
#define OSAL_ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OSAL_RUNTIME_ASSERT /* Allow to define this from the outside */
#include <stdio.h>
#include <stdlib.h>

/**
 * @def OSAL_RUNTIME_ASSERT(cond)
 * @brief Assertion macro for checking conditions.
 *
 * The macro checks the condition `cond` and prints an error message with file name
 * and line number if the condition is not met. It then aborts the program.
 *
 * @param cond The condition to be evaluated.
 */
#define OSAL_RUNTIME_ASSERT(cond)                                                                  \
	if (!(cond)) {                                                                                 \
		printf("assert:%s:%d!!!\n", __FILE__, __LINE__);                                           \
		abort();                                                                                   \
	}

#endif //OSAL_RUNTIME_ASSERT

/**
 * @brief Compile-time assertion macro.
 *
 * This macro causes a compilation failure if the condition `cond` evaluates to false.
 * It is typically used to validate assumptions during compilation,
 * such as structure sizes or constant values.
 *
 * @param cond Condition to evaluate at compile time.
 */
#define OSAL_STATIC_ASSERT(cond) extern char msg[(cond) ? 1 : -1]

#ifdef __cplusplus /* extern "C" */
}
#endif

#endif //OSAL_ASSERT_H

/** @}*/
