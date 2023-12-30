# OSAL - OS Abstraction Layer for Embedded Systems

OSAL (OS Abstraction Layer) is a lightweight, static resource allocation framework designed for Real-Time Operating Systems (RTOS) in embedded systems.
It offers a seamless interface for managing tasks, semaphores, mutexes, timers, and more, all statically defined at initialization.
Under the permissive BSD-2-Clause license, users can freely utilize and distribute the library without licensing complexities.

## Features

- **Static Resource Definition:** Allocate resources at initialization, promoting efficiency and predictability in resource-constrained environments.
- **Simple and Lightweight**: OSAL offers easily understandable source code and effortless integration into your application..
- **POSIX Compatibility:** Currently supports POSIX-based systems and designed for easy adaptation to other OS environments.
- **High Portability:** Flexible design facilitates adaptation to various RTOS and embedded platforms.
- **OS components:** Supports mutex, semaphore, task, timer and time.

## Usage

```
   +-------------------------------+
   |           Application         |
   +-------------------------------+
   +--------------+ +--------------+
   |       OSAL   | |    libc      |
   +--------------+ +--------------+
   +-------------------------------+
   |       OS User Space           |
   +-------------------------------+
   +-------------------------------+
   |         OS Kernel Space       |
   +-------------------------------+
   +-------------------------------+
   |       HW                      |
   +-------------------------------+
```

OSAL aims to enhance application code reliability by effectively managing OS resources, ensuring portability across multiple operating systems.

## Compilation

- Build the lib:

```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

- Install the lib:

```
$ sudo make install # install into `/usr/local/`
```

- Uninstall the lib:

```
$ sudo make uninstall # uninstall
```

## Test

The test can be run after the compilation steps above:
```
$ mkdir build
$ cd build
$ cmake ..
$ make
$ make check
```

## Example

```
#include <stdio.h>
#include "osal.h"
#include "osal_task.h"

static void task_handler(void *arg)
{
	printf("--task is started--\n");
}

int main()
{
	int res = -1;
	int err;
	osal_task_t *task = NULL;
	osal_task_cfg_t cfg = {
		.task_handler = &task_handler,
	};

	err = osal_init();
	if (err != OSAL_E_OK) {
		printf("failed to init osal\n");
		return -1;
	}
	printf("osal_init OK\n");

	task = osal_task_create(&cfg);
	if (task == NULL) {
		printf("failed to create task\n");
		goto exit;
	}
	osal_task_delete(task);
	res = 0;

exit:
	osal_deinit();
	printf("osal_deinit\n");
	return res;
}
```

This example code resides in the example folder and can be built using
the following steps:

- Ensure that the libosal library is already installed on your system.
- Execute the following commands in your terminal:

```
$ cd example
$ mkdir build
$ cd build
$ cmake ..
$ make
```

## Dependencies

To run the test code, cmocka is required. Install it on Ubuntu using:

```
$ sudo apt install libcmocka-dev
```

The OSAL is tested with the libcmocka 1.1.5-2

## License

This library is licensed under the BSD-2-Clause license. See the LICENSE file for details.

## Contributions

Contributions are welcome! If you find any bugs or have suggestions for improvements, feel free to open an issue or create a pull request.
