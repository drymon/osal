/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2023, nguyenvannam142@gmail.com */

/**
 * @addtogroup dmosal
 * @{
 * @file osal_tmcheck.h
 * @brief Time checkpoint utility for measuring execution time between code sections.
 * This module provides functionality to mark and measure the time elapsed
 * between multiple checkpoints in code. It is useful for debugging and
 * performance analysis, allowing developers to identify which parts of
 * the code consume the most time.
 * @copyright Copyright (c) 2025, nguyenvannam142@gmail.com
 * @author Nam Nguyen Van(nguyenvannam142@gmail.com)
 */

#ifndef OSAL_TMCHECK_H
#define OSAL_TMCHECK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "osal_mutex.h"
#include "osal_error.h"
#include "osal_config.h"

/**
 * @def OSAL_CONCATE(x, y)
 * @brief Concatenates two tokens.
 */
#define OSAL_CONCATE(x, y) x##y

/**
 * @def OSAL_CONCATE2(x, y)
 * @brief Expands and concatenates two tokens.
 */
#define OSAL_CONCATE2(x, y) OSAL_CONCATE(x, y)

/**
 * @def OSAL_TMCHECK_CAPTURE()
 * @brief Captures a timestamp at the current location in code.
 *
 * A static checkpoint is created (if not already allocated) based on the function
 * name and line number, and the current timestamp is captured for it.
 *
 * @warning The checkpoint is a per-call-site static, so the same code in the same
 * function must not be run from multiple threads through this macro: all threads would
 * share one checkpoint (capture is one-shot, so only the first timestamp is kept), and
 * the lazy creation is not thread-safe. In that case, allocate a unique id per thread,
 * create one checkpoint per id with osal_tmcheck_create() (for example named
 * "func:line:id"), and call osal_tmcheck_capture_ts() with that index instead of
 * using this macro.
 */
#define OSAL_TMCHECK_CAPTURE()                                                                     \
	do {                                                                                           \
		static int8_t OSAL_CONCATE2(tmcheck_, __LINE__) = -1;                                      \
		if (OSAL_CONCATE2(tmcheck_, __LINE__) == -1) {                                             \
			char OSAL_CONCATE2(tmcheck_name, __LINE__)[32];                                        \
			snprintf(OSAL_CONCATE2(tmcheck_name, __LINE__),                                        \
					 sizeof(OSAL_CONCATE2(tmcheck_name, __LINE__)),                                \
					 "%s:%d",                                                                      \
					 __func__,                                                                     \
					 __LINE__);                                                                    \
			OSAL_CONCATE2(tmcheck_, __LINE__) =                                                    \
				osal_tmcheck_create(OSAL_CONCATE2(tmcheck_name, __LINE__));                        \
		}                                                                                          \
		osal_tmcheck_capture_ts(OSAL_CONCATE2(tmcheck_, __LINE__));                                \
	} while (0)

/**
 * @def OSAL_TMCHECK_PRINT_DIFF_RESET_ALL()
 * @brief Prints and resets all time checkpoints.
 *
 * Prints the time differences between all captured checkpoints and resets them.
 */
#define OSAL_TMCHECK_PRINT_DIFF_RESET_ALL()                                                        \
	do {                                                                                           \
		osal_tmcheck_print_diff_all();                                                             \
		osal_tmcheck_reset_all();                                                                  \
	} while (0)

/**
 * @brief Initializes the time check module with a user-provided mutex.
 *
 * @param mutex Pointer to an OSAL mutex.
 * @return OSAL_SUCCESS on success, or an error code on failure.
 */
osal_error_t osal_tmcheck_init(osal_mutex_t *mutex);

/**
 * @brief Deinitializes the time check module and frees all resources.
 */
void osal_tmcheck_deinit(void);

/**
 * @brief Creates a named time checkpoint.
 *
 * @param name A unique name identifying the checkpoint.
 * @return Index of the checkpoint, or -1 on error.
 */
int osal_tmcheck_create(const char *name);

/**
 * @brief Deletes a previously created time checkpoint.
 *
 * @param idx Index of the checkpoint to delete.
 */
void osal_tmcheck_delete(int idx);

/**
 * @brief Captures the current timestamp for the given checkpoint index.
 *
 * @param idx Index of the checkpoint.
 */
void osal_tmcheck_capture_ts(int idx);

/**
 * @brief Prints timestamps of all checkpoints.
 */
void osal_tmcheck_print_all(void);

/**
 * @brief Prints the time difference between two checkpoint indices.
 *
 * @param idx1 Index of the first checkpoint.
 * @param idx2 Index of the second checkpoint.
 */
void osal_tmcheck_print_diff(int idx1, int idx2);

/**
 * @brief Prints the time differences between consecutive checkpoints in sorted order.
 *
 * Captured checkpoints are sorted by timestamp in ascending order, and the time
 * differences between each consecutive pair are printed. This helps visualize
 * the time spent between logically sequential points in code.
 */
void osal_tmcheck_print_diff_all(void);

/**
 * @brief Resets the timestamp of a specific checkpoint.
 *
 * @param idx Index of the checkpoint.
 */
void osal_tmcheck_reset(int idx);

/**
 * @brief Resets all captured checkpoints.
 */
void osal_tmcheck_reset_all(void);

/**
 * @brief Gets the time difference between two checkpoint indices, in nanoseconds.
 *
 * @param idx1 Index of the first checkpoint.
 * @param idx2 Index of the second checkpoint.
 * @param out_ns Output pointer receiving the diff `ts[idx2] - ts[idx1]` in nanoseconds.
 * @return OSAL_E_OK on success, OSAL_E_PARAM if `out_ns` is NULL or an index is invalid.
 */
osal_error_t osal_tmcheck_get_diff(int idx1, int idx2, int64_t *out_ns);

/**
 * @brief Prints the time difference between two named checkpoints.
 *
 * @param name1 Name of the first checkpoint.
 * @param name2 Name of the second checkpoint.
 */
void osal_tmcheck_name_print_diff(const char *name1, const char *name2);

/**
 * @brief Gets the time difference between two named checkpoints, in nanoseconds.
 *
 * @param name1 Name of the first checkpoint.
 * @param name2 Name of the second checkpoint.
 * @param out_ns Output pointer receiving the diff `ts[name2] - ts[name1]` in nanoseconds.
 * @return OSAL_E_OK on success, OSAL_E_PARAM on bad input or if either name is not found.
 */
osal_error_t osal_tmcheck_name_get_diff(const char *name1, const char *name2, int64_t *out_ns);

/**
 * @brief Gets the last captured timestamp for a given checkpoint index.
 *
 * @param idx Index of the checkpoint.
 * @return Captured timestamp in nanosecond unit
 */
uint64_t osal_tmcheck_get_captured_ts(int idx);

/**
 * @brief Gets the last captured timestamp for a given named checkpoint.
 *
 * @param name Name of the checkpoint.
 * @return Captured timestamp in nanosecond unit
 */
uint64_t osal_tmcheck_name_get_captured_ts(const char *name);

/**
 * @brief Retrieves the count of used tmcheck points.
 *
 * @return The count of currently used tmcheck points.
 */
uint32_t osal_tmcheck_use(void);

/**
 * @brief Retrieves the count of available tmcheck point.
 *
 * @return The count of currently available (unused) tmcheck point.
 */
uint32_t osal_tmcheck_avail(void);

#ifdef __cplusplus /* extern "C" */
}
#endif

#endif /* OSAL_TMCHECK_H */

/** @}*/
