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
 * @file osal_rm.h
 * @brief OS Abstraction Layer Resource Manager (RM) Definitions
 * @copyright Copyright (c) 2023, nguyenvannam142@gmail.com
 * @author Nam Nguyen Van(nguyenvannam142@gmail.com)
 */
#ifndef OSAL_RM_H
#define OSAL_RM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "osal_mutex.h"
#include "osal_lifo.h"

/**
 * @brief Structure defining a resource managed by the resource manager.
 */
typedef struct {
	osal_lifo_node_t node; /**< Node for managing resources in a LIFO manner. */
	bool used; /**< Flag indicating if the resource is currently in use. */
	void *data; /**< Pointer to user-assigned data for the resource. */
} osal_resrc_t;

/**
 * @brief Structure defining the resource manager.
 */
typedef struct {
	osal_mutex_t *mutex; /**< The mutex used for resource manager synchronization. */
	osal_lifo_t resrc_pool; /**< Resource pool for managing resources. */
	uint32_t n_resrces; /**< Number of resources in the pool. */
} osal_rm_t;

/**
 * @brief Structure defining the configuration for the resource manager.
 */
typedef struct {
	osal_mutex_t *mutex; /**< The mutex to protect the rm, set to NULL if protect is not required */
	uint32_t n_resrces; /**< Number of resources in the resource manager. */
	osal_resrc_t *resrces; /**< Pointer to the array of the global resources */
} osal_rm_cfg_t;

/**
 * @brief Initializes the resource manager.
 *
 * @param rm Pointer to the resource manager structure to be initialized.
 * @param cfg Pointer to the configuration for initializing the resource manager.
 * @return An error code indicating the status of the initialization.
 */
osal_error_t osal_rm_init(osal_rm_t *rm, osal_rm_cfg_t *cfg);

/**
 * @brief Deinitializes the resource manager.
 *
 * @param rm Pointer to the resource manager structure to be deinitialized.
 */
void osal_rm_deinit(osal_rm_t *rm);

/**
 * @brief Allocates a resource from the resource manager.
 *
 * @param rm Pointer to the resource manager.
 * @return Pointer to the allocated resource.
 */
osal_resrc_t *osal_rm_alloc(osal_rm_t *rm);

/**
 * @brief Frees a resource in the resource manager.
 *
 * @param rm Pointer to the resource manager.
 * @param resrc Pointer to the resource to be freed.
 */
void osal_rm_free(osal_rm_t *rm, osal_resrc_t *resrc);

/**
 * @brief Retrieves the count of available resources in the resource manager.
 *
 * @param rm Pointer to the resource manager.
 * @return The count of available resources.
 */
uint32_t osal_rm_avail(osal_rm_t *rm);

/**
 * @brief Retrieves the count of used resources in the resource manager.
 *
 * @param rm Pointer to the resource manager.
 * @return The count of used resources.
 */
uint32_t osal_rm_use(osal_rm_t *rm);

/**
 * @brief Macro for declaring a user-managed object array with the resource manager.
 *
 * @param userobj_type Type of user-managed object.
 * @param userobj_num Number of user-managed objects.
 */
#define OSAL_RM_USEROBJMAN_DECLARE(userobj_type, userobj_num) \
	userobj_type userobj[userobj_num]; \
	osal_resrc_t resrces[userobj_num]; \
	osal_rm_t rm;

/**
 * @brief Macro for initializing a user-managed object array with the resource manager.
 *
 * @param userobjman_ptr Pointer to the user-managed object array.
 * @param userobj_num Number of user-managed objects.
 * @param mutex_ptr External mutex if the resource manager should be thread-safe.
 */
#define OSAL_RM_USEROBJMAN_INIT(userobjman_ptr, userobj_num, mutex_ptr)	\
	{ \
		osal_rm_cfg_t rmcfg; \
		int i; \
		int res; \
		for (i = 0; i < userobj_num; i++) { \
			(userobjman_ptr)->resrces[i].data = &(userobjman_ptr)->userobj[i]; \
		} \
		rmcfg.mutex = mutex_ptr; \
		rmcfg.n_resrces = userobj_num; \
		rmcfg.resrces = (userobjman_ptr)->resrces; \
		res = osal_rm_init(&(userobjman_ptr)->rm, &rmcfg); \
		OSAL_RUNTIME_ASSERT(res == OSAL_E_OK); \
	}

#ifdef __cplusplus	/* extern "C" */
}
#endif

#endif //OSAL_RM_H

/** @}*/
