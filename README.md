# OSAL - OS Abstraction Layer for Embedded Systems

OSAL (OS Abstraction Layer) is a **lightweight, static resource allocation framework** designed for Real-Time Operating Systems (RTOS) in embedded systems. <br>
All OS resources are preconfigured within this framework, providing a seamless interface for managing tasks, semaphores, mutexes, timers, and more, all statically defined at initialization. <br>
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
   +--------------+----------------+
   |       OSAL   |    libc        |
   +--------------+----------------+
   +-------------------------------+
   |        OS User Space APIs     |
   +-------------------------------+
   +-------------------------------+
   |         OS Kernel Space       |
   +-------------------------------+
   +-------------------------------+
   |            HW                 |
   +-------------------------------+
```

OSAL aims to enhance application code reliability by effectively managing OS resources,
ensuring portability across multiple operating systems.

## Compilation

- Build the lib:

```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

You can configure OSAL resource limitations via the command line.
For detailed information on configuration parameters and how to set them up,
please consult the [osal_limit.cmake](osal_limit.cmake) file.

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
$ make check
```

## Examples

Please refer to the `examples` dir for how to use the OSAL APIs.

The examples code can be built as:

```
$ make examples
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
