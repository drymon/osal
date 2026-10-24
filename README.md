# dmosal: OS Abstraction Layer

[![CI](https://github.com/drymon/osal/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/drymon/osal/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/drymon/osal/branch/main/graph/badge.svg)](https://codecov.io/gh/drymon/osal)
[![License: BSD-2-Clause](https://img.shields.io/badge/License-BSD--2--Clause-blue.svg)](LICENSE)
[![Language: C](https://img.shields.io/badge/language-C-brightgreen.svg)](#)

A small, portable C abstraction layer for tasks, mutexes, semaphores, queues,
timers and time, with the same semantics on every supported OS.

All resources are statically allocated up front, so the code in dmosal itself
never calls `malloc` on any hot path and its worst-case memory use is known in
advance. This does not cover the OS calls underneath: pthreads, POSIX message
queues and timers on Linux, or the FreeRTOS kernel in dynamic allocation mode,
may allocate internally. New backends can be added behind the same API without
breaking existing callers.

## Supported backends

| Backend  | Source                                           | Tested on                           | Details                                  |
|----------|--------------------------------------------------|-------------------------------------|------------------------------------------|
| Linux    | [source/linux/](source/linux/)                   | Native Linux (pthreads, POSIX IPC)  | Quick start below                        |
| Zephyr   | [source/zephyr/](source/zephyr/)                 | `native_sim` virtual board (Docker) | [zephyr/README.md](zephyr/README.md)     |
| FreeRTOS | [source/freertos/](source/freertos/)             | POSIX/Linux simulator (Docker)      | [freertos/README.md](freertos/README.md) |
| ...      | your own, see [docs/porting.md](docs/porting.md) |                                     |                                          |

The same [examples](examples/) and [cmocka tests](test/) run unchanged on every
backend. The Zephyr and FreeRTOS targets need no hardware. More backends can be
added behind the same API.

```
+-------------------------------------------------+
|                   Application                   |
+-------------------------------------------------+
                         |
+-------------------------------------------------+
|                dmosal public API                |
+-------------------------------------------------+
                         |
+----------+ +----------+ +----------+ +----------+
|  Linux   | |  Zephyr  | | FreeRTOS | |   ...    |
+----------+ +----------+ +----------+ +----------+
```

## Quick start

On Linux you only need a compiler, CMake and cmocka:

```
sudo apt install cmake libcmocka-dev
just check             # build and run the tests
just run-examples      # build and run the examples
```

The `just` recipes are thin wrappers around `cmake` and `ctest`. Without
`just`, run the equivalent commands from the [`justfile`](justfile).

The [`Dockerfile`](Dockerfile) provides the full toolchain:

```
docker build -t dmosal-dev .
docker run --rm -v "$PWD":/workspace -w /workspace dmosal-dev just check
```

Resource pool sizes (tasks, queues, mutexes, etc.) are fixed at build time in
[osal_config.cmake](osal_config.cmake) and can be overridden, e.g.
`cmake -DOSAL_CONFIG_QUEUE_NUM_MAX=8 ..`.

## Zephyr and FreeRTOS

Both backends run inside Docker, so no local SDK is required:

```
just zephyr-build     # build examples and tests for native_sim
just zephyr-run       # run the examples
just zephyr-test      # run the cmocka tests

just freertos-build   # build examples and tests for the POSIX simulator
just freertos-run     # run the examples
just freertos-test    # run the cmocka tests
```

## More checks

Additional recipes: `just check-valgrind`, `check-asan`, `check-tsan` and
`check-coverage` for dynamic analysis, `just format` and `just tidy` for style
checks (`just zephyr-tidy` and `just freertos-tidy` for the RTOS backends).
`just --list` shows all recipes.

## Documentation

`just doc` builds the API reference into `build/doc/html/` (requires
`doxygen`). Design documents are in [docs/](docs/README.md): architecture, API
spec, object lifetime, thread safety and the porting guide.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for build, test and code style requirements.

## License

BSD-2-Clause. See [LICENSE](LICENSE).
