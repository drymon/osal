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

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <dmosal/osal.h>

#define OSALOG_MODULE 1

struct timer_ctx {
	const char *name;
	uint64_t start_ns;
	uint32_t period_us;
	volatile int count;
};

static void timer_expire(void *arg)
{
	struct timer_ctx *ctx = arg;
	uint64_t now_ns = 0;
	long long elapsed_us;
	long long expected_us;

	osal_clock_time(&now_ns);
	ctx->count++;
	elapsed_us = (long long)((now_ns - ctx->start_ns) / 1000);
	expected_us = (long long)ctx->period_us * ctx->count;
	OSALOG_INFO("%s #%d expired: elapsed %lld us, expected %lld us, error %+lld us\n",
				ctx->name,
				ctx->count,
				elapsed_us,
				expected_us,
				elapsed_us - expected_us);
}

static osal_error_t timer_begin(osal_timer_t *timer, struct timer_ctx *ctx, bool repeat)
{
	osal_clock_time(&ctx->start_ns);
	return osal_timer_start(timer, ctx->period_us, repeat);
}

int main(void)
{
	int res = -1;
	struct timer_ctx oneshot_ctx = {.name = "one-shot", .period_us = 100000};
	struct timer_ctx periodic_ctx = {.name = "periodic", .period_us = 50000};
	osal_timer_t *oneshot = NULL;
	osal_timer_t *periodic = NULL;

	if (osal_init(NULL) != OSAL_E_OK) {
		return -1;
	}

	if (osal_log_module_init(OSALOG_MODULE, "timer", OSALOG_LEVEL_DEBUG, true) != OSAL_E_OK) {
		return -1;
	}

	OSALOG_INFO("DMOSAL version: %s\n", osal_version());

	oneshot = osal_timer_create(timer_expire, &oneshot_ctx);
	periodic = osal_timer_create(timer_expire, &periodic_ctx);
	if (oneshot == NULL || periodic == NULL) {
		OSALOG_ERROR("failed to create timer\n");
		goto exit;
	}

	/* one-shot after 100 ms, periodic every 50 ms */
	if (timer_begin(oneshot, &oneshot_ctx, false) != OSAL_E_OK ||
		timer_begin(periodic, &periodic_ctx, true) != OSAL_E_OK) {
		OSALOG_ERROR("failed to start timer\n");
		goto exit;
	}

	osal_usleep(500000);

	osal_timer_stop(periodic);
	OSALOG_INFO("one-shot fired: %d, periodic ticks: %d\n", oneshot_ctx.count, periodic_ctx.count);
	res = (oneshot_ctx.count == 1 && periodic_ctx.count > 0) ? 0 : -1;

exit:
	if (periodic != NULL) {
		osal_timer_delete(periodic);
	}
	if (oneshot != NULL) {
		osal_timer_delete(oneshot);
	}
	OSALOG_DEBUG("osal_deinit\n");
	osal_deinit();
	return res;
}
