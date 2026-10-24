# Contributing

Contributions are welcome: bug reports, patches, or ideas.

## Build and test

```
sudo apt install cmake libcmocka-dev
cmake -S . -B build
cmake --build build --target check
```

Or use [`just`](https://github.com/casey/just): `just check`. Zephyr and FreeRTOS
backends are tested in Docker with `just zephyr-test` and `just freertos-test`.
CI runs all three, so please keep the portable tests in `test/osal/` free of
backend-specific code (put those in `test/<backend>/`).

## Style

There's a `.clang-format` at the repo root. If you have clang-format handy, running
it over your changes keeps diffs tidy; if not, don't worry about it, CI will point
out any drift.

## PRs

Open an issue or a pull request against `main`. Small, focused commits are easier
to review, but there are no hard rules.
