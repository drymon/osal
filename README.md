# dmosal — OS Abstraction Layer

[![CI](https://github.com/drymon/osal/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/drymon/osal/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/drymon/osal/branch/main/graph/badge.svg)](https://codecov.io/gh/drymon/osal)
[![License: BSD-2-Clause](https://img.shields.io/badge/License-BSD--2--Clause-blue.svg)](LICENSE)
[![Language: C](https://img.shields.io/badge/language-C-brightgreen.svg)](#)

A small, portable embedded C abstraction layer providing deterministic/bounded
resource management and consistent concurrency, timing, and communication
semantics across POSIX and RTOS platforms.

Everything (tasks, mutexes, semaphores, queues, timers, ...) is allocated
statically up front through a central resource manager, so there's no malloc
on the hot path and the worst-case memory usage is known ahead of time. A
POSIX/Linux backend and a Zephyr backend are included; other targets can be
added behind the same API without breaking existing callers.

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

## Getting started

You'll need `cmake`, `libcmocka-dev`, and `doxygen`:

```
sudo apt install cmake libcmocka-dev doxygen
```

Then build the library, run the tests, and check out the examples:

```
just check          # configure + build + run tests
just build           # build the examples target
```

If you'd rather not install a toolchain locally, there's a pinned
[`Dockerfile`](Dockerfile):

```
docker build -t dmosal-dev .
docker run --rm -v "$PWD":/workspace -w /workspace dmosal-dev just check
```

Don't have [`just`](https://github.com/casey/just) installed? Check the
[`justfile`](justfile) — every recipe there is just a couple of plain
`cmake`/`ctest` commands you can run directly.

There are a few more recipes worth knowing about: `just check-valgrind`,
`just check-asan`, `just check-tsan`, `just check-coverage`, `just doc`,
`just format`, `just tidy`. Run `just --list` to see everything.

Resource limits (how many tasks, queues, etc. you get) are set at CMake
configure time — see [osal_config.cmake](osal_config.cmake).

## Zephyr

dmosal also runs on [Zephyr RTOS](https://zephyrproject.org/) through a native
backend (`k_mutex`, `k_sem`, `k_thread`, `k_timer`, `k_msgq`) in
[source/zephyr/](source/zephyr/). The repo is a Zephyr module, and the same
programs from [examples/](examples/) build unchanged for Zephyr. It is tested
on the virtual `native_sim` board, so no hardware is needed, and everything
(including the Zephyr source) runs inside Docker:

```
zephyr/docker.sh build   # build the examples for Zephyr
zephyr/docker.sh run     # run them
```

See [zephyr/README.md](zephyr/README.md) for setup, configuration and how the
backend maps to Zephyr primitives.

## Examples

Sample programs live in [examples/](examples/). Build them with `just build`
(CMake target `examples`) for POSIX, or with `zephyr/docker.sh build` for
Zephyr.

## Documentation

- API reference (Doxygen): run `just doc`, output ends up in
  `build/doc/html/`.
- Design docs: [docs/](docs/README.md) — architecture, API spec, resource
  lifetime, thread-safety, porting guide.

## Contributing

Bug reports, patches, ideas — all welcome. See [CONTRIBUTING.md](CONTRIBUTING.md)
for how to build, test, and keep the style consistent.

## License

BSD-2-Clause — see [LICENSE](LICENSE).
