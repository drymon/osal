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

#include "osal.h"
#include "osal_assert.h"
#include "osal_mutex.h"
#include "osal_task.h"
#include "osal_timer.h"
#include "osal_sem.h"
#include "osal_queue.h"
#include "osal_version.h"

osal_error_t osal_init(void)
{
	osal_error_t res;

	/* mutex must be init first since it is used in other osal modules */
	res = osal_mutex_init();
	OSAL_ASSERT(res == OSAL_E_OK);
	res = osal_sem_init();
	OSAL_ASSERT(res == OSAL_E_OK);
	res = osal_task_init();
	OSAL_ASSERT(res == OSAL_E_OK);
	res = osal_timer_init();
	OSAL_ASSERT(res == OSAL_E_OK);
	res = osal_queue_init();
	OSAL_ASSERT(res == OSAL_E_OK);
	return OSAL_E_OK;
}

char *osal_version(void)
{
	return OSAL_VERSION;
}

void osal_deinit(void)
{
	osal_mutex_deinit();
	osal_sem_deinit();
	osal_task_deinit();
	osal_timer_deinit();
	osal_queue_deinit();
}
