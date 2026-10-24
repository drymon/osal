/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

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
#define OSAL_SEC_NSEC  1000000000ULL /**< Conversion factor: seconds to nanoseconds. */
#define OSAL_SEC_USEC  1000000ULL	 /**< Conversion factor: seconds to microseconds. */
#define OSAL_SEC_MSEC  1000ULL		 /**< Conversion factor: seconds to milliseconds. */
#define OSAL_MSEC_NSEC 1000000ULL	 /**< Conversion factor: milliseconds to nanoseconds. */
#define OSAL_MSEC_USEC 1000ULL		 /**< Conversion factor: milliseconds to microseconds. */
#define OSAL_USEC_NSEC 1000ULL		 /**< Conversion factor: microseconds to nanoseconds. */

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
 * @brief Retrieves the current monotonic clock time in nanoseconds.
 *
 * The clock is monotonically non-decreasing and unaffected by wall-clock
 * adjustments. Suitable for measuring elapsed intervals; not suitable for
 * real-world wall-clock timestamps.
 *
 * @param nsec Output pointer; receives the current clock reading in nanoseconds.
 * @return OSAL_E_OK on success, OSAL_E_PARAM if @p nsec is NULL,
 * OSAL_E_OSCALL on a backend failure.
 */
osal_error_t osal_clock_time(uint64_t *nsec);

#ifdef __cplusplus /* extern "C" */
}
#endif

#endif //OSAL_TIME_H

/** @}*/
