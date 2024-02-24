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

#include "cmocka_include.h"
#include "osal_error.h"

static void test_error(void **state)
{
	const char *estr;

	estr = osal_errstr(-1);
	assert_string_equal(estr, "OSAL_UNKNOWN");
	estr = osal_errstr(OSAL_E_MAX);
	assert_string_equal(estr, "OSAL_UNKNOWN");
	estr = osal_errstr(10000);
	assert_string_equal(estr, "OSAL_UNKNOWN");
	estr = osal_errstr(OSAL_E_OK);
	assert_string_equal(estr, "OSAL_E_OK");
	estr = osal_errstr(OSAL_E_PARAM);
	assert_string_equal(estr, "OSAL_E_PARAM");
	estr = osal_errstr(OSAL_E_RESRC);
	assert_string_equal(estr, "OSAL_E_RESRC");
	estr = osal_errstr(OSAL_E_FAILURE);
	assert_string_equal(estr, "OSAL_E_FAILURE");
	estr = osal_errstr(OSAL_E_OSCALL);
	assert_string_equal(estr, "OSAL_E_OSCALL");
	estr = osal_errstr(OSAL_E_TIMEOUT);
	assert_string_equal(estr, "OSAL_E_TIMEOUT");
	estr = osal_errstr(OSAL_E_QFULL);
	assert_string_equal(estr, "OSAL_E_QFULL");
	estr = osal_errstr(OSAL_E_QEMPTY);
	assert_string_equal(estr, "OSAL_E_QEMPTY");
}

int main(int argc, char **argv)
{
	setenv("CMOCKA_TEST_ABORT", "1", 1);

	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_error),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
