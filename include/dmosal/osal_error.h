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
 * @file osal_error.h
 * @brief OS Abstraction Layer Error Definitions
 * @copyright Copyright (c) 2023, nguyenvannam142@gmail.com
 * @author Nam Nguyen Van(nguyenvannam142@gmail.com)
 */
#ifndef OSAL_ERROR_H
#define OSAL_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Error codes for the OS abstraction layer.
 *
 * Defines error codes that can be returned by the OS abstraction layer functions.
 */
typedef enum {
	OSAL_E_OK, /**< No error */
	OSAL_E_PARAM, /**< Invalid parameter */
	OSAL_E_NOINIT, /**< Resource is not initialized yet */
	OSAL_E_RESRC, /**< Insufficient resources */
	OSAL_E_FAILURE, /**< Unspecified failure */
	OSAL_E_OSCALL, /**< Operating system call error */
	OSAL_E_TIMEOUT, /**< Timeout error */
	OSAL_E_QFULL, /**< Queue is full */
	OSAL_E_QEMPTY, /**< Queue is empty*/
	OSAL_E_INUSE, /**< Resource is in use */
	OSAL_E_ALREADY_INIT, /**< Resource is already initialized  */
	OSAL_E_MAX, /**< Maximum error code (for range checking) */
} osal_error_t;

/**
 * @brief Retrieves the error description for a given error code.
 *
 * This function returns a pointer to a string representing the description of the provided error code.
 *
 * @param e The error code of type ::osal_error_t for which the description is required.
 * @return A pointer to a string representing the error description.
 */
const char *osal_errstr(osal_error_t e);

#ifdef __cplusplus	/* extern "C" */
}
#endif

#endif //OSAL_ERROR_H

/** @}*/
