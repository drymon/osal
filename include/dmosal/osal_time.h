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
 * @file osal_time.h
 * @brief OS Abstraction Layer Time Definitions
 * @copyright Copyright (c) 2023, nguyenvannam142@gmail.com
 * @author Nam Nguyen Van(nguyenvannam142@gmail.com)
 */
#ifndef OSAL_TIME_H
#define OSAL_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "osal_error.h"

/**
 * @brief Defines for converting time units.
 */
#define OSAL_SEC_NSEC 1000000000ULL /**< Conversion factor: seconds to nanoseconds. */
#define OSAL_SEC_USEC 1000000ULL /**< Conversion factor: seconds to microseconds. */
#define OSAL_SEC_MSEC 1000ULL /**< Conversion factor: seconds to milliseconds. */
#define OSAL_MSEC_NSEC 1000000ULL  /**< Conversion factor: milliseconds to nanoseconds. */
#define OSAL_MSEC_USEC 1000ULL /**< Conversion factor: milliseconds to microseconds. */
#define OSAL_USEC_NSEC 1000ULL  /**< Conversion factor: microseconds to nanoseconds. */

/**
 * @brief Suspends execution for a specified number of seconds.
 *
 * @param sec Number of seconds to sleep.
 * @return An error code indicating the status of the sleep operation.
 */
osal_error_t osal_sleep(uint32_t sec);

/**
 * @brief Suspends execution for a specified number of microseconds.
 *
 * @param microsec Number of microseconds to sleep.
 * @return An error code indicating the status of the sleep operation.
 */
osal_error_t osal_usleep(uint32_t microsec);

/**
 * @brief Retrieves the current clock time in nanoseconds.
 *
 * @param nsec Pointer to a variable where the clock time in nanoseconds will be stored.
 * @return An error code indicating the status of the clock time retrieval.
 */
osal_error_t osal_clock_time(uint64_t *nsec);

#ifdef __cplusplus	/* extern "C" */
}
#endif

#endif //OSAL_TIME_H

/** @}*/
