# OSAL - OS Abstraction Layer for Embedded Systems

[![CI](https://github.com/drymon/osal/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/drymon/osal/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/drymon/osal/branch/main/graph/badge.svg)](https://codecov.io/gh/drymon/osal)
[![License: BSD-2-Clause](https://img.shields.io/badge/License-BSD--2--Clause-blue.svg)](LICENSE)
[![Language: C](https://img.shields.io/badge/language-C-brightgreen.svg)](#)

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

## Prerequisites

On Ubuntu/Debian:

```
$ sudo apt install cmake libcmocka-dev doxygen
```

- `cmake` — build system.
- `libcmocka-dev` — test framework used by `make check`. Tested with libcmocka 1.1.5-2.
- `doxygen` — generates API documentation via `make doc`.

Optional, for local style/lint checks (see [Style checks](#style-checks) below):

```
$ sudo apt install clang-format python3-pip
$ pip install pre-commit
```

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
please consult the [osal_config.cmake](osal_config.cmake) file.

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

## Style checks

Style is described by `.clang-format` at the repo root. CI enforces it, but
you can check locally too.

- Check without changing files:

```
$ git ls-files '*.c' '*.h' | grep -v '\.in$' | xargs clang-format --dry-run -Werror
```

- Auto-format in place:

```
$ git ls-files '*.c' '*.h' | grep -v '\.in$' | xargs clang-format -i
```

Optional `pre-commit` hooks (trailing whitespace, EOL fixer, clang-format,
etc.) are configured in `.pre-commit-config.yaml`. To enable them:

```
$ pre-commit install                    # run hooks automatically on `git commit`
$ pre-commit run --all-files            # run once against the whole tree
```

**If a pre-commit hook fails on `git commit`:** most hooks auto-fix the files
(clang-format, trailing whitespace, end-of-file). Just re-stage and commit again:

```
$ git commit -m "..."                   # hook edits some files and blocks the commit
$ git add -u                            # re-stage the hook's fixes
$ git commit -m "..."                   # commit succeeds
```

## Doc

Design and behavioral documentation (architecture, API spec, resource
lifetime, thread-safety, porting guide) lives in [docs/](docs/README.md).

To generate the Doxygen API reference, execute the following command.
The output can be located in the build directory.

```
$ make doc
```

## License

This library is licensed under the BSD-2-Clause license. See the LICENSE file for details.

## Contributions

Contributions are welcome! If you find any bugs or have suggestions for improvements, feel free to open an issue or create a pull request.
