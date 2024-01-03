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

#ifndef OSAL_ASSERT /* Allow to define this from the outside */
#include <stdio.h>
#include <stdlib.h>

/**
 * @def OSAL_ASSERT(a)
 * @brief Assertion macro for checking conditions.
 *
 * The macro checks the condition `a` and prints an error message with file name
 * and line number if the condition is not met. It then aborts the program.
 *
 * @param a The condition to be evaluated.
 */
#define OSAL_ASSERT(a) \
	if(!(a)) { \
		printf("assert:%s:%d!!!\n", __FILE__, __LINE__); \
		abort(); \
	}

#endif //OSAL_ASSERT

#ifdef __cplusplus	/* extern "C" */
}
#endif

#endif //OSAL_ASSERT_H

/** @}*/
