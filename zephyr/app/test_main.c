/* Runs the cmocka test whose main() was renamed to dmosal_test_main, then ends the
 * native_sim process with the test result so CI can use the exit code. */
#include <stdlib.h>

extern int dmosal_test_main(void);

int main(void)
{
	exit(dmosal_test_main() == 0 ? 0 : 1);
}
