# OSAL - OS Abstraction Layer for Embedded Systems

[![CI](https://github.com/drymon/osal/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/drymon/osal/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/drymon/osal/branch/main/graph/badge.svg)](https://codecov.io/gh/drymon/osal)
[![License: BSD-2-Clause](https://img.shields.io/badge/License-BSD--2--Clause-blue.svg)](LICENSE)
[![Language: C](https://img.shields.io/badge/language-C-brightgreen.svg)](#)

A small, portable embedded C abstraction layer providing deterministic/bounded
resource management and consistent concurrency, timing, and communication
semantics across POSIX and RTOS platforms. Licensed BSD-2-Clause.

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

## Build & test

Requires `cmake`, `libcmocka-dev`, `doxygen` (`sudo apt install cmake
libcmocka-dev doxygen` on Ubuntu/Debian).

```
$ just check          # configure + build + run tests
```

No [`just`](https://github.com/casey/just)? See the [`justfile`](justfile)
for the equivalent raw `cmake`/`ctest` commands, or use the pinned
[`Dockerfile`](Dockerfile) for a reproducible toolchain
(`docker build -t dmosal-dev . && docker run --rm -v "$PWD":/workspace -w
/workspace dmosal-dev just check`).

Other recipes: `just check-valgrind`, `just check-asan`, `just check-tsan`,
`just check-coverage`, `just doc`, `just format`, `just tidy` — run `just
--list` for the full set.

Resource limits (max tasks, queues, etc.) are configured at CMake time; see
[osal_config.cmake](osal_config.cmake).

## Examples

See [examples/](examples/) for sample programs. Build with `just build`
(CMake target `examples`).

## Documentation

- API reference (Doxygen): `just doc`, output in `build/doc/html/`.
- Design/behavioral docs: [docs/](docs/README.md) (architecture, API spec,
  resource lifetime, thread-safety, porting guide).

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md).

## License

BSD-2-Clause — see [LICENSE](LICENSE).
