# dmosal: OS Abstraction Layer

[![CI](https://github.com/drymon/osal/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/drymon/osal/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/drymon/osal/branch/main/graph/badge.svg)](https://codecov.io/gh/drymon/osal)
[![License: BSD-2-Clause](https://img.shields.io/badge/License-BSD--2--Clause-blue.svg)](LICENSE)
[![Language: C](https://img.shields.io/badge/language-C-brightgreen.svg)](#)

A small, portable C abstraction layer for tasks, mutexes, semaphores, queues,
timers and time, with the same semantics on every supported OS.

All resources are statically allocated up front, so there is no `malloc` on
any hot path and worst-case memory use is known in advance. New backends can
be added behind the same API without breaking existing callers.

## Supported backends

| Backend  | Source                          | Tested on                          | Details                                |
|----------|---------------------------------|------------------------------------|----------------------------------------|
| Linux    | [source/linux/](source/linux/)  | Native Linux (pthreads, POSIX IPC) | Quick start                                |
| Zephyr   | [source/zephyr/](source/zephyr/) | `native_sim` virtual board (Docker) | [zephyr/README.md](zephyr/README.md)   |
| FreeRTOS | [source/freertos/](source/freertos/) | POSIX/Linux simulator (Docker) | [freertos/README.md](freertos/README.md) |
| ...      | your own, see [docs/porting.md](docs/porting.md) | | |

The same [examples](examples/) and [cmocka tests](test/) run unchanged on every
backend. The Zephyr and FreeRTOS targets need no hardware. More backends can be
added behind the same API.

```
+-------------------------------+
|          Application          |
+---------------+---------------+
|     dmosal    |     libc      |
+---------------+---------------+
| Linux | Zephyr | FreeRTOS | ...|
+-------------------------------+
```

## Quick start

Install the dependencies, then build and test on Linux:

```
sudo apt install cmake libcmocka-dev doxygen
just check          # configure, build, run tests
just build          # build the examples
```

No toolchain locally? Use the pinned [`Dockerfile`](Dockerfile):

```
docker build -t dmosal-dev .
docker run --rm -v "$PWD":/workspace -w /workspace dmosal-dev just check
```

Without [`just`](https://github.com/casey/just), the [`justfile`](justfile)
recipes are plain `cmake`/`ctest` commands you can run directly.

Resource limits (number of tasks, queues, ...) are set at configure time in
[osal_config.cmake](osal_config.cmake).

## Zephyr and FreeRTOS

Both run inside Docker, so no local SDK is needed:

```
just zephyr-build     # build examples and tests for native_sim
just zephyr-run       # run the examples
just zephyr-test      # run the cmocka tests

just freertos-build   # build examples and tests for the POSIX simulator
just freertos-run     # run the examples
just freertos-test    # run the cmocka tests
```

## Other recipes

`just check-valgrind`, `check-asan`, `check-tsan`, `check-coverage`, `doc`,
`format`, `tidy`. Run `just --list` for everything.

## Documentation

- API reference (Doxygen): `just doc` → `build/doc/html/`.
- Design docs: [docs/](docs/README.md) (architecture, API spec, lifetime,
  thread safety, porting guide).

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for building, testing and style.

## License

BSD-2-Clause. See [LICENSE](LICENSE).
