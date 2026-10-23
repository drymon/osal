# dmosal on Zephyr

Zephyr backend for dmosal, tested on `native_sim/native/64` (Zephyr runs as a
normal Linux process, no real hardware). Only Docker is needed on the host.

## Layout

| Path | What |
|------|------|
| `source/zephyr/osal_*.c` | Zephyr backend (pool logic + `k_*` calls). |
| `zephyr/CMakeLists.txt`, `zephyr/Kconfig`, `zephyr/module.yml` | Makes the repo a Zephyr module; Kconfig sets pool sizes. |
| `zephyr/app/` | Zephyr app (CMake + `prj.conf` only). Builds a file from `examples/`, shared with all platforms. |
| `zephyr/docker.sh`, `zephyr/Dockerfile` | Slim build container + helper. |

The portable code (`source/*.c`) is shared with the Linux build; do not edit it
for Zephyr. Use `source/linux/*.c` as the reference for behaviour.

## Quick start

```
zephyr/docker.sh build   # first run builds the image (~1.8 GB, Zephyr v4.0.0 inside)
zephyr/docker.sh run     # run every example
zephyr/docker.sh shell   # interactive shell (west, cmake, gcc available)
EXAMPLE=task_sample zephyr/docker.sh build   # just one example; same for run
```

`run` passes `-stop_at=3` (3 s of simulated time) because `zephyr.exe` never exits on its own.
Board is a constant at the top of `docker.sh`; the Zephyr version is `ZEPHYR_VERSION` in `zephyr/Dockerfile`.

## Backend

`source/zephyr/osal_*.c` uses the native Zephyr API:

| Module | Zephyr primitive |
|--------|------------------|
| mutex | `k_mutex` (own pool lock, since the pool cannot use `osal_mutex`) |
| sem | `k_sem` (`-EBUSY`/`-EAGAIN` -> `OSAL_E_TIMEOUT`) |
| task | `k_thread` + static stack per slot (`CONFIG_DMOSAL_TASK_STACK_SIZE`, `_PRIORITY`); `cfg->stack_*`/`priority` are ignored |
| timer | `k_timer` + `k_work`; callback runs in the system work queue, not the ISR |
| queue | `k_msgq` + static buffer per queue (`CONFIG_DMOSAL_QUEUE_BUF_SIZE`, `_MSG_MAX_SIZE`); creating the same name again attaches to the same queue (refcounted, freed with the last handle; not persistent across `osal_deinit`); shorter sends are zero-padded to `msglen` |
| time | `k_sleep`, `k_usleep`, `k_uptime_ticks` |

Zephyr-specific behaviour to know: `osal_timer_delete` from inside the callback
is rejected, mutexes are recursive, `osal_task_delete` aborts the thread.

Add new examples under `examples/` (portable code only) and select them with `EXAMPLE=<name>`.

## Notes

- Pool sizes: `CONFIG_DMOSAL_*` in `zephyr/app/prj.conf`
  (defaults in `zephyr/Kconfig`), mapped to `OSAL_*` macros in `zephyr/CMakeLists.txt`.
- The app finds the module via `ZEPHYR_EXTRA_MODULES` in
  `zephyr/app/CMakeLists.txt`. Your own app does the same and sets `CONFIG_DMOSAL=y`.
- Zephyr source: inside the image at `/opt/zephyrws/zephyr` (browse with `zephyr/docker.sh shell`).
- Only one `main()` per Zephyr image; run extra "apps" as OSAL tasks.
- Log level, `osal_log_*` etc. come from the portable code and print via stdout.
